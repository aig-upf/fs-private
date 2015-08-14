
#include <constraints/gecode/translators/component_translator.hxx>
#include <constraints/gecode/simple_csp.hxx>
#include <constraints/gecode/csp_translator.hxx>
#include <constraints/gecode/helper.hxx>
#include <constraints/gecode/handlers/csp_handler.hxx>
#include <problem.hxx>
#include <languages/fstrips/builtin.hxx>
#include <utils/logging.hxx>

#include <gecode/int.hh>

namespace fs0 { namespace gecode {

	typedef fs::RelationalFormula::Symbol AFSymbol;
	const std::map<AFSymbol, Gecode::IntRelType> RelationalFormulaTranslator::symbol_to_gecode = {
		{AFSymbol::EQ, Gecode::IRT_EQ}, {AFSymbol::NEQ, Gecode::IRT_NQ}, {AFSymbol::LT, Gecode::IRT_LE},
		{AFSymbol::LEQ, Gecode::IRT_LQ}, {AFSymbol::GT, Gecode::IRT_GR}, {AFSymbol::GEQ, Gecode::IRT_GQ}
	};

	Gecode::IntRelType RelationalFormulaTranslator::gecode_symbol(fs::RelationalFormula::cptr formula) { return symbol_to_gecode.at(formula->symbol()); }


	const std::map<Gecode::IntRelType, Gecode::IntRelType> RelationalFormulaTranslator::operator_inversions = {
		{Gecode::IRT_EQ, Gecode::IRT_EQ}, {Gecode::IRT_NQ, Gecode::IRT_NQ}, {Gecode::IRT_LE, Gecode::IRT_GQ},
		{Gecode::IRT_LQ, Gecode::IRT_GR}, {Gecode::IRT_GR, Gecode::IRT_LQ}, {Gecode::IRT_GQ, Gecode::IRT_LE}
	};

	Gecode::IntRelType RelationalFormulaTranslator::invert_operator(Gecode::IntRelType op) {
		return operator_inversions.at(op);
	}


void ConstantTermTranslator::registerVariables(const fs::Term::cptr term, CSPVariableType type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& variables) const {
	auto constant = dynamic_cast<fs::Constant::cptr>(term);
	assert(constant);
	translator.registerConstant(constant, csp, variables);
}

void StateVariableTermTranslator::registerVariables(const fs::Term::cptr term, CSPVariableType type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& variables) const {
	auto variable = dynamic_cast<fs::StateVariable::cptr>(term);
	assert(variable);
	translator.registerStateVariable(variable, type, csp, variables);
}

void NestedTermTranslator::registerVariables(const fs::Term::cptr term, CSPVariableType type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& variables) const {
	auto nested = dynamic_cast<fs::NestedTerm::cptr>(term);
	assert(nested);
	// If the subterm occurs somewhere else in the action / formula, it might have already been parsed and registered,
	// in which case we do NOT want to register it again
	if (translator.isRegistered(nested, type)) return;
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	#ifdef DEBUG
	std::ostream& log = getDebugLogStream( "translation" );
	log << "Registering nested term ";
	nested->print(log, info );
	log << std::endl;
	#endif
	const auto& signature = info.getFunctionData(nested->getSymbolId()).getSignature();
	// MRJ: Why are we getting this vector? Did I eliminate code I shouldn't?
	const std::vector<fs::Term::cptr>& subterms = nested->getSubterms();

	// We first parse and register the subterms recursively. The type of subterm variables is always input
	GecodeCSPHandler::registerTermVariables(nested->getSubterms(), CSPVariableType::Input, csp, translator, variables);

	// MRJ: TODO: Discuss with Guillem how to introduce this into the linguistic type hierarchy
	fs::FluentHeadedNestedTerm::cptr dyn_nested_fluent = dynamic_cast< fs::FluentHeadedNestedTerm::cptr >( term);
	if ( dyn_nested_fluent != nullptr ) {

		unsigned count = 0;
		for (ObjectIdx object:info.getTypeObjects(signature[0])) {

			VariableIdx variable = info.resolveStateVariable(nested->getSymbolId(), {object});

			FDEBUG( "translation", "Registering implicitly referred variable " << info.getVariableName( variable ) << std::endl);
			if ( _implicit_ref_vars.find( std::make_pair(variable,type) ) == _implicit_ref_vars.end() ) {
				_implicit_ref_vars.insert( std::make_pair( std::make_pair(variable,type), new StateVariable( variable  ) ) );
			}
			translator.registerStateVariable( _implicit_ref_vars[std::make_pair(variable,type)], type, csp, variables );
			count++;
		}
		// MRJ: this registers the pointer variable in the csp
		FDEBUG( "translation", "Registering pointer variable")
		translator.registerNestedTermIndirection( dyn_nested_fluent, type, count-1, csp, variables  );
	}
	// And now register the CSP variable corresponding to the current term
	do_root_registration(nested, type, csp, translator, variables);
}

void NestedTermTranslator::do_root_registration(const fs::NestedTerm::cptr nested, CSPVariableType type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& variables) const {
	translator.registerNestedTerm(nested, type, csp, variables);
}


void ArithmeticTermTranslator::do_root_registration(const fs::NestedTerm::cptr nested, CSPVariableType type, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& variables) const {
	auto bounds = nested->getBounds();
	translator.registerNestedTerm(nested, type, bounds.first, bounds.second, csp, variables);
}

void ArithmeticTermTranslator::registerConstraints(const fs::Term::cptr term, CSPVariableType type, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) const {
	auto addition = dynamic_cast<fs::AdditionTerm::cptr>(term);
	assert(addition);

	// First we register recursively the constraints of the subterms
	GecodeCSPHandler::registerTermConstraints(addition->getSubterms(), CSPVariableType::Input, csp, translator);

	// Now we assert that the root temporary variable equals the sum of the subterms
	const Gecode::IntVar& result = translator.resolveVariable(addition, CSPVariableType::Input, csp);
	Gecode::IntVarArgs operands = translator.resolveVariables(addition->getSubterms(), CSPVariableType::Input, csp);
	post(csp, operands, result);
}


void AdditionTermTranslator::post(SimpleCSP& csp, const Gecode::IntVarArgs& operands, const Gecode::IntVar& result) const {
	Gecode::linear(csp, getLinearCoefficients(), operands, getRelationType(), result);
}

void SubtractionTermTranslator::post(SimpleCSP& csp, const Gecode::IntVarArgs& operands, const Gecode::IntVar& result) const {
	Gecode::linear(csp, getLinearCoefficients(), operands, getRelationType(), result);
}

void MultiplicationTermTranslator::post(SimpleCSP& csp, const Gecode::IntVarArgs& operands, const Gecode::IntVar& result) const {
	Gecode::mult(csp, operands[0], operands[1], result);
}

Gecode::IntArgs AdditionTermTranslator::getLinearCoefficients() const {
	std::vector<int> coefficients{1, 1};
	return Gecode::IntArgs(coefficients);
}

Gecode::IntArgs SubtractionTermTranslator::getLinearCoefficients() const {
	std::vector<int> coefficients{1, -1};
	return Gecode::IntArgs(coefficients);
}

Gecode::IntArgs MultiplicationTermTranslator::getLinearCoefficients() const {
	std::vector<int> coefficients{1, -1};
	return Gecode::IntArgs(coefficients);
}


void StaticNestedTermTranslator::registerConstraints(const fs::Term::cptr term, CSPVariableType type, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) const {
	auto stat = dynamic_cast<fs::StaticHeadedNestedTerm::cptr>(term);
	assert(stat);

	#ifdef DEBUG
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();
	std::ostream& info_log = getDebugLogStream( "translation" );
	info_log << "Registering constraints for static nested term ";
	stat->print(info_log, info );
	info_log << std::endl;
	#endif


	// First we register recursively the constraints of the subterms
	GecodeCSPHandler::registerTermConstraints(stat->getSubterms(), CSPVariableType::Input, csp, translator);

	// Assume we have a static term s(t_1, ..., t_n), where t_i are the subterms.
	// We have registered a termporary variable Z for the whole term, plus temporaries Z_i accounting for each subterm t_i
	// Now we need to post an extensional constraint on all temporary variables <Z_1, Z_2, ..., Z_n, Z> such that
	// the tuples <z_1, ..., z_n, z> satisfying the constraints are exactly those such that z = s(z_1, ..., z_n)

	FDEBUG( "translation", "Creating extensional constraint");
	// First compile the variables in the right order (order matters, must be the same than in the tupleset):
	Gecode::IntVarArgs variables = translator.resolveVariables(stat->getSubterms(), CSPVariableType::Input, csp);
	variables << translator.resolveVariable(stat, CSPVariableType::Input, csp);

	#ifdef DEBUG
	{
		std::ostream& log = getDebugLogStream("translation");
		log << "Scope:" << std::endl;
		unsigned i = 0;
		for ( auto var : variables )
			log << "x_" << i++ << " \\in " << var << std::endl;
	}
	#endif

	// Now compile the tupleset
	Gecode::TupleSet extension = Helper::extensionalize(stat);
	#ifdef DEBUG
	{
		std::ostream& log = getDebugLogStream("translation");
		unsigned arity = 0;
		for ( auto var : variables )
			arity++;
		log << "Tuples:" << std::endl;
		for ( int i = 0; i < extension.tuples(); i++  ) {
			log << "<";
			for ( unsigned j = 0; j < arity; j++ ) {
				log << extension[i][j];
				if ( j < arity-1) log << ", ";
			}

			log << ">" << std::endl;
		}
	}
	#endif


	// And finally post the constraint
	Gecode::extensional(csp, variables, extension);
}

void FluentNestedTermTranslator::registerConstraints(const fs::Term::cptr term, CSPVariableType type, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) const {
	const ProblemInfo& info = Problem::getCurrentProblem()->getProblemInfo();

	//assert(false); // TODO - REGISTER the full element constraint, with reindexing through an extensional constraint

	auto fluent = dynamic_cast<fs::FluentHeadedNestedTerm::cptr>(term);
	assert(fluent);
	const auto& signature = info.getFunctionData(fluent->getSymbolId()).getSignature();
	const std::vector<fs::Term::cptr>& subterms = fluent->getSubterms();

	#ifdef DEBUG
	std::ostream& info_log = getDebugLogStream( "translation" );
	info_log << "Registering constraints for fluent nested term ";
	fluent->print(info_log, info );
	info_log << std::endl;
	#endif

	// First we register recursively the constraints of the subterms - subterms' constraint will always have input type
	GecodeCSPHandler::registerTermConstraints(subterms, CSPVariableType::Input, csp, translator);

	assert(subterms.size() == signature.size());
	if (subterms.size() > 1) throw std::runtime_error("Nested subterms of arity > 1 not yet implemented");

	assert(signature.size() == 1); // Cannot be 0, or we'd have instead a StateVariable term
	unsigned idx = 0; // element constraints are 0-indexed

	Gecode::IntVarArgs array_variables; // The actual array of variables that will form the element constraint

	// The correspondence between the index variable possible values and their 0-indexed position in the element constraint table
	Gecode::TupleSet correspondence;
	for (ObjectIdx object:info.getTypeObjects(signature[0])) {

		VariableIdx variable = info.resolveStateVariable(fluent->getSymbolId(), {object});

		FDEBUG( "translation", "Noting correspondence for variable " << info.getVariableName( variable ) << std::endl);
		try {
			auto v = translator.resolveOutputStateVariable(csp, variable); // TODO - Output or Input???
			FDEBUG( "translation", "planning variable " << variable << " domain is " << v );
			array_variables << v;
		} catch( const UnregisteredStateVariableError& ) {
			// MRJ: These need to be always "input" variables, can we have nested fluents on the lhs of an effect?
			auto v = translator.resolveInputStateVariable(csp, variable);
			FDEBUG( "translation", "planning variable " << variable << " domain is " << v );
			array_variables << v;
		}

		correspondence.add(IntArgs(2, object, idx));
		++idx;
	}

	correspondence.finalize();



	// Post the extensional constraint relating the value of the index variables
	const Gecode::IntVar& index_variable = translator.resolveVariable(subterms[0], CSPVariableType::Input, csp);
	//Gecode::IntVar indexed_index; // TODO - XXX - WHERE DO WE GET THIS FROM??
	// MRJ: check NestedTerm variable registration
	Gecode::IntVar indexed_index = translator.resolveNestedTermIndirection( fluent, type, csp );
	Gecode::extensional(csp, IntVarArgs() << index_variable << indexed_index, correspondence);
	#ifdef DEBUG
	{
		std::ostream& log = getDebugLogStream("translation");
		log << "Extensional constraint:" << std::endl;
		log << "Scope: index_var \\in " << index_variable << " indexed_index \\in " << indexed_index << std::endl;
		log << "Tuples: " << std::endl;
		for ( int i = 0; i < correspondence.tuples(); i++ )
			log << "<" << correspondence[i][0] << ", " << correspondence[i][1] << ">" <<std::endl;
	}
	#endif

	// Now post the actual element constraint
	try {
		const Gecode::IntVar& element_result = translator.resolveVariable(fluent, CSPVariableType::Input, csp); // TODO - Output or Input???
		Gecode::element(csp, array_variables, indexed_index, element_result);
		#ifdef DEBUG
		{
			std::ostream& log = getDebugLogStream( "translation" );
			log << "Element constraint: element_result = array_variables[indexed_index]" << std::endl;
			log << "element_result \\in " << element_result << std::endl;
			log << "indexed_index \\in " << indexed_index << std::endl;
			log << "array_variables = {" << std::endl;
			for ( auto var : array_variables )
				log << var << std::endl;
			log << "}" << std::endl;
		}
		#endif
	} catch ( const  UnregisteredStateVariableError& e ) {
		const Gecode::IntVar& element_result = translator.resolveVariable(fluent, CSPVariableType::Output, csp); // TODO - Output or Input???
		Gecode::element(csp, array_variables, indexed_index, element_result);
		#ifdef DEBUG
		{
			std::ostream& log = getDebugLogStream("translation");
			log << "Element constraint: element_result = array_variables[indexed_index]" << std::endl;
			log << "element_result \\in " << element_result << std::endl;
			log << "indexed_index \\in " << indexed_index << std::endl;
			log << "array_variables = {" << std::endl;
			for ( auto var : array_variables )
				log << var << std::endl;
			log << "}" << std::endl;
		}
		#endif
	}
}

NestedTermTranslator::~NestedTermTranslator() {
	for ( auto o : _implicit_ref_vars )
		delete o.second;
}




void AtomicFormulaTranslator::registerVariables(const fs::AtomicFormula::cptr formula, SimpleCSP& csp, GecodeCSPVariableTranslator& translator, Gecode::IntVarArgs& variables) const {
	// We simply need to recursively register the variables of each subterm
	for (const Term::cptr subterm:formula->getSubterms()) {
		GecodeCSPHandler::registerTermVariables(subterm, CSPVariableType::Input, csp, translator, variables); // Formula variables are always input variables
	}
}

void AtomicFormulaTranslator::registerConstraints(const fs::AtomicFormula::cptr formula, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) const {
	// We simply need to recursively register the constraints of each subterm
	for (const Term::cptr subterm:formula->getSubterms()) {
		GecodeCSPHandler::registerTermConstraints(subterm, CSPVariableType::Input, csp, translator);
	}
}

void RelationalFormulaTranslator::registerConstraints(const fs::AtomicFormula::cptr formula, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) const {
	auto condition = dynamic_cast<fs::RelationalFormula::cptr>(formula);
	assert(condition);

	// Register possible nested constraints recursively by calling the parent registrar
	AtomicFormulaTranslator::registerConstraints(formula, csp, translator);

	// And register the relation constraint itself
	const Gecode::IntVar& lhs_gec_var = translator.resolveVariable(condition->lhs(), CSPVariableType::Input, csp);
	const Gecode::IntVar& rhs_gec_var = translator.resolveVariable(condition->rhs(), CSPVariableType::Input, csp);
	Gecode::rel(csp, lhs_gec_var, gecode_symbol(condition), rhs_gec_var);
}

void AlldiffGecodeTranslator::registerConstraints(const fs::AtomicFormula::cptr formula, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) const {
	auto alldiff = dynamic_cast<fs::AlldiffFormula::cptr>(formula);
	assert(alldiff);

	// Register possible nested constraints recursively by calling the parent registrar
	AtomicFormulaTranslator::registerConstraints(formula, csp, translator);

	Gecode::IntVarArgs variables = translator.resolveVariables(alldiff->getSubterms(), CSPVariableType::Input, csp);
	Gecode::distinct(csp, variables, Gecode::ICL_DOM);
}

void SumGecodeTranslator::registerConstraints(const fs::AtomicFormula::cptr formula, SimpleCSP& csp, const GecodeCSPVariableTranslator& translator) const {
	auto sum = dynamic_cast<fs::SumFormula::cptr>(formula);
	assert(sum);

	// Register possible nested constraints recursively by calling the parent registrar
	AtomicFormulaTranslator::registerConstraints(formula, csp, translator);


	Gecode::IntVarArgs variables = translator.resolveVariables(sum->getSubterms(), CSPVariableType::Input, csp);

	// The sum constraint is a particular subcase of gecode's linear constraint with all variables' coefficients set to 1
	// except for the coefficient of the result variable, which is set to -1
	std::vector<int> v_coefficients(variables.size(), 1);
	v_coefficients[variables.size() - 1] = -1; // Last coefficient is a -1, since the last variable of the scope is the element of the sum
	Gecode::IntArgs coefficients(v_coefficients);

	Gecode::linear(csp, coefficients, variables, Gecode::IRT_EQ, 0, Gecode::ICL_DOM);
}


} } // namespaces
