
#pragma once

#include <boost/python.hpp>

#include <fs/core/fstrips/language.hxx>
#include <fs/core/fstrips/problem.hxx>
#include <fs/core/base.hxx>
#include <fs/core/lambda/search/search_model.hxx>
#include <fs/core/lambda/search/factory.hxx>


namespace bp = boost::python;
namespace fs = fs0::fstrips;
namespace ls = lambda::search;

namespace fs0 { namespace fstrips {
    class Problem;
    class Interpretation;
}}

//! These are a number of helpers to create raw-pointer objects from the FS library, the main role of which is to
//! clone the pointers that are injected into the native constructors of the objects being created. This is because
//! otherwise the Python interpreter will assume it has ownership of these pointers.
//! A better way of handling this without unnecessary object clones (which are expensive, as they will clone the
//! full AST representing a formula, effect, action, etc.) would be to use smart pointers, as outlined in the Boost
//! Python documentation and elsewhere:
//! * [How can I wrap a function which needs to take ownership of a raw pointer?]
//!     (https://www.boost.org/doc/libs/1_70_0/libs/python/doc/html/faq/how_can_i_wrap_a_function_which0.html)
//! * [Boost.Python: Ownership of pointer variables]
//!     (https://stackoverflow.com/questions/4112561/boost-python-ownership-of-pointer-variables)
//!
//! We leave this for future work, as it would involve quite some refactoring of the main formula representation classes
//! in FS.

fs::AtomicFormula* create_atomic_formula(unsigned symbol_id, bp::list& subterms);

fs::CompositeTerm* create_composite_term(unsigned symbol_id, bp::list& subterms);

fs::CompositeFormula* create_composite_formula(fs::Connective connective, bp::list& subformulas);

fs::QuantifiedFormula* create_quantified_formula(fs::Quantifier quantifier, bp::list& variables, const fs::Formula* subformula);

fs::AtomicEffect* create_atomic_effect(const fs::AtomicFormula* atom, fs::AtomicEffect::Type type, const fs::Formula* condition);

fs::FunctionalEffect* create_functional_effect(const fs::CompositeTerm* lhs, const fs::Term* rhs, const fs::Formula* condition);

fs::ActionSchema* create_action_schema(unsigned id, const std::string& name, bp::list& signature,
                                       bp::list& parameter_names, const fs::Formula* precondition, bp::list& effects);

std::shared_ptr<fs::Problem> create_problem(const std::string& name, const std::string& domain_name,
                                            bp::list& schemas, const std::shared_ptr<fs::Interpretation> init, const fs::Formula* goal);


std::shared_ptr<ls::SearchModel> create_model(std::shared_ptr<fs::Problem> problem, bool use_match_tree);


std::shared_ptr<ls::SearchAlgorithm> create_breadth_first_search_engine(std::shared_ptr<ls::SearchModel> model);
