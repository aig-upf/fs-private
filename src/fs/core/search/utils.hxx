
#pragma once

#include <string>
#include <boost/lexical_cast.hpp>

#include <lapkt/tools/resources_control.hxx>
#include <lapkt/tools/logging.hxx>

#include <fs/core/fs_types.hxx>
#include <fs/core/languages/fstrips/language.hxx>
#include <fs/core/problem.hxx>
#include <fs/core/state.hxx>
#include <fs/core/search/stats.hxx>
#include <fs/core/search/options.hxx>
#include <fs/core/actions/checker.hxx>
#include <fs/core/utils/printers/printers.hxx>
#include <fs/core/utils/system.hxx>

namespace fs0::drivers {

class Utils {
public:

template <typename StatsT>
static void dump_stats(std::ofstream& out, const StatsT& stats) {
    for (const auto& point:stats.dump()) {
        std::string val = std::get<2>(point);

        try { auto _ = boost::lexical_cast<double>(val); _unused(_); }
        catch(boost::bad_lexical_cast& e) {
            // Not a number, we print a string
            val = "\"" + val + "\"";
        }

        out << "\t\"" << std::get<0>(point) << "\": " << val << "," << std::endl;
    }
}

    template <typename StateModelT>
    class SearchExecution {
    protected:
        const StateModelT& _model;
        const Problem& _problem;
    public:
        //! The type of any plan
        using PlanT = std::vector<typename StateModelT::ActionType::IdType>;

        explicit SearchExecution(const StateModelT& model) :
                _model(model),
                _problem(model.getTask())
        {}

        template <typename SearchAlgorithmT, typename StatsT>
        ExitCode do_search(SearchAlgorithmT& engine, const fs0::drivers::EngineOptions& options, float start_time, StatsT& stats, bool actionless = false) {
            const std::string& out_dir = options.getOutputDir();
            LPT_INFO("cout", "Starting search. Results written to " << out_dir);
            std::string plan_filename = options.getPlanfile();
            if (plan_filename.empty()) {
                plan_filename = out_dir + "/first.plan";
            }
            LPT_INFO("cout", "Plan will be output to " << plan_filename);

            std::ofstream json_out(out_dir + "/results.json");

            PlanT plan;
            double t0 = aptk::time_used();
            stats.log_start_of_search(t0);
            bool solved = false;
            bool valid_plan = false;

            if (actionless) {

                if (_model.goal(_problem.getInitialState())) {
                    LPT_INFO("cout", "The problem has no actions, and the initial state is a goal state");
                    solved = true;
                    // Validating CSP solutions might be too hard, and we would in fact be doing
                    // exactly the same twice. We should sought better ways of doing this
                    valid_plan = true;
                } else {
                    LPT_INFO("cout", "The problem has no actions, and the initial state is not a goal state");
                    solved = false;
                }
            } else {
                try {
                    solved = engine.solve_model(plan);
                }
                catch (const std::bad_alloc &ex) {
                    out_of_memory_handler();
                    exit_with(ExitCode::SEARCH_OUT_OF_MEMORY);
                } catch (const Gecode::MemoryExhausted& ex) {
                    out_of_memory_handler();
                    exit_with(ExitCode::SEARCH_OUT_OF_MEMORY);
                }
            }

            double search_time = aptk::time_used() - t0;
            double total_planning_time = aptk::time_used() - start_time;


            if (solved) {
                std::ofstream plan_out(plan_filename);
                PlanPrinter::print(plan, plan_out);
                if (!valid_plan) {
                    valid_plan = Checker::check_correctness(_problem, plan, _problem.getInitialState());
                }
                plan_out.close();
            }

            float genrate = (search_time > 0) ? (float) stats.generated() / search_time : 0.0;
            float evalrate = (search_time > 0) ? (float) stats.evaluated() / search_time : 0.0;


            json_out << "{" << std::endl;
            dump_stats(json_out, stats);
            json_out << "\t\"time_backend\": " << total_planning_time << "," << std::endl;
            json_out << "\t\"search_time\": " << search_time << "," << std::endl;
            // json_out << "\t\"search_time_alt\": " << _search_time << "," << std::endl;
            json_out << "\t\"memory\": " << get_peak_memory_in_kb() << "," << std::endl;
            json_out << "\t\"gen_per_second\": " << std::fixed << std::setprecision(2) << genrate << "," << std::endl;
            json_out << "\t\"eval_per_second\": " << std::fixed << std::setprecision(2) << evalrate << "," << std::endl;
            json_out << "\t\"solved\": " << ( solved ? "true" : "false" ) << "," << std::endl;
            json_out << "\t\"valid\": " << ( valid_plan ? "true" : "false" ) << "," << std::endl;
            json_out << "\t\"plan_length\": " << plan.size() << "," << std::endl;
            json_out << "\t\"plan\": ";
            PlanPrinter::print_json( plan, json_out);
            json_out << std::endl;
            json_out << "}" << std::endl;
            json_out.close();

            for (const auto& point:stats.dump()) {
                LPT_INFO("cout", std::get<1>(point) << ": " << std::get<2>(point));
            }
            LPT_INFO("cout", "Generation rate (nodes/sec): " << std::fixed << std::setprecision(2) << genrate);
            LPT_INFO("cout", "Total Backend Time: " << total_planning_time << " s.");
            LPT_INFO("cout", "Search Time: " << search_time << " s.");
            LPT_INFO("cout", "Peak mem. usage: " << get_peak_memory_in_kb() << " kB.");

            ExitCode result;
            if (solved) {
                if (!valid_plan) {
#ifdef DEBUG
                    LPT_INFO("cout", "ERROR: The plan output by the planner is not correct!");
                    Checker::debug_plan_execution(_problem, plan, _problem.getInitialState());
#else
                    Checker::debug_plan_execution(_problem, plan, _problem.getInitialState());
                    throw std::runtime_error("The plan output by the planner is not correct!");
#endif
                }
                LPT_INFO("cout", "Search Result: Found plan of length " << plan.size());

                char resolved_path[PATH_MAX];
                char* tmp = realpath(plan_filename.c_str(), resolved_path);
                if (tmp == nullptr) {
                    std::string error_message( strerror( errno ));
                    throw std::runtime_error("Could not resolve the real path for the plan filename: \n" + error_message );
                }
                LPT_INFO("cout", "Plan was saved in file \"" << resolved_path << "\"");

                result = ExitCode::SUCCESS;
            } else {
                result = ExitCode::SEARCH_UNSOLVABLE;
            }

            return result;
        }
    };
};

} // namespaces
