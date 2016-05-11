"""
    The main module script - generate a FSTRIPS problem from a pair of PDDL instance and domain.
"""
import argparse
import glob
import os
import shutil
import subprocess
import sys

# This should be imported from a custom-set PYTHONPATH containing the path to Fast Downward's PDDL parser
from fs_task import create_fs_task
from pddl import tasks, pddl_file
import util
from representation import ProblemRepresentation
from templates import tplManager


def parse_arguments(args):
    parser = argparse.ArgumentParser(description='Parse a given problem instance from a given benchmark set.')
    parser.add_argument('--tag', required=True, help="The name of the generation tag.")
    parser.add_argument('--instance', required=True,
                        help="The problem instance filename (heuristics are used to determine domain filename).")
    parser.add_argument('--domain', required=False, help="The problem domain filename.", default=None)
    parser.add_argument('--planner', default="generic", help="The directory containing the planner sources.")
    parser.add_argument('--output_base', default="../generated",
                        help="The base for the output directory where the compiled planner will be left. "
                             "Additional subdirectories will be created with the name of the domain and the instance")
    parser.add_argument('--output', default=None, help="The final directory (without any added subdirectories)"
                                                       "where the compiled planner will be left.")
    parser.add_argument('--debug', action='store_true', help="Flag to compile in debug mode.")
    parser.add_argument('--edebug', action='store_true', help="Flag to compile in extreme debug mode.")
    parser.add_argument('--run', action='store_true', help="Set to run the solver after compiling it.")

    parser.add_argument("--driver", help='The solver driver file', default=None)
    parser.add_argument("--defaults", help='The solver default options file', default=None)
    parser.add_argument("--options", help='The solver extra options file', default="")

    args = parser.parse_args(args)
    args.instance_dir = os.path.dirname(args.instance)
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


def move_files(base_dir, instance, domain, target_dir):
    """ Moves the domain and instance description files plus additional data files to the translation directory """
    definition_dir = target_dir + '/definition'
    data_dir = target_dir + '/data'

    # Copy the domain and instance file to the subfolder "definition" on the destination dir
    util.mkdirp(definition_dir)
    shutil.copy(instance, definition_dir)
    shutil.copy(domain, definition_dir)

    # The ad-hoc external definitions file - if it does not exist, we use the default.
    if os.path.isfile(base_dir + '/external.hxx'):
        shutil.copy(base_dir + '/external.hxx', target_dir)

        if os.path.isfile(base_dir + '/external.cxx'):  # We also copy a possible cxx implementation file
            shutil.copy(base_dir + '/external.cxx', target_dir)

    else:
        default = tplManager.get('external_default.hxx').substitute()  # No substitutions for the default template
        util.save_file(target_dir + '/external.hxx', default)

    # Copy, if they exist, all data files
    origin_data_dir = base_dir + '/data'
    if os.path.isdir(origin_data_dir):
        for filename in glob.glob(os.path.join(origin_data_dir, '*')):
            if os.path.isfile(filename):
                shutil.copy(filename, data_dir)


def compile_translation(translation_dir, use_vanilla, args):
    """
    Copies the relevant files from the BFS directory to the newly-created translation directory,
     and then calls scons to compile the problem there.
    """
    debug_flag = "edebug=1" if args.edebug else ("debug=1" if args.debug else "")

    planner_dir = os.path.abspath(os.path.join('../planners', args.planner))

    shutil.copy(os.path.join(planner_dir, 'defaults.json'), translation_dir)

    vanilla_solver_name = solver_name(args)
    vanilla_solver_path = os.path.join(planner_dir, vanilla_solver_name)

    if use_vanilla and os.path.isfile(vanilla_solver_path):
        print("Using pre-compiled vanilla solver from '{}'".format(vanilla_solver_path))
        shutil.copy(vanilla_solver_path, translation_dir)

    else:  # We compile the solver from scratch
        shutil.copy(os.path.join(planner_dir, 'main.cxx'), translation_dir)
        shutil.copy(os.path.join(planner_dir, 'SConstruct'), os.path.join(translation_dir, 'SConstruct'))

        command = "scons {}".format(debug_flag)

        print("{0:<30}{1}\n".format("Compilation command:", command))
        sys.stdout.flush()  # Flush the output to avoid it mixing with the subprocess call.
        output = subprocess.call(command.split(), cwd=translation_dir)
        if output != 0:
            raise RuntimeError('Error compiling problem at {0}'.format(translation_dir))


def run_solver(translation_dir, args):
    """ Runs the solver binary resulting from the compilation """
    if not args.run:  # Simply return without running anything
        return

    solver = solver_name(args)
    solver = os.path.join(translation_dir, solver)

    if not args.driver:
        raise RuntimeError("Need to specify a driver to be able to run the solver")

    command = [solver, "--driver", args.driver]

    if args.defaults:
        command += ["--defaults", args.defaults]

    if args.options:
        command += ["--options", args.options]

    print("{0:<30}{1}\n".format("Running solver:", solver))
    sys.stdout.flush()  # Flush the output to avoid it mixing with the subprocess call.
    output = subprocess.call(command, cwd=translation_dir)
    if output != 0:
        print("Error sunning solver")
        sys.exit(output)


def solver_name(args):
    return "solver.edebug.bin" if args.edebug else ("solver.debug.bin" if args.debug else "solver.bin")


def main(args):
    # Determine the proper domain and instance filenames
    if args.domain is None:
        args.domain = pddl_file.extract_domain_name(args.instance)

    domain_name, instance_name = extract_names(args.domain, args.instance)

    # Determine the appropriate output directory for the problem solver, and create it, if necessary
    translation_dir = args.output
    if not translation_dir:
        components = [args.output_base, args.tag, domain_name, instance_name]
        translation_dir = os.path.abspath(os.path.join(*components))
    util.mkdirp(translation_dir)

    print("{0:<30}{1}".format("Problem domain:", domain_name))
    print("{0:<30}{1}".format("Problem instance:", instance_name))
    print("{0:<30}{1}".format("Chosen Planner:", args.planner))
    print("{0:<30}{1}".format("Translation directory:", translation_dir))

    # Parse the task with FD's parser and transform it to our format
    fd_task = parse_pddl_task(args.domain, args.instance)
    fs_task = create_fs_task(fd_task, domain_name, instance_name)

    # Generate the appropriate problem representation from our task, store it, and (if necessary) compile
    # the C++ generated code to obtain a binary tailored to the particular instance
    representation = ProblemRepresentation(fs_task, translation_dir, args.edebug or args.debug)
    representation.generate()
    move_files(args.instance_dir, args.instance, args.domain, translation_dir)
    use_vanilla = not representation.requires_compilation()
    compile_translation(translation_dir, use_vanilla, args)
    run_solver(translation_dir, args)


if __name__ == "__main__":
    # Run only if the hash seed has been set
    if not util.fix_seed_and_possibly_rerun():
        main(parse_arguments(sys.argv[1:]))
