
#include <memory>

#include <fs/core/problem.hxx>
#include <fs/core/utils/loader.hxx>
#include <fs/core/actions/actions.hxx>
#include <fs/core/actions/grounding.hxx>
#include <fs/core/utils/component_factory.hxx>
#include <fs/core/languages/fstrips/loader.hxx>
#include <fs/core/languages/fstrips/metrics.hxx>
#include <lapkt/tools/logging.hxx>
#include <fs/core/constraints//gecode/helper.hxx>
#include <fs/core/constraints//registry.hxx>
#include <fs/core/utils/utils.hxx>
#include <fs/core/utils/printers/registry.hxx>
#include <fs/core/utils/config.hxx>
#include <fs/core/state.hxx>
#include <fs/core/fstrips/language_info.hxx>
#include <fs/core/languages/fstrips/formulae.hxx>
#include <fs/core/languages/fstrips/operations.hxx>
#include <fs/core/constraints//grounding.hxx>
#include <fs/core/validator.hxx>


namespace fs = fs0::language::fstrips;

namespace fs0 {

std::unordered_map<std::string, const fs::Axiom*>
_index_axioms(const std::vector<const fs::Axiom*>& axioms) {
	std::unordered_map<std::string, const fs::Axiom*> index;
	for (const fs::Axiom* axiom:axioms) {
		index.insert(std::make_pair(axiom->getName(), axiom));
	}
	return index;
}

// UGLY HACK. This should not be here.
bool
_check_negated_preconditions(std::vector<const ActionData*>& schemas) {
	for (const ActionData* schema:schemas) {
		for (const fs::AtomicFormula* atom:fs::all_atoms(*schema->getPrecondition())) {
			const fs::EQAtomicFormula* eq = dynamic_cast<const fs::EQAtomicFormula*>(atom);
			if (!eq) continue;
			const fs::Constant* cnst = dynamic_cast<const fs::Constant*>(eq->rhs());
			if (!cnst) continue;
			if (o_type(cnst->getValue()) == type_id::bool_t && !(fs0::value<bool>(cnst->getValue()))) {
				return true;
			}
		}
	}

	return false;
}

Problem* Loader::loadProblem(const rapidjson::Document& data) {
	const Config& config = Config::instance();
	const ProblemInfo& info = ProblemInfo::getInstance();

	LPT_INFO("main", "Creating State Indexer...");
	auto indexer = StateAtomIndexer::create(info);

	LPT_INFO("main", "Loading initial state...");
	if (!data.HasMember("init")) {
	       throw std::runtime_error("Could not find initial state in data/problem.json!");
	}
	auto init = loadState(*indexer, data["init"], info);
	LPT_DEBUG("main", *init );

	LPT_INFO("main", "Loading action data...");
	if (!data.HasMember("action_schemata")) {
	       throw std::runtime_error("Could not find action schemas in data/problem.json!");
	}
	auto control_data = loadAllActionData(data["action_schemata"], info, true);

	LPT_INFO("main", "Loading process data...");
	if (!data.HasMember("process_schemata")) {
	       throw std::runtime_error("Could not find process schemas in data/problem.json!");
	}
	auto process_data = loadAllActionData(data["process_schemata"], info, true);

	LPT_INFO("main", "Loading event data...");
	if (!data.HasMember("event_schemata")) {
	       throw std::runtime_error("Could not find events schemas in data/problem.json!");
	}
	auto events_data = loadAllActionData(data["event_schemata"], info, true);

	auto tmp = Utils::merge( control_data, process_data );
	auto action_data = Utils::merge( tmp, events_data );

	LPT_INFO("main", "Loading axiom data...");
	// Axiom schemas are simply action schemas but without effects
	auto axioms = loadAxioms(data["axioms"], info);
	if (!data.HasMember("axioms")) {
	       throw std::runtime_error("Could not find axioms schemas in data/problem.json!");
	}
	auto axiom_idx = _index_axioms(axioms);

	LPT_INFO("main", "Loading goal formula...");
	if (!data.HasMember("goal")) {
	       throw std::runtime_error("Could not find axioms schemas in data/problem.json!");
	}
	auto goal = loadGroundedFormula(data["goal"], info);

	LPT_INFO("main", "Loading state constraints...");


	std::vector<const fs::Formula*> conjuncts;
	if (!data.HasMember("state_constraints")) {
	       throw std::runtime_error("Could not find state constraints in data/problem.json!");
	}
	for ( unsigned i = 0; i < data["state_constraints"].Size(); ++i ) {
	   auto sc = loadGroundedFormula(data["state_constraints"][i], info);
		if (sc == nullptr ) continue;
		const fs::Conjunction* conj = dynamic_cast<const fs::Conjunction*>(sc);
		if ( conj == nullptr ) {
	   		conjuncts.push_back(sc);
	   		continue;
		}
		for ( auto c : conj->getSubformulae() )
		    conjuncts.push_back(c->clone());
			delete conj;
	}

	std::vector<const fs::Axiom*> sc_set;

	if ( !conjuncts.empty())
		sc_set.push_back( new fs::Axiom("variable_bounds", {}, {}, fs::BindingUnit({},{}), new fs::Conjunction(conjuncts)));

	std::vector< const fs::Axiom* > named_sc_schemata;
	for ( unsigned i = 0; i < data["state_constraints"].Size(); ++i ) {
		auto named_sc = loadNamedStateConstraint(data["state_constraints"][i], info);
		if ( named_sc == nullptr ) continue;
	   	named_sc_schemata.push_back( named_sc );
	}
	//MRJ: We ground the state constraints here and now
	std::vector< const fs::Axiom* > named_sc_grounded = AxiomGrounder::fully_ground( named_sc_schemata, info );
	for ( auto sc : named_sc_schemata )
		delete sc;
	for ( auto sc : named_sc_grounded )
		sc_set.push_back(sc);

	auto sc_idx = _index_axioms(sc_set);

	LPT_INFO("main", "Loading Problem Metric...");
	const fs::Metric* metric = data.HasMember("metric") ? loadMetric( data["metric"], info ) : nullptr;

	//! Set the global singleton Problem instance
	bool has_negated_preconditions = _check_negated_preconditions(action_data);
	LPT_INFO("main", "Quick Negated-Precondition Test: Does the problem have negated preconditions? " << has_negated_preconditions);
	LPT_INFO("main", "Atom Index: Indexing negative literals? " << has_negated_preconditions);
	// We will index the negative literals if either the problem has neg. precs, or the user explicitly wants _not_ to ignore them on novelty computations.
	bool index_negative_literals = has_negated_preconditions || !(config.getOption<bool>("ignore_neg_literals", true));
	Problem* problem = new Problem(init, indexer, action_data, axiom_idx, goal, sc_idx, metric, AtomIndex(info, index_negative_literals));
	Problem::setInstance(std::unique_ptr<Problem>(problem));

	problem->consolidateAxioms();

	LPT_INFO("components", "Bootstrapping problem with following external component repository\n" << print::logical_registry(LogicalComponentRegistry::instance()));

	if (config.validate()) {
		LPT_INFO("main", "Validating problem...");
		Validator::validate_problem(*problem, info);
	}

	return problem;
}

void
Loader::loadFunctions(const BaseComponentFactory& factory, ProblemInfo& info) {

	// First load the extensions of the static symbols
	for (auto name:info.getSymbolNames()) {
		unsigned id = info.getSymbolId(name);
		if (info.getSymbolData(id).isStatic()) {
			info.set_extension(id, StaticExtension::load_static_extension(name, info));
		}
	}

	// Load the function objects for externally-defined symbols
	for (auto elem:factory.instantiateFunctions(info)) {
		info.setFunction(info.getSymbolId(elem.first), elem.second);
	}
}

ProblemInfo&
Loader::loadProblemInfo(const rapidjson::Document& data, const std::string& data_dir, const BaseComponentFactory& factory) {
	// Load and set the ProblemInfo data structure
	auto info = std::unique_ptr<ProblemInfo>(new ProblemInfo(data, data_dir));
	loadFunctions(factory, *info);
	return ProblemInfo::setInstance(std::move(info));
}

State*
Loader::loadState(const StateAtomIndexer& indexer, const rapidjson::Value& data, const ProblemInfo& info ) {
	// The state is an array of two-sized arrays [x,v], representing atoms x=v
	unsigned numAtoms = data["variables"].GetInt();
	std::vector<Atom> facts;
	for (unsigned i = 0; i < data["atoms"].Size(); ++i) {
		const rapidjson::Value& node = data["atoms"][i];
		VariableIdx var = node[0].GetInt();
		object_id value;
		type_id var_type = info.sv_type(var);

		if (var_type == type_id::bool_t) {
			value =  make_object((bool)node[1].GetInt());
		} else if (var_type == type_id::float_t) {
			// MRJ: We're using the specialization so the floating point number
			// is stored correctly via type punning
		    value =  make_object((float)node[1].GetDouble());
		}
		else if (var_type == type_id::int_t) {
		    value =  make_object(type_id::int_t, node[1].GetInt());
		}
		else if (var_type == type_id::object_t) {
			value =  make_object(type_id::object_t, node[1].GetInt());
		}
		else {
			throw std::runtime_error("Loader::loadState() : Cannot load state variable '" + info.getVariableName(var)
									 + "' of type '" + fstrips::LanguageInfo::instance().get_typename(var) + "'");
		}

		facts.push_back(Atom(var,value));
	}
	return State::create(indexer, numAtoms, facts);
}


std::vector<const ActionData*>
Loader::loadAllActionData(const rapidjson::Value& data, const ProblemInfo& info, bool load_effects) {
	std::vector<const ActionData*> schemata;
	for (unsigned i = 0; i < data.Size(); ++i) {
		if (const ActionData* adata = loadActionData(data[i], i, info, load_effects)) {
			schemata.push_back(adata);
		}
	}
	return schemata;
}

std::vector<const fs::Axiom*>
Loader::loadAxioms(const rapidjson::Value& data, const ProblemInfo& info) {
	std::vector<const fs::Axiom*> axioms;
	for (const ActionData* action:loadAllActionData(data, info, false)) {
		axioms.push_back(new fs::Axiom(action->getName(), action->getSignature(), action->getParameterNames(), action->getBindingUnit(), action->getPrecondition()->clone()));
		delete action;
	}
	return axioms;
}

const ActionData*
Loader::loadActionData(const rapidjson::Value& node, unsigned id, const ProblemInfo& info, bool load_effects) {
	const std::string& name = node["name"].GetString();
	const Signature signature = parseNumberList<unsigned>(node["signature"]);
	const std::vector<std::string> parameters = parseStringList(node["parameters"]);
	const fs::BindingUnit unit(parameters, fs::Loader::parseVariables(node["unit"], info));

    //! MRJ: this method is being re-used to load axioms and state constraints, so the "type"
    //! member is actually optional
    ActionData::Type action_type = ActionData::Type::Control;
    if ( node.HasMember("type") ) {
        const std::string& action_type_str = node["type"].GetString();
        if ( action_type_str == "control" )
            action_type = ActionData::Type::Control;
        else if ( action_type_str == "exogenous" )
            action_type = ActionData::Type::Exogenous;
        else if ( action_type_str == "natural" )
            action_type = ActionData::Type::Natural;
        else {
            throw std::runtime_error("Action '" + name +"' has unrecognized type '" + action_type_str + "'");
        }
    }

	const fs::Formula* precondition = fs::Loader::parseFormula(node["conditions"], info);
	std::vector<const fs::ActionEffect*> effects;

	if (load_effects) {
		effects = fs::Loader::parseEffectList(node["effects"], info);
	}

	ActionData adata(id, name, signature, parameters, unit, precondition, effects, action_type);
	if (adata.has_empty_parameter()) {
		LPT_INFO("main", "Schema \"" << adata.getName() << "\" discarded because of empty parameter type.");
		return nullptr;
	}
	LPT_DEBUG("loader", "Loaded action data: " << adata );

	// We perform a first binding on the action schema so that state variables, etc. get consolidated, but the parameters remain the same
	// This is possibly not optimal, since for some configurations we might be duplicating efforts, but ATM we are happy with it
	return ActionGrounder::process_action_data(adata, info, load_effects);
}

const fs::Metric*
Loader::loadMetric( const rapidjson::Value& data, const ProblemInfo& info ) {
	// Get first the type of the metric
	if ( !data.HasMember("optimization")) return nullptr; // Empty metric

	MetricType optMode;
	std::string optModeString( data["optimization"].GetString() );
	if ( optModeString == "minimize" )
		optMode = MetricType::MINIMIZE;
	else if ( optModeString == "maximize" )
		optMode = MetricType::MAXIMIZE;
	else {
		std::string errorMsg( "Unrecognized metric optimization mode ");
		throw std::runtime_error(  errorMsg + optModeString );
	}

	LPT_INFO( "main", "Metric optimization mode is " << data["optimization"].GetString() );
	auto expr = fs::Loader::parseTerm( data["expression"], info );
    auto processed = fs::bind(*expr, Binding::EMPTY_BINDING, info);

	LPT_INFO( "main", "Metric loaded: " << processed );
    delete expr;
	return new fs::Metric(optMode, processed );
}


const fs::Formula*
Loader::loadGroundedFormula(const rapidjson::Value& data, const ProblemInfo& info) {
    if (data.HasMember("name")) return nullptr;
    assert( data.HasMember("conditions"));
    const fs::Formula* unprocessed = fs::Loader::parseFormula(data["conditions"], info);
    // The conditions are by definition already grounded, and hence we need no binding, but we process the formula anyway
    // to detect tautologies, contradictions, etc., and to consolidate state variables
    auto processed = fs::bind(*unprocessed, Binding::EMPTY_BINDING, info);
    delete unprocessed;
    return processed;
}

const fs::Axiom*
Loader::loadNamedStateConstraint(const rapidjson::Value& data, const ProblemInfo& info) {
    std::vector<const fs::Axiom*> axioms;
    if ( !data.HasMember("name")) {
        return nullptr;
    }
    const ActionData* adata = loadActionData(data, 0, info, false);
    if (adata != nullptr )
        return new fs::Axiom(adata->getName(), adata->getSignature(), adata->getParameterNames(), adata->getBindingUnit(), adata->getPrecondition()->clone());
    assert(false);
    return nullptr;
}

rapidjson::Document
Loader::loadJSONObject(const std::string& filename) {
	// Load and parse the JSON data file.
	std::ifstream in(filename);
	if (in.fail()) throw std::runtime_error("Could not open filename '" + filename + "'");
	std::string str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	rapidjson::Document data;
	data.Parse(str.c_str());
	return data;
}


std::vector<std::string>
Loader::parseStringList(const rapidjson::Value& data) {
	std::vector<std::string> output;
	for (unsigned i = 0; i < data.Size(); ++i) {
		output.push_back(data[i].GetString());
	}
	return output;
}


template<typename T>
std::vector<std::vector<T>> Loader::parseDoubleNumberList(const rapidjson::Value& data) {
	std::vector<std::vector<T>> output;
	assert(data.IsArray());
	if (data.Size() == 0) {
		output.push_back(std::vector<T>());
	} else {
		for (unsigned i = 0; i < data.Size(); ++i) {
			output.push_back(parseNumberList<T>(data[i]));
		}
	}
	return output;
}

} // namespaces
