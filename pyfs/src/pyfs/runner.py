import argparse

from . import util, translations
from tarski.io import FstripsReader


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

    args = parser.parse_args(args)

    if not args.parse_only and args.driver is None:
        parser.error('The "--driver" option is required to run the solver')

    return args


def run(instance, domain=None):
    domain = domain or util.find_domain_filename(instance)
    reader = FstripsReader(raise_on_error=True, theories=[])
    tarski_problem = reader.read_problem(domain, instance)
    fs_problem = translations.tarski.translate_problem(tarski_problem)

    assert False


    domain_name, instance_name = extract_names(args.domain, args.instance)

    # Determine the appropriate output directory for the problem solver, and create it, if necessary
    out_dir = create_output_dir(args, domain_name, instance_name)

    print("{0:<30}{1}".format("Problem domain:", domain_name))
    print("{0:<30}{1}".format("Problem instance:", instance_name))
    print("{0:<30}{1}".format("Working directory:", out_dir))

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
        adl_task = processor.parse_and_ground(args.domain, args.instance, out_dir, not args.debug)
        fs_task = create_fs_task_from_adl(adl_task, domain_name, instance_name)

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

    run_solver(out_dir, args, args.parse_only)

    return 0


def enter(args):
    return run(parse_arguments(args))
