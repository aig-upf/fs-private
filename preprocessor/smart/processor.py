# Nathan Robinson (nathan.m.robinson@gmail.com) 2014
# Miquel Ramirez (miquel.ramirez@gmail.com) 2015, 2016, 2017

import sys, os, time, subprocess, errno, argparse, traceback, resource

from utilities import CodeException, remove, PRE_SUFFIX, GROUND_SUFFIX,\
    solving_error_code, extracting_error_code, ProblemException, tmp_path

from parser import ParsingException, Parser, Grounder

def parse_and_ground( tag, domain_file, problem_file, remove_tmp = True ) :
    #Parse the input PDDL
    try:
        t0 = time.time()
        parser = Parser(domain_file, problem_file)
        print("Parsing the PDDL domain...")
        parser.parse_domain()
        print("Parsing the PDDL problem...")
        parser.parse_problem()

        print("Simplifying the problem representation...")
        problem = parser.problem
        problem.simplify()
        problem.assign_cond_codes()

        end_parsing_time = time.time()
        print("Parsing time:", (end_parsing_time - t0))

        print("Grounding the problem...")
        pre_file_name = os.path.join(tmp_path, tag + PRE_SUFFIX)
        ground_file_name = os.path.join(tmp_path, tag + GROUND_SUFFIX)

        grounder = Grounder(problem, pre_file_name, ground_file_name)
        grounder.ground()

        end_grounding_time = time.time()
        print("Grounding time:", (end_grounding_time - end_parsing_time))

        print("Simplifying the ground encoding...")
        problem.compute_static_preds()
        problem.link_groundings()
        problem.make_flat_preconditions()
        problem.make_flat_effects()
        problem.get_encode_conds()
        problem.make_cond_and_cond_eff_lists()
        problem.link_conditions_to_actions()
        problem.make_strips_conditions()
        # MRJ: Not necessary
        #problem.compute_conflict_mutex()

        end_linking_time = time.time()
        print("Simplify time:", (end_linking_time - end_grounding_time))


    except (ParsingException, ProblemException) as e:
        print(e)
        sys.exit(1)
    finally:
        if remove_tmp:
            try:
                os.system("rm " + pre_file_name)
            except: pass
            try:
                os.system("rm " + ground_file_name)
            except: pass
    return problem

if __name__ == '__main__' :
    print(("Domain: {}\n Problem: {}".format(sys.argv[1],sys.argv[2])))
    t0 = time.time()
    parse_and_ground( 'test', sys.argv[1], sys.argv[2], False )
    print("Total time: {}".format(time.time() - t0))
