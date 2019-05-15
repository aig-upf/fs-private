import shutil

from . import util, translations
from tarski.io import FstripsReader
from tarski.grounding.lp import ground_actions



def run(instance, domain=None):
    domain = domain or util.find_domain_filename(instance)
    reader = FstripsReader(raise_on_error=True, theories=[])
    tarski_problem = reader.read_problem(domain, instance)

    fs_problem = translations.tarski.translate_problem(tarski_problem)

    groundings = ground(tarski_problem)






    # TODO Continue from here on


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


def ground(problem):
    if shutil.which("gringo") is None:
        raise RuntimeError('Install the Clingo ASP solver and put the "gringo" binary on your PATH in order to run '
                           'the requested ASP-based reachability analysis')
    return ground_actions(problem)
