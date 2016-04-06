	
#include <languages/fstrips/language.hxx>
#include <problem.hxx>
#include <actions/actions.hxx>
#include <constraints/gecode/handlers/effect_schema_handler.hxx>
#include <constraints/gecode/utils/novelty_constraints.hxx>
#include <constraints/gecode/supports.hxx>
#include <utils/printers/actions.hxx>
#include <utils/logging.hxx>
#include <utils/tuple_index.hxx>
#include <heuristics/relaxed_plan/rpg_index.hxx>
#include <gecode/int.hh>
#include <gecode/search.hh>

namespace fs0 { namespace gecode {


std::vector<std::shared_ptr<EffectSchemaCSPHandler>> EffectSchemaCSPHandler::create(const std::vector<const PartiallyGroundedAction*>& schemata, const TupleIndex& tuple_index, bool approximate, bool novelty) {
	std::vector<std::shared_ptr<EffectSchemaCSPHandler>> handlers;
	
	for (const PartiallyGroundedAction* schema:schemata) {
		for (unsigned eff_idx = 0; eff_idx < schema->getEffects().size(); ++eff_idx) {
			const fs::ActionEffect* effect = schema->getEffects().at(eff_idx);
			auto handler = std::make_shared<EffectSchemaCSPHandler>(*schema, effect, tuple_index, approximate);
			handler->init(novelty);
			FDEBUG("main", "Generated CSP for the effect #" << eff_idx << " of action " << *schema << std::endl <<  *handler << std::endl);
			handlers.push_back(handler);
		}
	}
	return handlers;
}

EffectSchemaCSPHandler::EffectSchemaCSPHandler(const PartiallyGroundedAction& action, const fs::ActionEffect* effect, const TupleIndex& tuple_index, bool approximate) :
	ActionSchemaCSPHandler(action, { effect }, tuple_index, approximate), _achievable_tuple_idx(INVALID_TUPLE)
{}

EffectSchemaCSPHandler::~EffectSchemaCSPHandler() {
	delete _effect_novelty;
}

void EffectSchemaCSPHandler::init(bool use_novelty_constraint) {
	ActionSchemaCSPHandler::init(use_novelty_constraint);
	
	_lhs_symbol = index_lhs_symbol(get_effect());
	_rhs_variable = _translator.resolveVariableIndex(get_effect()->rhs(), CSPVariableType::Input);
	_effect_tuple = index_tuple_indexes(get_effect());
	_achievable_tuple_idx = detect_achievable_tuple();

	// Register all fluent symbols involved
	_tuple_indexes = _translator.index_fluents(_all_terms);
}

TupleIdx EffectSchemaCSPHandler::detect_achievable_tuple() const {
	const ProblemInfo& info = Problem::getInfo();
	
	TupleIdx achievable_tuple_idx = INVALID_TUPLE;
	
	// We necessarily assume that the head of the effect is fluent-less
	if (info.isPredicate(_lhs_symbol)) { // If the effect is predicative, it must be an add-effect
		achievable_tuple_idx = _tuple_index.to_index(_lhs_symbol, _effect_tuple);
	} else {
		auto constant = dynamic_cast<const fs::Constant*>(get_effect()->rhs());
		if (constant) {
			ValueTuple tuple(_effect_tuple);
			tuple.push_back(constant->getValue());
			achievable_tuple_idx = _tuple_index.to_index(_lhs_symbol, tuple);
		}
	}
	
	return achievable_tuple_idx;
}

ValueTuple EffectSchemaCSPHandler::index_tuple_indexes(const fs::ActionEffect* effect) {
	auto lhs_statevar = check_valid_effect(effect);
	
	ValueTuple variables;
	for (auto subterm:lhs_statevar->getSubterms()) {
		auto constant = dynamic_cast<const fs::Constant*>(subterm);
		assert(constant);  // Otherwise this could not be a state variable
		variables.push_back(constant->getValue());
	}
	return variables;
}

void EffectSchemaCSPHandler::log() const {
	assert(_effects.size() == 1);
	FFDEBUG("heuristic", "Processing effect schema \"" << *get_effect() << " of action " << _action);
}

const fs::ActionEffect* EffectSchemaCSPHandler::get_effect() const { 
	assert(_effects.size() == 1);
	return _effects[0];
}

unsigned EffectSchemaCSPHandler::index_lhs_symbol(const fs::ActionEffect* effect) {
	return check_valid_effect(effect)->getSymbolId();
}

const fs::StateVariable* EffectSchemaCSPHandler::check_valid_effect(const fs::ActionEffect* effect) {
	auto lhs_statevar = dynamic_cast<const fs::StateVariable*>(effect->lhs());
	if (!lhs_statevar) throw std::runtime_error("EffectSchemaCSPHandler accepts only effects with state-variable (fluent-less) heads");
	return lhs_statevar;
}


void EffectSchemaCSPHandler::seek_novel_tuples(RPGIndex& rpg, const State& seed) const {
	if (SimpleCSP* csp = instantiate_effect_csp(rpg)) {
		if (!csp->checkConsistency()) {
			FFDEBUG("heuristic", "The effect CSP cannot produce any new tuple");
		}
		else {
		
			Gecode::DFS<SimpleCSP> engine(csp);
			unsigned num_solutions = 0;
			while (SimpleCSP* solution = engine.next()) {
		// 		FFDEBUG("heuristic", std::endl << "Processing action CSP solution #"<< num_solutions + 1 << ": " << print::csp(_translator, *solution))
				process_effect_solution(solution, rpg);
				++num_solutions;
				delete solution;
			}
			FFDEBUG("heuristic", "The Effect CSP produced " << num_solutions << " novel tuples");
		}
		delete csp;
	}
}

TupleIdx EffectSchemaCSPHandler::compute_reached_tuple(const SimpleCSP* solution) const {
	TupleIdx tuple_idx = _achievable_tuple_idx;
	if (tuple_idx == INVALID_TUPLE) { // i.e. we have a functional effect, and thus need to factor the function result into the tuple.
		ValueTuple tuple(_effect_tuple); // Copy the tuple
		tuple.push_back(_translator.resolveValueFromIndex(_rhs_variable, *solution));
		tuple_idx = _tuple_index.to_index(_lhs_symbol, tuple);
	}
	return tuple_idx;
}

void EffectSchemaCSPHandler::process_effect_solution(const SimpleCSP* solution, RPGIndex& rpg) const {
	TupleIdx tuple_idx = compute_reached_tuple(solution);
	
	bool reached = rpg.reached(tuple_idx);
	FFDEBUG("heuristic", "Processing effect \"" << *get_effect() << "\" produces " << (reached ? "repeated" : "new") << " tuple " << tuple_idx);
	
	if (reached) return; // The value has already been reached before
	
	// Otherwise, the value is actually new - we extract the actual support from the solution
	std::vector<TupleIdx> support = Supports::extract_support(solution, _translator, _tuple_indexes);
	rpg.add(tuple_idx, get_action_id(solution), std::move(support));
}


void EffectSchemaCSPHandler::create_novelty_constraint() {
	_effect_novelty = new EffectNoveltyConstraint(_translator, get_effect());
}

SimpleCSP::ptr EffectSchemaCSPHandler::instantiate_effect_csp(const RPGIndex& rpg) const {
	if (_failed) return nullptr;
	SimpleCSP* clone = static_cast<SimpleCSP::ptr>(_base_csp.clone());
	_translator.updateStateVariableDomains(*clone, rpg.get_domains());
	for (const ExtensionalConstraint& constraint:_extensional_constraints) {
		if (!constraint.update(*clone, _translator, rpg)) {
			delete clone;
			return nullptr;
		}
	}
	// Post the novelty constraint
	if (_effect_novelty) _effect_novelty->post_constraint(*clone, rpg);
	return clone;
}




} } // namespaces
