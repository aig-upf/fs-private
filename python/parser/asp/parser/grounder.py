# Nathan Robinson (nathan.m.robinson@gmail.com) 2014
# Miquel Ramirez (miquel.ramirez@gmail.com) 2015, 2016, 2017

""" This file represents a Problem in ASP (Answer Set Programming) and uses
    gringo, the grounding system of the clasp ASP solver) to find all reachable
    atoms. It is very efficient.
"""

from ..utilities import grounding_error_code, grounder_run_success_code, \
    asp_convert, grounder_path, var_alphabet, neg_prec_prefix, equality_prefix, \
    inequality_prefix, default_type_name
from .parser import ParsingException
from ..problem import Object, Type, Function, Predicate, \
    PredicateCondition, NotCondition, AndCondition, \
    OrCondition, ForAllCondition, ExistsCondition, \
    IncreaseCondition, EqualsCondition, ConditionalEffect

import os, subprocess, itertools


class Grounder(object):
    """ Used to ground PDDL domains and problems. """

    def __init__(self, problem, pre_file_name, grounding_file_name):
        """ Create the grounding system.
            (Grounder, Problem) -> None
        """
        self.problem = problem
        self.pre_file_name = pre_file_name
        self.grounding_file_name = grounding_file_name

        # The following are simply maps from the modified names of the problem
        # components back to the components themselves
        self.asp_types = dict(problem.types)

        self.asp_objects = {}
        self.asp_predicates = {}
        self.asp_functions = {}
        self.asp_actions = {}
        self.asp_dp = {}

    def rename_asp_components(self):
        """ Rename the appropriate problem elements using the function asp_convert
            defined in utilities.
            (Grounder) -> None
        """
        # Types
        self.asp_types = {}
        for ttype in list(self.problem.types.values()):
            ttype.asp_name = asp_convert(ttype.name)
            self.asp_types[ttype.asp_name] = ttype

        # Objects
        for obj in list(self.problem.objects.values()):
            obj.asp_name = asp_convert(obj.name)
            self.asp_objects[obj.asp_name] = obj

        # Predicates
        for pred in list(self.problem.predicates.values()):
            pred.asp_name = asp_convert(pred.name)
            self.asp_predicates[pred.asp_name] = pred

        # Functions
        for func in list(self.problem.functions.values()):
            func.asp_name = asp_convert(func.name)
            self.asp_functions[func.asp_name] = func

        # Actions
        for action in list(self.problem.actions.values()):
            action.asp_name = asp_convert(action.name)
            self.asp_actions[action.asp_name] = action

    def write_prec_asp(self, condition, out_file):
        """ Write the description of this conditon as a precondition into asp.
            (Grounder, Condition, file) -> None
        """
        if isinstance(condition, PredicateCondition):
            pred_name = condition.pred.asp_name
            if not condition.sign:
                pred_name = neg_prec_prefix + pred_name
            if condition.variables:
                v_strs, p_strs, alph_pos = [], [], 0
                for v in condition.variables:
                    if v in condition.relevant_vars:
                        v_strs.append(var_alphabet[alph_pos])
                        p_strs.append(var_alphabet[alph_pos])
                        alph_pos += 1
                    else:
                        p_strs.append(self.problem.objects[v].asp_name)
                if v_strs:
                    out_file.write("reachable(" + condition.cond_code + "(" + \
                                   ", ".join(v_strs) + ")) :- reachable_f(" + pred_name + \
                                   "(" + ", ".join(p_strs) + ")" + ").\n")
                else:
                    out_file.write("reachable(" + condition.cond_code + ") :- reachable_f(" + \
                                   pred_name + "(" + ", ".join(p_strs) + ")" + ").\n")
            else:
                out_file.write("reachable(" + condition.cond_code + ") :- reachable_f(" + \
                               pred_name + ").\n")

        elif isinstance(condition, AndCondition):
            cond_strs = []
            for cond in condition.conditions:
                if cond.relevant_vars:
                    cond_strs.append("reachable(" + cond.cond_code + "(" + ", ".join([var_alphabet[condition.var_indices[v]] for v in cond.relevant_vars]) + "))")
                else:
                    cond_strs.append("reachable(" + cond.cond_code + ")")
            if condition.relevant_vars:
                out_file.write("reachable(" + condition.cond_code + "(" + \
                               ", ".join(var_alphabet[:len(condition.relevant_vars)]) + ")) :- " + \
                               ", ".join(cond_strs) + ".\n")
            else:
                out_file.write("reachable(" + condition.cond_code + ") :- " + \
                               ", ".join(cond_strs) + ".\n")

            for cond in condition.conditions:
                self.write_prec_asp(cond, out_file)

        elif isinstance(condition, OrCondition):
            for cond in condition.conditions:
                if condition.relevant_vars:
                    out_file.write("reachable(" + condition.cond_code + "(" + \
                                   ", ".join(var_alphabet[:len(condition.relevant_vars)]) + ")) :- ")
                else:
                    out_file.write("reachable(" + condition.cond_code + ") :- ")

                if cond.relevant_vars:
                    out_file.write("reachable(" + cond.cond_code + "(" + \
                                   ", ".join([var_alphabet[condition.var_indices[v]] \
                                              for v in cond.relevant_vars]) + "))")
                else:
                    out_file.write("reachable(" + cond.cond_code + ")")

                extra_bindings = [condition.var_types[v].name + "(" + var_alphabet[vid] + ")" \
                                  for vid, v in enumerate(condition.relevant_vars) if v not in cond.relevant_vars]
                if extra_bindings:
                    out_file.write(", " + ", ".join(extra_bindings))
                out_file.write(".\n")

            for cond in condition.conditions:
                self.write_prec_asp(cond, out_file)

        elif isinstance(condition, ForAllCondition):
            if condition.relevant_vars:
                out_file.write("reachable(" + condition.cond_code + "(" + \
                               ", ".join(var_alphabet[:len(condition.relevant_vars)]) + ")) :- ")
            else:
                out_file.write("reachable(" + condition.cond_code + ") :- ")

            cond_strs = []
            if condition.condition.relevant_vars:
                for obj in condition.v_type.objects:
                    cond_strs.append("reachable(" + condition.condition.cond_code + "(" + \
                                     ", ".join([var_alphabet[condition.var_indices[v]] \
                                                    if v in condition.relevant_vars else obj.asp_name \
                                                for v in condition.condition.relevant_vars]) + "))")
            else:
                cond_strs.append("reachable(" + condition.condition.cond_code + ")")
            out_file.write(", ".join(cond_strs) + ".\n")
            self.write_prec_asp(condition.condition, out_file)

        elif isinstance(condition, ExistsCondition):
            if condition.relevant_vars:
                out_file.write("reachable(" + condition.cond_code + "(" + \
                               ", ".join(var_alphabet[:len(condition.relevant_vars)]) + ")) :- ")
            else:
                out_file.write("reachable(" + condition.cond_code + ") :- ")

            if condition.condition.relevant_vars:
                cond_str = "reachable(" + condition.condition.cond_code + "(" + \
                           ", ".join([var_alphabet[condition.var_indices[v]] \
                                          if v in condition.relevant_vars \
                                          else var_alphabet[len(condition.relevant_vars)] \
                                      for v in condition.condition.relevant_vars]) + "))"
            else:
                cond_str = "reachable(" + condition.condition.cond_code + ")"
            out_file.write(cond_str + ".\n")

            self.write_prec_asp(condition.condition, out_file)

        elif isinstance(condition, EqualsCondition):
            prefix = inequality_prefix if not condition.sign else equality_prefix
            v_strs, p_strs, alph_pos = [], [], 0
            for v in condition.variables:
                if v in condition.relevant_vars:
                    v_strs.append(var_alphabet[alph_pos])
                    p_strs.append(var_alphabet[alph_pos])
                    alph_pos += 1
                else:
                    p_strs.append(self.problem.objects[v].asp_name)
            out_file.write("reachable(" + condition.cond_code + "(" + \
                           ", ".join(v_strs) + ")) :- " + prefix + "(" + ", ".join(p_strs) + ").\n")

        else:
            raise ParsingException("Invalid condition type in precondion: " + \
                                   str(condition), grounding_error_code)

    def write_eff_asp(self, condition, out_file):
        """ Write the effect of this condition into asp.
            (Grounder, Condition, file) -> None
        """
        if isinstance(condition, PredicateCondition):
            if condition.sign or condition.pred in self.problem.neg_precs:
                pred_name = condition.pred.asp_name
                if not condition.sign:
                    pred_name = neg_prec_prefix + pred_name
                if condition.variables:
                    v_strs, p_strs, alph_pos = [], [], 0
                    for v in condition.variables:
                        if v in condition.relevant_vars:
                            v_strs.append(var_alphabet[alph_pos])
                            p_strs.append(var_alphabet[alph_pos])
                            alph_pos += 1
                        else:
                            p_strs.append(self.problem.objects[v].asp_name)
                    out_file.write("reachable_f(" + pred_name + \
                                   "(" + ", ".join(p_strs) + ")" + ") :- reachable(" + \
                                   condition.cond_code + "(" + ", ".join(v_strs) + ")).\n")
                else:
                    out_file.write("reachable_f(" + pred_name + ") :- reachable(" + \
                                   condition.cond_code + ").\n")

        elif isinstance(condition, AndCondition):
            if condition.relevant_vars:
                and_str = "reachable(" + condition.cond_code + "(" + \
                          ", ".join(var_alphabet[:len(condition.relevant_vars)]) + "))"
            else:
                and_str = "reachable(" + condition.cond_code + ")"
            for cond in condition.conditions:
                if cond.relevant_vars:
                    out_file.write("reachable(" + cond.cond_code + "(" + \
                                   ", ".join([var_alphabet[condition.var_indices[v]] \
                                              for v in cond.relevant_vars]) + ")) :- " + and_str + ".\n")
                else:
                    out_file.write("reachable(" + cond.cond_code + ") :- " + and_str + ".\n")

            for cond in condition.conditions:
                self.write_eff_asp(cond, out_file)

        elif isinstance(condition, ForAllCondition):
            if condition.relevant_vars:
                forall_str = "reachable(" + condition.cond_code + "(" + \
                             ", ".join(var_alphabet[:len(condition.relevant_vars)]) + "))"
            else:
                forall_str = "reachable(" + condition.cond_code + ")"
            forall_str += ", " + condition.v_type.name + "(" + \
                          var_alphabet[len(condition.relevant_vars)] + ")"
            if condition.condition.relevant_vars:
                v_str = "reachable(" + condition.condition.cond_code + "(" + \
                        ", ".join([var_alphabet[condition.var_indices[v]] \
                                       if v in condition.relevant_vars else \
                                       var_alphabet[len(condition.relevant_vars)] \
                                   for v in condition.condition.relevant_vars]) + "))"
                out_file.write(v_str + " :- " + forall_str + ".\n")
            else:
                out_file.write("reachable(" + condition.condition.cond_code + ") :- " + \
                               forall_str + ".\n")

            self.write_eff_asp(condition.condition, out_file)

        elif isinstance(condition, IncreaseCondition):
            # print "Increase: ", condition.cond_code
            pass
        elif isinstance(condition, ConditionalEffect):
            if condition.relevant_vars:
                self_str = "reachable(" + condition.cond_code + "(" + \
                           ", ".join(var_alphabet[:len(condition.relevant_vars)]) + "))"
            else:
                self_str = "reachable(" + condition.cond_code + ")"
            if condition.condition.relevant_vars:
                cond_str = "reachable(" + condition.condition.cond_code + "(" + \
                           ", ".join([var_alphabet[condition.var_indices[v]] \
                                      for v in condition.condition.relevant_vars]) + "))"
            else:
                cond_str = "reachable(" + condition.condition.cond_code + ")"
            if condition.effect.relevant_vars:
                eff_str = "reachable(" + condition.effect.cond_code + "(" + \
                          ", ".join([var_alphabet[condition.var_indices[v]] \
                                     for v in condition.effect.relevant_vars]) + "))"
            else:
                eff_str = "reachable(" + condition.effect.cond_code + ")"
            out_file.write(eff_str + " :- " + self_str + ", " + cond_str + ".\n")

            self.write_prec_asp(condition.condition, out_file)
            self.write_eff_asp(condition.effect, out_file)

        else:
            raise ParsingException("Invalid condition type in effect: " + \
                                   str(condition), grounding_error_code)

    def write_asp(self):
        """ Write an ASP representation of the problem to pre_file_name for
            grounding.

            (Grounder, file, bool) -> None
        """
        from . import asp
        problem = self.problem
        translator = asp.Translator()
        writer = asp.FileWriter(self.pre_file_name)

        # Objects
        objects = (translator.object(o) for o in problem.objects.values())
        writer.write(objects)

        # Typing
        types = filter(None, (translator.type(t, var_alphabet[0]) for t in problem.types.values()))  # Filter out Nones
        writer.write(types)

        # Actions and derived predicates
        for action in list(problem.actions.values()):
            # problem.derived_predicates.itervalues()):
            if action.parameters:
                action_str = action.asp_name + "(" + \
                             ", ".join(var_alphabet[:len(action.parameters)]) + ")"
            else:
                action_str = action.asp_name

            reachable_str = "reachable_a("

            # Precondition
            if action.precondition:
                if not action.parameters:
                    writer.writeln(reachable_str + action_str + ") :- " + \
                                   "reachable(" + action.precondition.cond_code + ").")
                else:
                    writer.write_str(reachable_str + action_str + ") :- " + \
                                   "reachable(" + action.precondition.cond_code + "(" + \
                                   ", ".join([var_alphabet[action.var_indices[v]] \
                                              for v in action.precondition.relevant_vars]) + "))")
                    for pid, (param, ptype) in enumerate(action.parameters):
                        writer.write_str(", " + ptype.asp_name + "(" + var_alphabet[pid] + ")")
                    writer.write_str(".\n")
                self.write_prec_asp(action.precondition, writer.desc())
            else:
                writer.writeln(reachable_str + action_str + ").")

            # Effect
            if not action.parameters:
                writer.writeln("reachable(" + action.effect.cond_code + ") :- " + \
                               reachable_str + action.asp_name + ").")
            else:
                eff_vars = [var_alphabet[action.var_indices[v]] \
                            for v in action.effect.relevant_vars]
                if eff_vars:
                    writer.writeln("reachable(" + action.effect.cond_code + "(" + \
                                   ", ".join(eff_vars) + ")) :- " + reachable_str + action_str + ").")
                else:
                    writer.writeln("reachable(" + action.effect.cond_code + \
                                   ") :- " + reachable_str + action_str + ").")
            self.write_eff_asp(action.effect, writer.desc())

        # start state
        for init_fact, init_args in problem.initial_state:
            if init_args:
                writer.writeln("reachable_f(" + init_fact.asp_name + "(" + \
                               ", ".join([problem.objects[v].asp_name for v in init_args]) + ")).")
            else:
                writer.writeln("reachable_f(" + init_fact.asp_name + ").")

        writer.writeln(equality_prefix + "(" + var_alphabet[0] + \
                       ", " + var_alphabet[1] + ") :- " + default_type_name + "(" + \
                       var_alphabet[0] + "), " + default_type_name + "(" + var_alphabet[1] + \
                       "), " + var_alphabet[0] + " = " + var_alphabet[1] + ".")

        # Inequalities
        for (v1, v2) in problem.inequalities:
            writer.writeln(inequality_prefix + "(" + problem.objects[v1].asp_name + \
                           ", " + problem.objects[v2].asp_name + ").")

        # negated preconditions in start state
        for pred, grounding in problem.neg_initial_state:
            if not grounding:
                writer.writeln("reachable_f(" + neg_prec_prefix + pred.asp_name + ").")
            else:
                writer.writeln("reachable_f(" + neg_prec_prefix + pred.asp_name + \
                               "(" + ", ".join([problem.objects[v].asp_name for v in grounding]) + ")).")

        # goal
        writer.writeln("reachable_goal :- " + "reachable(" + problem.goal.cond_code + ").")
        self.write_prec_asp(problem.goal, writer.desc())

        # Only show the reachability info
        writer.writeln("#show reachable_a/1.")
        writer.writeln("#show reachable_f/1.")
        writer.writeln("#show reachable/1.")
        writer.writeln("#show reachable_goal/0.")
        writer.close()

    def ground(self):
        """ Write the problem out into ASP and use Gringo to ground it and then
            read the solution back in to the problem

            (Problem, str, str) -> None
        """
        if not os.path.isfile(grounder_path):
            raise RuntimeError("In order to use the ASP-based grounder, you need to set the environment variable "
                               "'GRINGO_PATH' to the path where the 'gringo' binary is located")
        print("Using ASP grounder found at '{}'".format(grounder_path))
        self.rename_asp_components()
        self.write_asp()
        reachable_goal = False
        try:
            solver_res = subprocess.call(grounder_path + " " + self.pre_file_name + " > " + \
                                         self.grounding_file_name, shell=True)
            if solver_res != grounder_run_success_code:
                raise ParsingException("Error: There was a problem running the grounder: " + \
                                       grounder_path, grounding_error_code)

            with open(self.grounding_file_name) as grounding_file:
                print("Processing result of grounding: {}".format(self.grounding_file_name))

                for line in grounding_file:
                    if 'reachable' not in line: continue
                    line = line.strip()
                    line = line.split(' ')[2]
                    line = line.rstrip(")")

                    tokens = line.split("(")
                    if tokens[0] == "reachable":
                        cond_name = tokens[1]
                        if len(tokens) == 3:
                            cond_args = tokens[2].split(",")
                        else:
                            cond_args = []
                        condition = self.problem.code_conds[cond_name]

                        grounding = tuple(self.asp_objects[arg].name for arg in cond_args)
                        condition.groundings.append(grounding)

                        if hasattr(condition, "variables"):
                            binding = dict(zip(condition.relevant_vars, grounding))
                            condition.bindings.append(tuple(binding.get(var, var) for var in condition.variables))

                    elif tokens[0] == "reachable_a":
                        pred_name = tokens[1]
                        if len(tokens) == 3:
                            pred_args = tokens[2].split(",")
                        else:
                            pred_args = []

                        action = self.asp_actions[pred_name]
                        grounding = tuple(self.asp_objects[arg].name for arg in pred_args)
                        binding = dict(zip(action.param_names, grounding))
                        action.groundings.append(grounding)
                        action.bindings.append(tuple(binding.get(var) for var in action.param_names))

                    elif tokens[0] == "reachable_f":
                        pred_name = tokens[1]
                        if len(tokens) == 3:
                            pred_args = tokens[2].split(",")
                        else:
                            pred_args = []
                        pred_args = [self.asp_objects[arg].name for arg in pred_args]
                        if pred_name.startswith(neg_prec_prefix):
                            self.asp_predicates[pred_name[len(neg_prec_prefix):]]. \
                                neg_groundings.append(tuple(pred_args))
                        else:
                            self.asp_predicates[pred_name].groundings.append(tuple(pred_args))

                    elif tokens[0] == "reachable_goal":
                        reachable_goal = True
                    else:
                        raise ParsingException("Error: unknown line in grounding file: " + \
                                               line, grounding_error_code)

        except IOError as e:
            print(e)
            raise ParsingException("Error: could not open the grounding file: " + \
                                   self.grounding_file_name, grounding_error_code)
        except OSError as e:
            import sys
            sys.stdout.flush()
            print(e)
            raise ParsingException("Error: There was a problem running the grounder: " + \
                                   grounder_path, grounding_error_code)

        if not reachable_goal:
            raise ParsingException("Error: the goal is not relaxed reachable.",
                                   grounding_error_code)
