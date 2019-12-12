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

from pathlib import Path

from tarski.grounding import LPGroundingStrategy

from .. import utils, FS_PATH, FS_WORKSPACE, FS_BUILD
from .pddl import tasks, pddl_file
from .fs_task import create_fs_task, create_fs_task_from_adl, create_fs_plus_task
from .representation import ProblemRepresentation
from .templates import tplManager

from tarski.io import FstripsReader
from tarski.utils import resources


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
    parser.add_argument('-p', '--parse-only', action='store_true', help="Parse the problem and compile the generated"
                                                                        " code, if any, but don't run the solver yet.")

    parser.add_argument("--driver", help='The solver driver (controller) to be used.', default=None)
    parser.add_argument("--options", help='The solver extra options', default="")
    parser.add_argument("--asp", action='store_true', help='(Experimental) Use the ASP-based parser+grounder '
                                                           '(strict ADL, without numerics etc.).')

    parser.add_argument('-t', '--tag', default=None,
                        help="(Optional) An arbitrary name that will be used to create the working directory where "
                             "intermediate files will be left, unless overriden by the '-o' option."
                             "If none of both options is provided, a random tag will be generated.")

    parser.add_argument('-o', '--output', default=None, help="(Optional) Path to the working directory. If provided,"
                                                             "overrides the \"-t\" option.")
    parser.add_argument('-w', '--workspace', default=None, help="(Optional) Path to the workspace directory.")
    parser.add_argument('--planfile', default=None, help="(Optional) Path to the file where the solution plan will be left.")
    parser.add_argument("--hybrid", action='store_true', help='Use f-PDDL+ parser and front-end')
    parser.add_argument("--disable-static-analysis", action='store_true', help='Disable static fluent symbol analysis')

    parser.add_argument("--sdd", action='store_true', help='Use SDD-based successor generator')
    parser.add_argument("--var_ordering", default=None, help='Variable ordering for SDD construction')
    parser.add_argument("--sdd_incr_minimization_time", default=0, type=int, help='Incremental minimization time for SDD construction')
    parser.add_argument("--sdd_with_reachability", action='store_true', help='Use reachability analysis to improve SDD')

    args = parser.parse_args(args)

    args.sdd_incr_minimization_time = args.sdd_incr_minimization_time if args.sdd_incr_minimization_time != 0 else None

    if not args.parse_only and args.driver is None:
        parser.error('The "--driver" option is required to run the solver')

    return args


def parse_pddl_task(domain, instance):
    """ Parse the given domain and instance filenames by resorting to the FD PDDL parser """
    domain_pddl = pddl_file.parse_pddl_file("domain", domain)
    task_pddl = pddl_file.parse_pddl_file("task", instance)
    task = tasks.Task.parse(domain_pddl, task_pddl)
    return task


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


def run_solver(translation_dir, args, dry_run):
    """ Runs the solver binary resulting from the compilation """

    solver = solver_name(args)
    solver = os.path.join(translation_dir, solver)

    if not args.driver:
        raise RuntimeError("Need to specify a driver to be able to run the solver")

    command = [solver, "--driver", args.driver]

    if args.options:
        command += ["--options", args.options]

    if args.planfile:
        command += ["--planfile", args.planfile]

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
    output = subprocess.call(command_str, cwd=translation_dir, shell=True, env=env)

    explain_output(output)

    return translation_dir


def explain_output(output):
    if output == 0:
        return

    if output == 1:
        print("Critical error while running the planner.")
    elif output == 2:
        print("Input error while running the planner.")
    elif output == 3:
        print("Unsupported feature requested on the planner.")
    elif output == 4:
        print("No plan was found")
    elif output == 5:
        print("Search ended without finding a solution")
    elif output == 6:
        print("The planner ran out of memory.")

    sys.exit(output)


def solver_name(args):
    return "solver.edebug.bin" if args.edebug else ("solver.debug.bin" if args.debug else "solver.bin")


def create_output_dir(args, domain_name, instance_name):
    """ Determine what the output dir should be and create it. Return the output dir path. """
    translation_dir = args.output
    if not translation_dir:
        if args.tag is None:
            import time
            args.tag = time.strftime("%y%m%d")
        workspace = FS_WORKSPACE if args.workspace is None else args.workspace
        translation_dir = os.path.abspath(os.path.join(*[workspace, args.tag, domain_name, instance_name]))
    utils.mkdirp(translation_dir)
    return translation_dir


def run(args):
    # Determine the proper domain and instance filenames
    if args.domain is None:
        args.domain = pddl_file.extract_domain_name(args.instance)

    domain_name, instance_name = extract_names(args.domain, args.instance)

    # Determine the appropriate output directory for the problem solver, and create it, if necessary
    out_dir = create_output_dir(args, domain_name, instance_name)

    print(f'Problem domain: "{domain_name}" ({os.path.realpath(args.domain)})')
    print(f'Problem instance: "{instance_name}" ({os.path.realpath(args.instance)})')
    print(f'Workspace: {os.path.realpath(out_dir)}')

    # Parse the task with FD's parser and transform it to our format
    if not args.asp:
        if args.hybrid:
            from . import f_pddl_plus
            hybrid_task = f_pddl_plus.parse_f_pddl_plus_task(args.domain, args.instance)
            fs_task = create_fs_plus_task(hybrid_task, domain_name, instance_name, args.disable_static_analysis)
        else:
            fd_task = parse_pddl_task(args.domain, args.instance)
            fs_task = create_fs_task(fd_task, domain_name, instance_name)
    else:
        from .asp import processor
        with resources.timing(f"Parsing and simplifying the problem with the ASP-based parser", newline=True):
            adl_task = processor.parse_and_ground(args.domain, args.instance, out_dir, not args.debug)
        fs_task = create_fs_task_from_adl(adl_task, domain_name, instance_name)

        # TODO This is part of the same quickfix=hack in create_fs_task_from_adl - read the big TODO there!.
        # TODO Clearly it's not too good to be parsing TWICE the problem, once with the ASP parser and the other
        # TODO with the FD parser. This should go away ASAP.
        fs_task_parsed_by_fd = create_fs_task(parse_pddl_task(args.domain, args.instance), domain_name, instance_name)
        fs_task.initial_fluent_atoms = fs_task_parsed_by_fd.initial_fluent_atoms


    # Generate the appropriate problem representation from our task, store it, and (if necessary) compile
    # the C++ generated code to obtain a binary tailored to the particular instance
    representation = ProblemRepresentation(fs_task, out_dir, args.edebug or args.debug)
    representation.generate()
    use_vanilla = not representation.requires_compilation()

    move_files(args, out_dir, use_vanilla)
    if not args.parse_only:
        compile_translation(out_dir, use_vanilla, args)

    if args.debug:  # If debugging, we perform a dry-run to get the call arguments and generate debugging scripts
        planner_arguments = run_solver(out_dir, args, True)
        generate_debug_scripts(out_dir, planner_arguments)

    if args.sdd:
        with resources.timing(f"Parsing problem with Tarski", newline=True):
            problem = parse_problem_with_tarski(args.domain, args.instance)

        if args.sdd_with_reachability:
            grounding = LPGroundingStrategy(problem, ground_actions=False)
            reachable_vars = grounding.ground_state_variables()
            # reachable_actions = grounding.ground_actions()
            # num_groundings = {k: len(gr) for k, gr in reachable_actions.items()}
            # print(f'Number of reachable groundings for action schema: {num_groundings}')

        else:
            reachable_vars = None

        sdddir = os.path.join(out_dir, 'data', 'sdd')
        utils.mkdirp(sdddir)
        from tarski.sdd.sdd import process_problem
        process_problem(problem, serialization_directory=sdddir, conjoin_with_init=False,
                        sdd_minimization_time=None, graphs_directory=None,
                        var_ordering=args.var_ordering, reachable_vars=reachable_vars,
                        sdd_incr_minimization_time=args.sdd_incr_minimization_time)

    # return True  # Just to debug the preprocessing

    translation_dir = run_solver(out_dir, args, args.parse_only)

    return validate(args.domain, args.instance, os.path.join(translation_dir, 'first.plan'))


def validate(domain_name, instance_name, planfile):

    with resources.timing(f"Running validate", newline=True):
        plan = Path(planfile)
        if not plan.is_file():
            logging.info("No plan file could be found.")
            return -1

        validate_inputs = ["validate", domain_name, instance_name, planfile]

        try:
            output = subprocess.call(' '.join(validate_inputs), shell=True)
        except OSError as err:
            if err.errno == errno.ENOENT:
                logging.error("Error: 'validate' binary not found. Is it on the PATH?")
                return -1
            else:
                logging.error("Error executing 'validate': {}".format(err))

    return 0


def parse_problem_with_tarski(domain_file, inst_file):
    reader = FstripsReader(raise_on_error=True, theories=None, strict_with_requirements=False)
    return reader.read_problem(domain_file, inst_file)


def main(args):
    args = parse_arguments(args)
    import logging
    logging.basicConfig(stream=sys.stdout, level=logging.DEBUG if args.debug else logging.INFO)
    return run(args)
