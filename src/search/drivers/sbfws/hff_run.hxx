
#pragma once

#include <iomanip>
#include <vector>

#include <interfaces/agnostic/types.hxx>
#include <interfaces/ff-wrapped/ff_to_aptk.hxx>
#include <interfaces/agnostic/strips_prob.hxx>
#include <interfaces/agnostic/fwd_search_prob.hxx>
#include <interfaces/agnostic/h_1.hxx>
#include <interfaces/agnostic/action.hxx>
#include <interfaces/agnostic/cond_eff.hxx>

#include "nir/rp_heuristic_bfws.hxx"
#include <state.hxx>


#include <aptk2/tools/logging.hxx>
#include <search/drivers/sbfws/relevant_atomset.hxx>


namespace fs0 { namespace bfws {

using aptk::STRIPS_Problem;
using aptk::agnostic::Fwd_Search_Problem;
using aptk::agnostic::H1_Heuristic;
using aptk::agnostic::H_Add_Evaluation_Function;
using aptk::agnostic::Relaxed_Plan_Heuristic;



class HFFRun {
public:
	typedef typename Fwd_Search_Problem::State_Type State;
	using H_Add_Fwd = H1_Heuristic<Fwd_Search_Problem, H_Add_Evaluation_Function>;
	using H_Add_Rp_Fwd = Relaxed_Plan_Heuristic< Fwd_Search_Problem, H_Add_Fwd>;

	//! Factory method
	static HFFRun* create(const std::string& domain_file, const std::string& instance_file, bool one_HA_per_fluent) {
		return new HFFRun(load_problem(domain_file, instance_file), one_HA_per_fluent);
	}
	
	static Fwd_Search_Problem load_problem(const std::string& domain_file, const std::string& instance_file) {
		STRIPS_Problem prob;
		aptk::FF_Parser::get_problem_description(domain_file, instance_file, prob, true);
		std::cout << "PDDL problem description loaded: " << std::endl;
		std::cout << "\tDomain: " << prob.domain_name() << std::endl;
		std::cout << "\tProblem: " << prob.problem_name() << std::endl;
		std::cout << "\t#Actions: " << prob.num_actions() << std::endl;
		std::cout << "\t#Fluents: " << prob.num_fluents() << std::endl;

		prob.compute_edeletes();
		return Fwd_Search_Problem(&prob);
	}
	
	HFFRun(Fwd_Search_Problem&& problem, bool one_HA_per_fluent) :
		_problem(std::move(problem)),
		_hff_heuristic(_problem),
		_one_HA_per_fluent(one_HA_per_fluent)
	{
		_hff_heuristic.ignore_rp_h_value(true);
		_hff_heuristic.set_one_HA_per_fluent(one_HA_per_fluent);
	}
	
	inline const STRIPS_Problem& task() const { return _problem.task(); }
	
	aptk::State _convert_state(const fs0::State& state) {
		aptk::State s(task());
		
		assert(0);
		
		return s;
	}

	
	//! Compute the set or relevant atoms R_{FF}(s)
	RelevantAtomSet compute_r_ff(const fs0::State& s, const AtomIndex& atomidx) {
		aptk::Fluent_Vec rp_vec;
		aptk::Fluent_Set rp_set(task().num_fluents());
		
		compute_r_ff(_convert_state(s), rp_vec, rp_set);
		
		RelevantAtomSet atomset(&atomidx);
		
		assert(0);
		
		return atomset;
	}
	
	
	
	void compute_r_ff(const aptk::State& s, aptk::Fluent_Vec& rp_vec, aptk::Fluent_Set& rp_set) {

		std::vector<aptk::Action_Idx> po;
		std::vector<aptk::Action_Idx> relaxed_plan;
		

		// Compute the relaxed plan itself
		float h = 0;
		_hff_heuristic.ignore_rp_h_value(true);
		_hff_heuristic.eval(s, h, po, relaxed_plan);
		std::cout << "relaxed_plan size: "<< relaxed_plan.size() << " "<<std::flush;

		for (auto action_i:relaxed_plan) {
			const aptk::Action* a = task().actions()[action_i];

			// Add Conditional Effects
			if(!a->ceff_vec().empty()) {
				for( unsigned i = 0; i < a->ceff_vec().size(); i++ ){
					aptk::Conditional_Effect* ce = a->ceff_vec()[i];
					for (auto p : ce->add_vec()) {
						if ( ! rp_set.isset( p ) ){
							rp_vec.push_back( p );
							rp_set.set( p );
							//std::cout << task().fluents()[add[i]]->signature() << std::endl;
						}
					}
				}
			}

			const aptk::Fluent_Vec& add = a->add_vec();

			//std::cout << task().actions()[action_i]->signature() << std::endl;
			for (unsigned i = 0; i < add.size(); i++) {
				if ( ! rp_set.isset( add[i] ) ) {
					rp_vec.push_back( add[i] );
					rp_set.set( add[i] );
					//std::cout << task().fluents()[add[i]]->signature() << std::endl;
				}
			}
		}
	}


protected:
	Fwd_Search_Problem _problem;
	
	H_Add_Rp_Fwd _hff_heuristic;
	
	bool _one_HA_per_fluent;
};

} } // namespaces
