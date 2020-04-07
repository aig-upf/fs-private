
#include "action_schema_csp.hxx"
#include "constraints.hxx"
#include <fs/core/constraints/gecode/extensions.hxx>
#include <fs/core/utils/lexical_cast.hxx>
#include <fs/core/utils/system.hxx>
#include <fs/core/problem_info.hxx>
#include <fs/core/constraints/gecode/v2/gecode_space.hxx>

#include <lapkt/tools/logging.hxx>

#include <boost/algorithm/string.hpp>



namespace fs0::gecode::v2 {

//! Map a given relation string such as ">=" into the corresponding Gecode IntRelType object.
inline Gecode::IntRelType gecode_symbol(const std::string& symbol) {
    static const std::unordered_map<std::string, Gecode::IntRelType> symbol_to_gecode = {
            {"=", Gecode::IRT_EQ},
            {"!=", Gecode::IRT_NQ},
            {"<", Gecode::IRT_LE},
            {"<=", Gecode::IRT_LQ},
            {">", Gecode::IRT_GR},
            {">=", Gecode::IRT_GQ}
    };

    return symbol_to_gecode.at(symbol);
}

int retrieve_int_variable_index(
        const std::string& name,
        const std::unordered_map<std::string, FSGecodeSpace::var_id>& varidx) {
    auto it = varidx.find(name);
    if (it == varidx.end()) {
        std::cerr << "ActionSchemaCSP: Failed to find variable with name '" << name
                  << "' among " << varidx.size() << " registered variable names" << std::endl;
        exit_with(ExitCode::SEARCH_INPUT_ERROR);
    }
    assert(it->second.first == 'i');
    return it->second.second;
}

TableConstraint parse_table_constraint(
        const std::string& line,
        const std::unordered_map<std::string, FSGecodeSpace::var_id>& varidx,
        const ProblemInfo& info)
{
    // e.g.: "clear negative=False ?x ?y ?z ..."
    std::vector<std::string> components;
    boost::split(components, line, boost::is_any_of(" "));
    assert(components.size() >= 3);
    unsigned symbolid = info.getSymbolId(components[0]);
    bool negative = (components[1] == "negative=True");

    std::vector<int> varidxs;
    for (unsigned i = 2; i < components.size(); ++i) {
        varidxs.emplace_back(retrieve_int_variable_index(components[i], varidx));
    }

    return TableConstraint(symbolid, varidxs, negative);
}

StateVariableConstraint parse_statevar_constraint(
        const std::string& line,
        const std::unordered_map<std::string, FSGecodeSpace::var_id>& varidx,
        const ProblemInfo& info)
{
    // e.g.: "28 True"
    std::vector<std::string> components;
    boost::split(components, line, boost::is_any_of(" "));
    assert(components.size() == 2);
    auto variable = boost::lexical_cast<unsigned>(components[0]);
    // So far we're only dealing with propositional problems; we'll extend this when there's need
    assert(components[1] == "True" || components[1] == "False");
    object_id value = (components[1] == "True") ? object_id::TRUE : object_id::FALSE;
    return StateVariableConstraint(variable, value);
}


RelationalConstraint parse_relational_constraint(
        const std::string& line,
        const std::unordered_map<std::string, FSGecodeSpace::var_id>& varidx,
        const ProblemInfo& info)
{
    // e.g.: "?x = ?y" or "?x != ?y"
    std::vector<std::string> components;
    boost::split(components, line, boost::is_any_of(" "));
    assert(components.size() == 3);
    auto lhs = retrieve_int_variable_index(components[0], varidx);
    auto rhs = retrieve_int_variable_index(components[2], varidx);
    auto reltype = gecode_symbol(components[1]);
    return RelationalConstraint(lhs, reltype, rhs);
}



void check_magic(std::istream& in, const std::string& magic) {
    std::string word;
    in >> word;
    if (word != magic) {
        std::cerr << "Failed to match magic word '" << magic << "'. Got '" << word << "' instead." << std::endl;
        exit_with(ExitCode::SEARCH_INPUT_ERROR);
    }
    in >> std::ws;
}

ActionSchemaCSP ActionSchemaCSP::load(std::ifstream& in, const ProblemInfo& info, std::vector<unsigned>& symbols_in_extensions) {
    ActionSchemaCSP csp;
    std::string line, line2;
    std::vector<std::string> components;
    unsigned nvariables, nparameters, nconstraints;

    std::unordered_map<std::string, FSGecodeSpace::var_id> varidx;

    // We will first use VarArgs, as we don't know how many variables of each type (int/bool) we have
    Gecode::IntVarArgs intvars;
    Gecode::BoolVarArgs boolvars;

    check_magic(in, "variables");
    in >> nvariables >> std::ws;
    for (unsigned i = 0; i < nvariables; ++i) {
        std::getline(in, line);
        boost::split(components, line, boost::is_any_of(" "));
        assert(components.size()==5);

        if (components[1] == "const") {
            assert(components[2] == "int");
            // TODO: With constants, we might want to do something different here
            intvars <<  Gecode::IntVar(*csp.space,
                    boost::lexical_cast<int>(components[3]), boost::lexical_cast<int>(components[4]));
            varidx[components[0]] = {'i', intvars.size()-1};
        }
        else {
            if (components[2] == "int") {
                intvars <<  Gecode::IntVar(*csp.space,
                                           boost::lexical_cast<int>(components[3]), boost::lexical_cast<int>(components[4]));
                varidx[components[0]] = {'i', intvars.size()-1};

            } else {
                assert(components[2] == "bool");
                boolvars << Gecode::BoolVar(*csp.space, 0, 1);
                varidx[components[0]] = {'b', boolvars.size()-1};
            }
        }
    }
    check_magic(in, "end-variables");

    // Once all variables have been processed, we can create a Gecode space and put them there.
    csp.space->intvars = Gecode::IntVarArray(*csp.space, intvars);
    csp.space->boolvars = Gecode::BoolVarArray(*csp.space, boolvars);


    // Let's now index the action parameters
    check_magic(in, "parameter-index");
    in >> nparameters >> std::ws;
    for (unsigned i = 0; i < nparameters; ++i) {
        std::getline(in, line);
        boost::split(components, line, boost::is_any_of(" "));
        assert(components.size()==1);
        auto it = varidx.find(components[0]);
        if (it == varidx.end()) {
            std::cerr << "ActionSchemaCSP: Failed to find parameter with name '" << components[0]
                      << "' among " << varidx.size() << " registered variable names" << std::endl;
            exit_with(ExitCode::SEARCH_INPUT_ERROR);
        }
        FSGecodeSpace::var_id idx = it->second;
        assert(idx.first == 'i'); // CSP variables that model action parameters need to be integers
        csp.parameter_variables.push_back(idx.second);
    }
    check_magic(in, "end-parameter-index");


    check_magic(in, "constraints");
    in >> nconstraints >> std::ws;
    for (unsigned i = 0; i < nconstraints; ++i) {
        std::getline(in, line);
        std::getline(in, line2);
        if (line == "table-constraint") {
            csp.table_constraints.emplace_back(parse_table_constraint(line2, varidx, info));
            symbols_in_extensions.at(csp.table_constraints.back().symbol_idx()) = 1;
        }
        else if (line == "statevar=const") {
            csp.statevar_constraints.emplace_back(parse_statevar_constraint(line2, varidx, info));
        }
        else if (line == "relational") {
            csp.relational_constraints.emplace_back(parse_relational_constraint(line2, varidx, info));
        }
        else {
            std::cerr << "ActionSchemaCSP: Unknown constraint type '" << line << "'" << std::endl;
            exit_with(ExitCode::SEARCH_INPUT_ERROR);
        }
    }
    check_magic(in, "end-constraints");
    return std::move(csp);
}

ActionSchemaCSP::ActionSchemaCSP() :
    space(new FSGecodeSpace())
{}

bool ActionSchemaCSP::initialize() {
    // Beware that the order in which the branching strategies are posted matters.
    // TODO We might want to explore more sophisticated strategies.
    Gecode::branch(*space, space->intvars, Gecode::INT_VAR_SIZE_MIN(), Gecode::INT_VAL_MIN());
    Gecode::branch(*space, space->boolvars, Gecode::BOOL_VAR_NONE(), Gecode::BOOL_VAL_MIN());

    // We post here those constraints that do not depend on any particular state. Currently, this includes only
    // relational constraints
    for (const auto& c:relational_constraints) {
        if (!c.post(*space)) return false;
    }
    return space->propagate();
}


FSGecodeSpace* ActionSchemaCSP::instantiate(const State& state, const StateBasedExtensionHandler& handler) const {
    // Note that for state-variable constraints we don't even need to have cloned the CSP
    for (const auto& c:statevar_constraints) {
        if (!c.post(state)) return nullptr;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    auto* clone = static_cast<FSGecodeSpace*>(space->clone());

    for (const auto& c:table_constraints) {
        if (!c.post(*clone, handler.retrieve(c.symbol_idx()))) {
            delete clone;
            return nullptr;
        }
    }

    return clone;
}


std::vector<object_id> ActionSchemaCSP::build_binding_from_solution(const FSGecodeSpace* solution) const {
    std::vector<object_id> values;
    values.reserve(parameter_variables.size());
    for (int csp_var_idx:parameter_variables) {
        values.push_back(make_object(type_id::object_t, solution->intvars[csp_var_idx].val()));
    }
    return values;
}


} // namespaces
