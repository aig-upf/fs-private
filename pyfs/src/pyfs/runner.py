
import itertools

from tarski.util import IndexDictionary

from . import util, translations, engines, search
from .grounding import lpgrounding
from tarski.io import FstripsReader


# def compute_problem_index(problem):
#     """ Compute an index of all relevant elements of the problem """
#     lang = problem.language
#     symbols = IndexDictionary(s for s in itertools.chain(lang.predicates, lang.functions) if not s.builtin)
#     types = IndexDictionary(s for s in lang.sorts if not s.builtin)
#     return dict(
#         symbols=symbols,
#         types=types,
#         schemas=IndexDictionary(problem.actions),
#         objects=IndexDictionary(lang.constants()),
#     )


def run(instance, domain=None):
    domain = domain or util.find_domain_filename(instance)
    reader = FstripsReader(raise_on_error=True, theories=[])

    tarski_problem = reader.read_problem(domain, instance)
    # index = compute_problem_index(tarski_problem)

    problem, language_info = translations.tarski.translate_problem(tarski_problem)

    grounding = lpgrounding(tarski_problem, language_info)

    model = search.create_model(problem, index, grounding=grounding, use_match_tree=True)

    engine = engines.breadth_first_search(model)

    result = engine.run()

    if result.solved:
        print("Problem solved")
        print("Plan: {}".format(result.plan))

    # Things we need to expose to the Python interface:
    # - model creator (atm: ground model)
    # - search engine creator (atm: breadth-first search)
    # - search engine objects
    #
    #
    #
    #







    # TODO Code below to be removed, just for the work-in-progress reference

    domain_name, instance_name = extract_names(args.domain, args.instance)

    # Determine the appropriate output directory for the problem solver, and create it, if necessary
    out_dir = create_output_dir(args, domain_name, instance_name)

    print("{0:<30}{1}".format("Problem domain:", domain_name))
    print("{0:<30}{1}".format("Problem instance:", instance_name))
    print("{0:<30}{1}".format("Working directory:", out_dir))

    # Parse the task with FD's parser and transform it to our format
    if not args.asp:
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

    if args.debug:  # If debugging, we perform a dry-run to get the call arguments and generate debugging scripts
        planner_arguments = run_solver(out_dir, args, True)
        generate_debug_scripts(out_dir, planner_arguments)

    run_solver(out_dir, args, args.parse_only)
