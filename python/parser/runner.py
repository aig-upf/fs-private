"""
    The main module script - generate a FSTRIPS problem from a pair of PDDL instance and domain.
"""
import sys
import os
import argparse
import glob
import shutil
import subprocess

from python import utils, FS_PATH, FS_WORKSPACE
from .pddl import tasks, pddl_file
from .fs_task import create_fs_task, create_fs_task_from_adl, create_fs_plus_task
from .representation import ProblemRepresentation
from .templates import tplManager


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
    parser.add_argument("--hybrid", action='store_true', help='Use f-PDDL+ parser and front-end')
    parser.add_argument("--disable-static-analysis", action='store_true', help='Disable static fluent symbol analysis')

    args = parser.parse_args(args)

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


def move_files(instance, domain, target_dir, use_vanilla):
    """ Moves the domain and instance description files plus additional data files to the translation directory """
    base_dir = os.path.dirname(instance)
    definition_dir = target_dir + '/definition'
    data_dir = target_dir + '/data'

    # Copy the domain and instance file to the subfolder "definition" on the destination dir
    utils.mkdirp(definition_dir)
    shutil.copy(instance, definition_dir)
    shutil.copy(domain, definition_dir)

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
        if os.path.exists(os.path.join(translation_dir, 'util')) :
            shutil.rmtree( os.path.join(translation_dir, 'util') )
        shutil.copytree( 'util', os.path.join(translation_dir, 'util'))
        command = "python2 /usr/bin/scons {}".format(debug_flag)

        print("{0:<30}{1}\n".format("Compilation command:", command))
        sys.stdout.flush()  # Flush the output to avoid it mixing with the subprocess call.
        output = subprocess.call(command.split(), cwd=translation_dir)
        if output != 0:
            raise RuntimeError('Error compiling problem at {0}'.format(translation_dir))


def run_solver(translation_dir, args):
    """ Runs the solver binary resulting from the compilation """
    if args.parse_only:  # Simply return without running anything
        return

    solver = solver_name(args)
    solver = os.path.join(translation_dir, solver)

    if not args.driver:
        raise RuntimeError("Need to specify a driver to be able to run the solver")

    command = [solver, "--driver", args.driver]

    if args.options:
        command += ["--options", args.options]

    print("{0:<30}{1}".format("Running solver:", solver))
    print("{0:<30}{1}\n".format("Command line arguments:", ' '.join(command[1:])))
    sys.stdout.flush()  # Flush the output to avoid it mixing with the subprocess call.

    command_str = ' '.join(command)
    # We run the command spawning a new shell so that we can get typical shell kill signals such as OOM, etc.
    output = subprocess.call(command_str, cwd=translation_dir, shell=True)
    if output != 0:
        print("Error running solver. Output code: {}".format(output))
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
        translation_dir = os.path.abspath(os.path.join(*[FS_WORKSPACE, args.tag, domain_name, instance_name]))
    utils.mkdirp(translation_dir)
    return translation_dir


def run(args):
    # Determine the proper domain and instance filenames
    if args.domain is None:
        args.domain = pddl_file.extract_domain_name(args.instance)

    domain_name, instance_name = extract_names(args.domain, args.instance)

    # Determine the appropriate output directory for the problem solver, and create it, if necessary
    out_dir = create_output_dir(args, domain_name, instance_name)

    print("{0:<30}{1}".format("Problem domain:", domain_name))
    print("{0:<30}{1}".format("Problem instance:", instance_name))
    print("{0:<30}{1}".format("Working directory:", out_dir))

    # Parse the task with FD's parser and transform it to our format
    if not args.asp:
        if args.hybrid :
            from . import f_pddl_plus
            hybrid_task = f_pddl_plus.parse_f_pddl_plus_task(args.domain, args.instance)
            fs_task = create_fs_plus_task( hybrid_task, domain_name, instance_name, args.disable_static_analysis)
        else :
            fd_task = parse_pddl_task(args.domain, args.instance)
            fs_task = create_fs_task(fd_task, domain_name, instance_name)
    else:
        from .asp import processor
        adl_task = processor.parse_and_ground(args.domain, args.instance, out_dir)
        fs_task = create_fs_task_from_adl(adl_task, domain_name, instance_name)

    # Generate the appropriate problem representation from our task, store it, and (if necessary) compile
    # the C++ generated code to obtain a binary tailored to the particular instance
    representation = ProblemRepresentation(fs_task, out_dir, args.edebug or args.debug)
    representation.generate()
    use_vanilla = not representation.requires_compilation()

    move_files(args.instance, args.domain, out_dir, use_vanilla)
    if not args.parse_only :
        compile_translation(out_dir, use_vanilla, args)
    run_solver(out_dir, args)
    return 0


def main(args):
    return run(parse_arguments(args))
