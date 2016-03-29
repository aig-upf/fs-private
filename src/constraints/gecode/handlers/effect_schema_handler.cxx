	
#include <languages/fstrips/language.hxx>
#include <actions/action_schema.hxx>
#include <constraints/gecode/handlers/effect_schema_handler.hxx>
#include <constraints/gecode/utils/indexed_tupleset.hxx>
#include <utils/printers/actions.hxx>
#include <utils/logging.hxx>
#include <heuristics/relaxed_plan/rpg_data.hxx>
#include <problem.hxx>
#include <gecode/int.hh>
#include <gecode/search.hh>

namespace fs0 { namespace gecode {

std::vector<std::shared_ptr<BaseActionCSPHandler>> EffectSchemaCSPHandler::create(const std::vector<const ActionSchema*>& schemata, const std::vector<IndexedTupleset>& symbol_tuplesets, bool approximate, bool novelty) {
	// Simply upcast the shared_ptrs
	std::vector<std::shared_ptr<BaseActionCSPHandler>> handlers;
	for (const auto& element:create_derived(schemata, symbol_tuplesets, approximate, novelty)) {
		handlers.push_back(std::static_pointer_cast<BaseActionCSPHandler>(element));
	}
	return handlers;
}

std::vector<std::shared_ptr<EffectSchemaCSPHandler>> EffectSchemaCSPHandler::create_derived(const std::vector<const ActionSchema*>& schemata, const std::vector<IndexedTupleset>& symbol_tuplesets, bool approximate, bool novelty) {
	std::vector<std::shared_ptr<EffectSchemaCSPHandler>> handlers;
	
	for (const ActionSchema* schema:schemata) {
		for (unsigned eff_idx = 0; eff_idx < schema->getEffects().size(); ++eff_idx) {
			const fs::ActionEffect* effect = schema->getEffects().at(eff_idx);
			auto handler = std::make_shared<EffectSchemaCSPHandler>(*schema, effect, approximate);
			handler->init(novelty, symbol_tuplesets.at(index_lhs_symbol(effect)));
			FDEBUG("main", "Generated CSP for the effect #" << eff_idx << " of action " << schema->fullname() << std::endl <<  *handler << std::endl);
			handlers.push_back(handler);
		}
	}
	return handlers;
}

EffectSchemaCSPHandler::EffectSchemaCSPHandler(const ActionSchema& action, const fs::ActionEffect* effect, bool approximate) :
	ActionSchemaCSPHandler(action, { effect }, approximate)
{}

void EffectSchemaCSPHandler::init(bool use_novelty_constraint, const IndexedTupleset& tupleset) {
	// Register the two CSP variables necessary for the indexing extensional constraint
	_tuple_index_var = _translator.registerIntVariable(0, tupleset.size()-1);
	_novelty_bool_var = _translator.create_bool_variable();
	
	ActionSchemaCSPHandler::init(use_novelty_constraint);
	
	_lhs_symbol = index_lhs_symbol(get_effect());
	_lhs_argument_variables = index_lhs_variables(get_effect());
	_rhs_variable = index_rhs_term(get_effect());
	
	// Register the indexing constraint itself
	register_indexing_constraint(tupleset.get_tupleset());
}

void EffectSchemaCSPHandler::log() const {
	assert(_effects.size() == 1);
	FFDEBUG("heuristic", "Processing effect schema \"" << *_effects.at(0) << " of action " << _action.fullname());
}

const fs::ActionEffect* EffectSchemaCSPHandler::get_effect() const { 
	assert(_effects.size() == 1);
	return _effects[0];
}

std::vector<unsigned> EffectSchemaCSPHandler::index_lhs_variables(const fs::ActionEffect* effect) {
	std::vector<unsigned> variables;
	const auto lhs = effect->lhs();
	auto lhs_statevar = dynamic_cast<const fs::StateVariable*>(lhs);
	auto lhs_nested = dynamic_cast<const fs::NestedTerm*>(lhs);
	if (!lhs_statevar && !lhs_nested) throw std::runtime_error("Invalid effect LHS");
	
	std::vector<const fs::Term*> subterms; // TODO - FIX THIS HACK
	if (lhs_statevar) subterms = lhs_statevar->getSubterms();
	if (lhs_nested) subterms = lhs_nested->getSubterms();	
	
	
	for (auto subterm:subterms) {
		variables.push_back(_translator.resolveVariableIndex(subterm, CSPVariableType::Input));
	}
	return variables;
}

unsigned EffectSchemaCSPHandler::index_lhs_symbol(const fs::ActionEffect* effect) {
	const auto lhs = effect->lhs();
	auto lhs_statevar = dynamic_cast<const fs::StateVariable*>(lhs);
	auto lhs_nested = dynamic_cast<const fs::NestedTerm*>(lhs);
	if (!lhs_statevar && !lhs_nested) throw std::runtime_error("Invalid effect LHS");
	
	if (lhs_statevar) return lhs_statevar->getSymbolId();
	// else lhs_nested
	return lhs_nested->getSymbolId();
}

unsigned EffectSchemaCSPHandler::index_rhs_term(const fs::ActionEffect* effect) {
	return _translator.resolveVariableIndex(effect->rhs(), CSPVariableType::Input);
}

/*
SimpleCSP* EffectSchemaCSPHandler::post_novelty(const SimpleCSP& csp, const Gecode::TupleSet& extension) {
	// For an effect f(t_1, ..., t_n) := w, we constrain the variables for the values <t_1, ..., t_n, w>
	// to be among the given tupleset, which should contains all values that have not been reached yet for symbol f.
	
	SimpleCSP* clone = static_cast<SimpleCSP*>(csp.clone());
	
	Gecode::IntVarArgs variables;
	for (unsigned variable:_lhs_argument_variables) {
		variables << _translator.resolveVariableFromIndex(variable, *clone);
	}
	
	variables << _translator.resolveVariableFromIndex(_rhs_variable, *clone);
	
	Gecode::extensional(*clone, variables, extension);
	return clone;
}
*/

/*
bool EffectSchemaCSPHandler::solve(const State& seed, SimpleCSP& csp, RPGData& bookkeeping, gecode::Tupleset& extension) {
	log();
	
	if (!csp.checkConsistency()) { // This colaterally enforces propagation of constraints
		FFDEBUG("heuristic", "No novel atom can be derived from effect CSP " << *get_effect());
		return false;
	} 
	
	// Else, the CSP is locally consistent
	if (_approximate) throw std::runtime_error("Unimplemented");
	
	// We just want to search for one solution an extract the support from it
	Gecode::DFS<SimpleCSP> engine(&csp);
	SimpleCSP* solution = engine.next();
	if (!solution) return false; // The CSP has no solution at all
	
	process_solution(solution, bookkeeping); // TODO - This is not optimal, but for the moment being it saves us a lot of code duplication
	
	// Retrieve the tuple that has been found and erase it from the set of unreached tuples
	std::vector<ObjectIdx> tuple;
	for (unsigned variable:_lhs_argument_variables) {
		tuple.push_back(_translator.resolveValueFromIndex(variable, *solution));
	}	
	tuple.push_back(_translator.resolveValueFromIndex(_rhs_variable, *solution)); // The RHS
	
	extension.remove(tuple);

	delete solution;
	return true;
}
*/


void EffectSchemaCSPHandler::seek_novel_tuples(const GecodeRPGLayer& layer, std::set<unsigned>& reached, RPGData& bookkeeping, const State& seed) const {
	
	SimpleCSP* csp = instantiate_csp(layer);
	if (!csp || !csp->checkConsistency()) return; // The effect CSP is not applicable
	
	
	auto& index_var = _translator.resolveVariableFromIndex(_tuple_index_var, *csp);
// 	auto& reification_var = csp->_boolvars[_novelty_bool_var];

// 	Gecode::dom(*csp, index_var, reached, reification_var);  // b <=> I \in Reached(f)
// 	Gecode::rel(*csp, Gecode::IRT_EQ, reification_var, 0); // NOT b  (i.e.... I \not \in Reached(f)
	
	for (unsigned val:reached) {
		Gecode::rel(*csp, index_var, Gecode::IRT_NQ, val); // I != index of the tuple we just found
	}
	
	if (!csp->checkConsistency()) {
		FFDEBUG("heuristic", "The effect CSP cannot produce any new tuple");
		return;
	}
	
	unsigned num_solutions = 0;
	
	int novel_tuple_idx = seek_single_solution(*csp, bookkeeping, seed);
	while (novel_tuple_idx != -1) {
// 		assert(reached.find(novel_tuple_idx) != reached.end());
		Gecode::rel(*csp, index_var, Gecode::IRT_NQ, novel_tuple_idx); // I != index of the tuple we just found
		reached.insert(novel_tuple_idx);
		novel_tuple_idx = seek_single_solution(*csp, bookkeeping, seed);
		++num_solutions;
	}
	
	FFDEBUG("heuristic", "The Effect CSP produced " << num_solutions << " novel tuples");	
}


int EffectSchemaCSPHandler::seek_single_solution(SimpleCSP& csp, RPGData& bookkeeping, const State& seed) const {
	Gecode::DFS<SimpleCSP> engine(&csp);
	SimpleCSP* solution = engine.next();
	if (!solution) return -1; // The CSP has no solution at all
	
	process_solution(solution, bookkeeping); // TODO - This is not optimal, but for the moment being it saves us a lot of code duplication
	
	/*
	PartialAssignment assignment = _translator.buildAssignment(*solution);
	Binding binding = build_binding_from_solution(solution);
	auto all_effects = _action.getEffects();
	for (unsigned i = 0; i < all_effects.size(); ++i) {
		auto eff = all_effects[i];
		if (get_effect() == eff) continue;
		VariableIdx affected = eff->lhs()->interpretVariable(assignment, binding);
		ObjectIdx value = eff->rhs()->interpret(assignment, binding);
		Atom atom(affected, value);
// 		std::cout << "Derived novel atom: " << Atom(affected, value) << std::endl;
// 		simple_atom_processing(solution, bookkeeping, atom, i, assignment, binding);
		auto hint = bookkeeping.getInsertionHint(atom);

		if (hint.first) { // The value is actually new - let us compute the supports, i.e. the CSP solution values for each variable relevant to the effect.
			Atom::vctrp support = extract_support_from_solution(solution, 0, assignment, binding);
			bookkeeping.add(atom, get_action_id(solution), support, hint.second);
		}		
		
		extra.push_back(std::move(atom));
	}
	*/
	
	// Retrieve the index of the newly foung tuple
	int tuple_idx = _translator.resolveValueFromIndex(_tuple_index_var, *solution);
	delete solution;
	return tuple_idx;
}

void EffectSchemaCSPHandler::register_indexing_constraint(const Gecode::TupleSet& index_extension) {
	const ProblemInfo& info = Problem::getInfo();

	Gecode::IntVarArgs variables;
	
	for (unsigned variable:_lhs_argument_variables) {
		variables << _translator.resolveVariableFromIndex(variable, _base_csp);
	}
	
	if (!info.isPredicate(get_lhs_symbol())) { // If we have a function, we take into account the function result.
		variables << _translator.resolveVariableFromIndex(_rhs_variable, _base_csp);
	}
	
	variables << _translator.resolveVariableFromIndex(_tuple_index_var, _base_csp);
	
	Gecode::extensional(_base_csp, variables, index_extension);
	
	Gecode::SpaceStatus st = _base_csp.status();
	_unused(st);
	assert(st != Gecode::SpaceStatus::SS_FAILED); // This should never happen, as the indexing constraint should not restrict any possible solution
}

} } // namespaces
