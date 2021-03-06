"""
    The main module script - generate a FSTRIPS problem from a pair of PDDL instance and domain.
"""
import errno
import logging
import sys
import os
import stat
import argparse
import glob
import shutil
import subprocess
from collections import OrderedDict
from pathlib import Path

from .. import utils, FS_PATH, FS_WORKSPACE, FS_BUILD
from .templates import tplManager
from .tarski_serialization import generate_tarski_problem, serialize_representation, Serializer, print_groundings
from tarski.syntax.ops import compute_sort_id_assignment
from tarski.fstrips.representation import compile_universal_effects_away
from tarski.io import FstripsReader, find_domain_filename
from tarski.utils import resources
from tarski.grounding import LPGroundingStrategy, NaiveGroundingStrategy
import tarski.errors as terr


def parse_arguments(args):
    parser = argparse.ArgumentParser(description='Bootstrap and run the FS planner on a given instance.'
                                                 'The process might involve generating, compiling and linking'
                                                 'some C++ code in order to accommodate externally-defined symbols.'
                                                 'That code will be left in the "working directory", whose path is '
                                                 'controlled through the "-t" and "-o" options.')
    parser.add_argument('-i', '--instance', required=True, help="The path to the problem instance file.")
    parser.add_argument('--domain', default=None, help="(Optional) The path to the problem domain file. If none is "
                                                       "provided, the system will try to automatically deduce "
                                                       "it from the instance filename.")

    parser.add_argument('--debug', action='store_true', help="Compile in debug mode.")
    parser.add_argument('--edebug', action='store_true', help="Compile in _extreme_ debug mode.")

    parser.add_argument("--fd", action='store_true',
                        help='Run in "SAS+ mode", assuming a standard IPC/propositional encoding, and using Fast '
                             'Downward\'s preprocessor to generate a SAS file from the PDDL')

    parser.add_argument("--safe", action='store_true',
                        help='Run in "safe mode", meaning: do no assume a standard IPC / propositional encoding, but '
                             'make room for the possibility of having functions and other advanced features')

    parser.add_argument('-p', '--parse-only', action='store_true', help="Parse the problem and compile the generated"
                                                                        " code, if any, but don't run the solver yet.")

    parser.add_argument("--driver", help='The solver driver (controller) to be used.', default=None)
    parser.add_argument("--options", help='The solver extra options', default="")
    parser.add_argument("--reachability", help='The type of reachability analysis performed', default="full",
                        choices=('full', 'vars', 'none'))
    parser.add_argument("--reachability-includes-variable-inequalities", action='store_true',
                        help='Include inequalities of the form X != Y in the reachability analysis. This makes the '
                             'analysis NP-hard, but results in a tighter approximation.')

    parser.add_argument('-t', '--tag', default=None,
                        help="(Optional) An arbitrary name that will be used to create the working directory where "
                             "intermediate files will be left, unless overriden by the '-o' option."
                             "If none of both options is provided, a random tag will be generated.")

    parser.add_argument('-o', '--output', default=None, help="(Optional) Path to the working directory. If provided,"
                                                             "overrides the \"-t\" option.")
    parser.add_argument('-w', '--workspace', default=None, help="(Optional) Path to the workspace directory.")
    parser.add_argument('--planfile', default=None, help="(Optional) Path to the file where the solution plan "
                                                         "will be left.")

    parser.add_argument("--sdd", action='store_true', help='Use SDD-based successor generator.')
    parser.add_argument("--var_ordering", default=None, help='Variable ordering for SDD construction')
    parser.add_argument("--sdd_incr_minimization_time", default=0, type=int,
                        help='Incremental minimization time for SDD construction')
    parser.add_argument("--sdd_with_reachability", action='store_true', help='Use reachability analysis to improve SDD')

    args = parser.parse_args(args)

    args.sdd_incr_minimization_time = args.sdd_incr_minimization_time if args.sdd_incr_minimization_time != 0 else None

    if not args.parse_only and args.driver is None:
        parser.error('The "--driver" option is required to run the solver')

    return args


def extract_names(domain_filename, instance_filename):
    """ Extract the canonical domain and instance names from the corresponding filenames """
    domain = os.path.basename(os.path.dirname(domain_filename))
    instance = os.path.splitext(os.path.basename(instance_filename))[0]
    return domain, instance


def move_files(args, target_dir, use_vanilla):
    """ Moves the domain and instance description files plus additional data files to the translation directory """
    base_dir = os.path.dirname(args.instance)
    definition_dir = target_dir + '/definition'
    data_dir = target_dir + '/data'

    # Copy the domain and instance file to the subfolder "definition" on the destination dir
    utils.mkdirp(definition_dir)
    shutil.copy(args.instance, definition_dir)
    shutil.copy(args.domain, definition_dir)

    is_external_defined = os.path.isfile(base_dir + '/external.hxx')

    if is_external_defined and use_vanilla:
        raise RuntimeError("An external definitions file was found at '{}', but the runner script determined"
                           " that no external files were needed. Something is wrong.".format(base_dir))

    if not use_vanilla:
        # The ad-hoc external definitions file - if it does not exist, we use the default.
        if is_external_defined:
            shutil.copy(base_dir + '/external.hxx', target_dir)
            if os.path.isfile(base_dir + '/external.cxx'):  # We also copy a possible cxx implementation file
                shutil.copy(base_dir + '/external.cxx', target_dir)

        else:
            default = tplManager.get('external_default.hxx').substitute()  # No substitutions for the default template
            utils.save_file(target_dir + '/external.hxx', default)

    # Copy, if they exist, all data files
    origin_data_dir = base_dir + '/data'
    if os.path.isdir(origin_data_dir):
        for filename in glob.glob(os.path.join(origin_data_dir, '*')):
            if os.path.isfile(filename):
                shutil.copy(filename, data_dir)
            else:
                dst = os.path.join(data_dir, os.path.basename(filename))
                if os.path.exists(dst):
                    shutil.rmtree(dst)
                shutil.copytree(filename, dst)


def generate_debug_scripts(target_dir, planner_arguments):
    # If generating a debug build, create some debug script helpers
    shebang = "#!/usr/bin/env bash"
    ld_string = "LD_LIBRARY_PATH={}:$LD_LIBRARY_PATH".format(FS_BUILD)
    args = ' '.join(planner_arguments)
    debug_script = "{}\n\n{} cgdb -ex=run --args ./solver.debug.bin {}".format(shebang, ld_string, args)
    memleaks = "{}\n\n{} valgrind --leak-check=full --show-leak-kinds=all --num-callers=50 --track-origins=yes " \
               "--log-file=\"valgrind-output.$(date '+%H%M%S').txt\" ./solver.debug.bin {}"\
        .format(shebang, ld_string, args)

    memprofile = "{}\n\n{} valgrind --tool=massif ./solver.debug.bin {}".format(shebang, ld_string, args)
    callgrind = f"{shebang}\n\n{ld_string} valgrind --tool=callgrind ./solver.debug.bin {args}"

    make_script(os.path.join(target_dir, 'debug.sh'), debug_script)
    make_script(os.path.join(target_dir, 'memleaks.sh'), memleaks)
    make_script(os.path.join(target_dir, 'memprofile.sh'), memprofile)
    make_script(os.path.join(target_dir, 'callgrind.sh'), callgrind)


def make_script(filename, code):
    with open(filename, 'w') as f:
        print(code, file=f)
    st = os.stat(filename)
    os.chmod(filename, st.st_mode | stat.S_IEXEC)


def compile_translation(translation_dir, use_vanilla, args):
    """
    Copies the relevant files from the planner directory to the newly-created translation directory,
     and then calls scons to compile the problem there.
    """
    debug_flag = "edebug=1" if args.edebug else ("debug=1" if args.debug else "")

    planner_dir = os.path.abspath(os.path.join(FS_PATH, 'planners', 'generic'))
    shutil.copy(os.path.join(planner_dir, 'defaults.json'), translation_dir)

    vanilla_solver_name = solver_name(args)
    vanilla_solver_path = os.path.join(planner_dir, vanilla_solver_name)

    if use_vanilla and not os.path.isfile(vanilla_solver_path):
        raise RuntimeError("The problem requires using the pre-compiled vanilla solver binary, but it can't be found on"
                           " the expected path. Please re-build the project with the appropriate debug configuration.")

    if use_vanilla:
        print("Using pre-compiled vanilla solver from '{}'".format(vanilla_solver_path))
        shutil.copy(vanilla_solver_path, translation_dir)

    else:  # We compile the solver from scratch
        shutil.copy(os.path.join(planner_dir, 'main.cxx'), translation_dir)
        shutil.copy(os.path.join(planner_dir, 'SConstruct'), os.path.join(translation_dir, 'SConstruct'))
        shutil.copy(os.path.join(planner_dir, 'custom.py'), os.path.join(translation_dir, 'custom.py'))
        if os.path.exists(os.path.join(translation_dir, 'util')):
            shutil.rmtree(os.path.join(translation_dir, 'util'))
            shutil.copytree('util', os.path.join(translation_dir, 'util'))
        command = "python2 /usr/bin/scons {}".format(debug_flag)

        print("{0:<30}{1}\n".format("Compilation command:", command))
        sys.stdout.flush()  # Flush the output to avoid it mixing with the subprocess call.
        output = subprocess.call(command.split(), cwd=translation_dir)
        if output != 0:
            raise RuntimeError('Error compiling problem at {0}'.format(translation_dir))


def run_solver(workdir, args, dry_run):
    """ Runs the solver binary resulting from the compilation """

    solver = solver_name(args)
    solver = os.path.join(workdir, solver)

    if not args.driver:
        raise RuntimeError("Need to specify a driver to be able to run the solver")

    command = [solver, "--driver", args.driver]

    if args.options:
        command += ["--options", args.options]

    if args.planfile:
        command += ["--planfile", args.planfile]

    if args.fd:
        command += ["--fd"]

    arguments = command[1:]
    if dry_run:  # Simply return without running anything
        return arguments

    print("{0:<30}{1}".format("Running solver:", solver))
    print("{0:<30}{1}\n".format("Command line arguments:", ' '.join(arguments)))
    sys.stdout.flush()  # Flush the output to avoid it mixing with the subprocess call.

    env = dict(os.environ)

    # We prioritize the FS library that resides within this project
    env['LD_LIBRARY_PATH'] = ':'.join([FS_BUILD, env['LD_LIBRARY_PATH']])

    command_str = ' '.join(command)
    # We run the command spawning a new shell so that we can get typical shell kill signals such as OOM, etc.
    output = subprocess.call(command_str, cwd=workdir, shell=True, env=env)

    explain_output(output)

    return output


def is_nonerror_code(code):
    return 0 <= code <= 19


def print_code_message(code):
    msg = {
        0: None,
        11: "Problem is provably unsolvable",
        12: "Search ended without finding a plan",
        22: "Search ran out of memory",
        23: "Search ran out of time",
        32: "Critical error during search",
        33: "Input error",
        34: "Unsupported feature requested",
    }.get(code)
    if msg is not None:
        print(msg)


def explain_output(code):
    print_code_message(code)

    if is_nonerror_code(code):
        return

    sys.exit(code)


def solver_name(args):
    return "solver.edebug.bin" if args.edebug else ("solver.debug.bin" if args.debug else "solver.bin")


def create_working_dir(args, domain_name, instance_name):
    """ Determine what the output dir should be and create it. Return the output dir path. """
    translation_dir = args.output
    if not translation_dir:
        if args.tag is None:
            import time
            args.tag = time.strftime("%y%m%d")
        workspace = FS_WORKSPACE if args.workspace is None else args.workspace
        translation_dir = os.path.abspath(os.path.join(workspace, args.tag, domain_name, instance_name))

    # Remove previous directory, if existed *and* is below the planner directory tree
    wd = Path(translation_dir).resolve()
    if wd.exists() and Path(FS_PATH) == wd:
        shutil.rmtree(translation_dir)

    wd.mkdir(parents=True, exist_ok=True)
    return str(wd)


def sort_state_variables(ground_variables):
    from tarski.util import SymbolIndex
    varnames = sorted(ground_variables, key=str)
    variables = SymbolIndex()
    for v in varnames:
        variables.add(v)
    return variables


def run(args):
    is_debug_run = args.edebug or args.debug
    # Determine the proper domain and instance filenames
    if args.domain is None:
        args.domain = find_domain_filename(args.instance)
        if args.domain is None:
            raise RuntimeError(f'Could not find domain filename that matches instance file "{args.instance}"')

    domain_name, instance_name = extract_names(args.domain, args.instance)

    # Determine the appropriate output directory for the problem solver, and create it, if necessary
    workdir = create_working_dir(args, domain_name, instance_name)

    print(f'Problem domain: "{domain_name}" ({os.path.realpath(args.domain)})')
    print(f'Problem instance: "{instance_name}" ({os.path.realpath(args.instance)})')
    print(f'Workspace: {os.path.realpath(workdir)}')

    if args.fd:
        from .fd import parse_with_fd_translator
        parse_with_fd_translator(args.domain, args.instance, workdir)
    else:
        preprocess_with_tarski(args, is_debug_run, workdir)

    use_vanilla = True
    move_files(args, workdir, use_vanilla)
    if not args.parse_only:
        compile_translation(workdir, use_vanilla, args)
    if is_debug_run:  # If debugging, we perform a dry-run to get the call arguments and generate debugging scripts
        planner_arguments = run_solver(workdir, args, True)
        generate_debug_scripts(workdir, planner_arguments)

    # Invoke the planner:
    output = run_solver(workdir, args, args.parse_only)

    # Validate the resulting plan:
    if output == 0:
        validate(args.domain, args.instance, os.path.join(workdir, 'first.plan'))

    return output


def preprocess_with_tarski(args, is_debug_run, workdir):
    t0 = resources.Timer()

    with resources.timing(f"Parsing problem", newline=True):
        problem = parse_problem_with_tarski(args.domain, args.instance)
        # Indexes 0 and 1 are reserved for two special boolean objects true and false
        sort_bounds, object_ids = compute_sort_id_assignment(problem.language, start=2)

    # Both the LP reachability analysis and the backend expect a problem without universally-quantified effects
    with resources.timing(f"Compiling universal effects away", newline=False):
        problem = compile_universal_effects_away(problem)

    action_groundings, ground_variables, fluents, statics = ground_problem(args, problem)

    if is_debug_run:
        # Note: Make sure that this is done before the ground variable index is used for any purpose!
        ground_variables = sort_state_variables(ground_variables)

    # Be careful with this, as it'll introduce new state variables
    # if not args.safe and 'bfws' in args.driver:
    #     with resources.timing(f"Compiling negated preconditions away", newline=False):
    #         problem = compile_negated_preconditions_away(problem, inplace=False)

    if args.sdd:
        generate_sdds(args, ground_variables, problem, workdir)

    if not args.safe and '-csp' in args.driver:
        generate_csps(args, ground_variables, problem, sort_bounds, object_ids, workdir)

    data, init_atoms, obj_idx = generate_tarski_problem(
        problem, fluents, statics, sort_bounds, object_ids, variables=ground_variables)
    serializer = Serializer(os.path.join(workdir, 'data'))
    serialize_representation(data, init_atoms, serializer, debug=is_debug_run)

    # Print the reachable action groundings if available; otherwise remove possible grounding files from previous runs.
    print_groundings(data['action_schemata'], action_groundings, obj_idx, serializer)
    print(f"Python parser and preprocessing: {t0}")


def generate_sdds(args, ground_variables, problem, workdir):
    from tarski.analysis.sdd import process_problem
    workdir = os.path.join(workdir, 'data', 'sdd')

    utils.wipedir(workdir)
    utils.mkdirp(workdir)

    process_problem(problem, serialization_directory=workdir, conjoin_with_init=False,
                    sdd_minimization_time=None, graphs_directory=None,
                    var_ordering=args.var_ordering, reachable_vars=ground_variables,
                    sdd_incr_minimization_time=args.sdd_incr_minimization_time)


def generate_csps(args, ground_variables, problem, sort_bounds, object_ids, workdir):
    from tarski.analysis.csp_schema import CSPCompiler
    workdir = os.path.join(workdir, 'data', 'csps')

    utils.wipedir(workdir)
    utils.mkdirp(workdir)

    comp = CSPCompiler(problem, ground_variables, sort_bounds, object_ids)
    inapplicable = comp.process_problem(serialization_directory=workdir)
    if inapplicable:
        # Prune those actions deemed as not applicable
        logging.info(f"The following action schemas were statically deemed non-applicable: {inapplicable}")
        problem.actions = {aname: action for aname, action in problem.actions.items() if aname not in inapplicable}


def ground_problem(args, problem):
    do_reachability = args.reachability != 'none' and not args.sdd  # SDD not compatible with reachability
    if not do_reachability and args.reachability_includes_variable_inequalities:
        raise RuntimeError("Cannot do reachability analysis with inequalities if reachability=none is specified.")

    action_groundings = None  # Schemas will be ground in the backend
    if do_reachability:
        do_ground_actions = args.reachability == 'full'
        msg = "Computing reachable groundings " + ("(actions+vars)" if do_ground_actions else "(vars only)")
        with resources.timing(msg, newline=True):
            grounding = LPGroundingStrategy(
                problem, ground_actions=do_ground_actions,
                include_variable_inequalities=args.reachability_includes_variable_inequalities)
            ground_variables = grounding.ground_state_variables()
            if do_ground_actions:
                action_groundings = grounding.ground_actions()

    else:
        with resources.timing(f"Computing naive groundings", newline=True):
            grounding = NaiveGroundingStrategy(problem, ignore_symbols={'total-cost'})
            ground_variables = grounding.ground_state_variables()
    statics, fluents = grounding.static_symbols, grounding.fluent_symbols

    # If we did generate some action groundings, then we prune those action schemas that have no grounding at all
    if action_groundings:
        reachable_schemas = OrderedDict()
        for name, act in problem.actions.items():
            if action_groundings[name]:
                reachable_schemas[name] = act
        problem.actions = reachable_schemas

    return action_groundings, ground_variables, fluents, statics


def validate(domain_name, instance_name, planfile):

    with resources.timing(f"Running validate", newline=True):
        plan = Path(planfile)
        if not plan.is_file():
            logging.info("No plan file to validate could be found.")
            return

        validate_inputs = ["validate", domain_name, instance_name, planfile]

        try:
            _ = subprocess.call(' '.join(validate_inputs), shell=True)
        except OSError as err:
            if err.errno == errno.ENOENT:
                logging.error("Error: 'validate' binary not found. Is it on the PATH?")
                return
            else:
                logging.error("Error executing 'validate': {}".format(err))


def parse_problem_with_tarski(domain_file, inst_file):
    reader = FstripsReader(raise_on_error=True, theories=None, strict_with_requirements=False, case_insensitive=True)
    return reader.read_problem(domain_file, inst_file)


def main(args):
    args = parse_arguments(args)
    import logging
    logging.basicConfig(stream=sys.stdout, level=logging.DEBUG if args.debug else logging.INFO)

    try:
        return run(args)
    except terr.OutOfMemoryError:
        explain_output(22)
        return 22
    except terr.OutOfTimeError:
        explain_output(23)
        return 23
