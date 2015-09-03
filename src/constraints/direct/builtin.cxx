
#include <constraints/direct/builtin.hxx>
#include <problem.hxx>

namespace fs0 {

LTConstraint::LTConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) :
	BinaryDirectConstraint(scope, parameters)
{
	assert(parameters.empty());
}
	
FilteringOutput LTConstraint::filter(unsigned variable) {
	assert(projection.size() == 2);
	assert(variable == 0 || variable == 1);
	
	Domain& x_dom = *(projection[0]);
	Domain& y_dom = *(projection[1]);
	ObjectIdx x_min = *(x_dom.cbegin()), x_max = *(x_dom.crbegin());
	ObjectIdx y_min = *(y_dom.cbegin()), y_max = *(y_dom.crbegin());
	
	if (variable == 0) { // We filter x_dom, the domain of X
		if (x_max < y_max) return FilteringOutput::Unpruned;
		if (x_min >= y_max) return FilteringOutput::Failure;
		
		// Otherwise there must be at least a value, but not all, in the new domain.
		auto it = x_dom.lower_bound(y_max); // it points to the first x in x_dom such that x >= y_max
		assert(it != x_dom.begin() && it != x_dom.end());
		x_dom = Domain(x_dom.begin(), it); // Update the domain by using the assignment operator.
		return FilteringOutput::Pruned;
	} else { // We filter y_dom, the domain of Y
		if (x_min < y_min) return FilteringOutput::Unpruned;
		if (x_min >= y_max) return FilteringOutput::Failure;
		
		// Otherwise the domain has necessarily to be pruned, but is not inconsistent
		auto it = y_dom.lower_bound(x_min);
		assert(it != y_dom.end());
		if (*it == x_min) ++it;
		y_dom = Domain(it, y_dom.end());  // Update the domain by using the assignment operator.
		return FilteringOutput::Pruned;
	}
}

LEQConstraint::LEQConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) :
	BinaryDirectConstraint(scope, parameters)
{
	assert(parameters.empty());
}
	
FilteringOutput LEQConstraint::filter(unsigned variable) {
	assert(projection.size() == 2);
	assert(variable == 0 || variable == 1);
	
	Domain& x_dom = *(projection[0]);
	assert( x_dom.size() > 0 );
	Domain& y_dom = *(projection[1]);
	assert( y_dom.size() > 0 );
	ObjectIdx x_min = *(x_dom.cbegin()), x_max = *(x_dom.crbegin());
	ObjectIdx y_min = *(y_dom.cbegin()), y_max = *(y_dom.crbegin());
	
	if (variable == 0) { // We filter x_dom, the domain of X
		if (x_max <= y_max) return FilteringOutput::Unpruned;
		if (x_min > y_max) return FilteringOutput::Failure;
		
		// Otherwise there must be at least a value, but not all, in the new domain.
		auto it = x_dom.lower_bound(y_max);
		assert( it != x_dom.end() );
		if (*it == y_max) ++it;
		x_dom = Domain(x_dom.begin(), it); // Update the domain by using the assignment operator.
		return FilteringOutput::Pruned;
	} else { // We filter y_dom, the domain of Y
		if (x_min <= y_min) return FilteringOutput::Unpruned;
		if (x_min > y_max) return FilteringOutput::Failure;
		
		// Otherwise the domain has necessarily to be pruned, but is not inconsistent
		auto it = y_dom.lower_bound(x_min);
		assert(it != y_dom.begin() && it != y_dom.end());
		
		y_dom = Domain(it, y_dom.end());  // Update the domain by using the assignment operator.
		return FilteringOutput::Pruned;
	}
}

EQConstraint::EQConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) :
	BinaryDirectConstraint(scope, parameters)
{
	assert(parameters.empty());
}
	
FilteringOutput EQConstraint::filter(unsigned variable) {
	assert(projection.size() == 2);
	assert(variable == 0 || variable == 1);
	unsigned other = (variable == 0) ? 1 : 0;
	Domain& domain = *(projection[variable]);
	Domain& other_domain = *(projection[other]);
	Domain new_domain;
	
	for (ObjectIdx x:domain) {
		// We simply check that x is in the domain of the other variable
		auto it = other_domain.lower_bound(x);
		if (it != other_domain.end() && *it == x) { // `it` points to the first x in x_dom such that x >= y_max
			new_domain.insert(new_domain.cend(), x); //  x is an arc-consistent value. We will insert on the end of the container, as it is already sorted.
		}
	}
	if (new_domain.size() == domain.size()) return FilteringOutput::Unpruned;
	if (new_domain.size() == 0) return FilteringOutput::Failure;

	// Otherwise the domain has necessarily been pruned
	domain = new_domain; // Update the domain by using the assignment operator.
	return FilteringOutput::Pruned;
}


NEQConstraint::NEQConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) :
	BinaryDirectConstraint(scope, parameters)
{
	assert(parameters.empty());
}
	
FilteringOutput NEQConstraint::filter(unsigned variable) {
	assert(projection.size() == 2);
	assert(variable == 0 || variable == 1);
	unsigned other = (variable == 0) ? 1 : 0;
	Domain& domain = *(projection[variable]);
	Domain& other_domain = *(projection[other]);
	
	if (other_domain.size() >= 2) return FilteringOutput::Unpruned; // If the other domain has at least two domains, we won't be able to prune anything
	
	assert(other_domain.size() == 1);
	ObjectIdx other_val = *(other_domain.cbegin());
	
	// If we can erase the only value, i.e. it was in the domain, we do it, otherwise the result is an unpruned domain.
	if (domain.erase(other_val) == 0) return FilteringOutput::Unpruned;
	return domain.size() > 0 ? FilteringOutput::Pruned : FilteringOutput::Failure;
}

EQXConstraint::EQXConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) :
	UnaryDirectConstraint(scope, parameters)
{
	assert(parameters.size() == 1);
}
	
// Filtering for a X = c constraint simply consists on pruning from the variable
// domain all values different than c.
FilteringOutput EQXConstraint::filter(const DomainMap& domains) const {
	assert(_scope.size() == 1);
	Domain& domain = *(domains.at(_scope[0]));
	auto it = domain.find(_parameters[0]);
	if (it == domain.end()) {
		domain.clear(); // Just in case
		return FilteringOutput::Failure;
	}
	
	if (domain.size() == 1) return FilteringOutput::Unpruned; // 'c' is the only value in the set
	
	Domain new_domain;
	new_domain.insert(new_domain.cend(), _parameters[0]);
	domain = new_domain; // Update the domain by using the assignment operator.	
	
	return FilteringOutput::Pruned;
}

NEQXConstraint::NEQXConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) :
	UnaryDirectConstraint(scope, parameters)
{
	assert(parameters.size() == 1);
}
	
// Filtering for a X <> c constraint simply consists on pruning from the variable
// domain value 'c', if available.
FilteringOutput NEQXConstraint::filter(const DomainMap& domains) const {
	assert(_scope.size() == 1);
	Domain& domain = *(domains.at(_scope[0]));
	unsigned erased = domain.erase(_parameters[0]);
	if (erased == 0) return FilteringOutput::Unpruned;
	else return (domain.size() == 0) ? FilteringOutput::Failure : FilteringOutput::Pruned;
}


ValueAssignmentEffect::ValueAssignmentEffect(VariableIdx affected, ObjectIdx value)
	: ZeroaryDirectEffect(affected, {value})
{}

Atom ValueAssignmentEffect::apply() const {
	assert(applicable());
	return Atom(_affected, _parameters[0]);
}

std::ostream& ValueAssignmentEffect::print(std::ostream& os) const {
	const ProblemInfo& info = Problem::getInfo();
	os << info.getVariableName(_affected) << " := " << info.getObjectName(_affected, _parameters[0]);
	return os;
}

VariableAssignmentEffect::VariableAssignmentEffect(VariableIdx relevant, VariableIdx affected)
	: UnaryDirectEffect(relevant, affected, {})
{}


Atom VariableAssignmentEffect::apply(ObjectIdx v1) const {
	assert(applicable(v1));
	return Atom(_affected, v1);
}

std::ostream& VariableAssignmentEffect::print(std::ostream& os) const {
	const ProblemInfo& info = Problem::getInfo();
	os << info.getVariableName(_affected) << " := " << info.getVariableName(_scope[0]);
	return os;
}


AdditiveUnaryEffect::AdditiveUnaryEffect(VariableIdx relevant, VariableIdx affected, int value)
	: UnaryDirectEffect(relevant, affected, {value})
{}


Atom AdditiveUnaryEffect::apply(ObjectIdx v1) const {
	assert(applicable(v1));
	return Atom(_affected, v1 + _parameters[0]);
}

std::ostream& AdditiveUnaryEffect::print(std::ostream& os) const {
	const ProblemInfo& info = Problem::getInfo();
	os << info.getVariableName(_affected) << " := " << info.getVariableName(_scope[0])  << " + " << _parameters[0];
	return os;
}

AdditiveBinaryEffect::AdditiveBinaryEffect(const VariableIdxVector& scope, VariableIdx affected)
	: BinaryDirectEffect(scope, affected, {})
{}

Atom AdditiveBinaryEffect::apply(ObjectIdx v1, ObjectIdx v2) const {
	assert(applicable(v1, v2));
	return Atom(_affected, v1 + v2);
}

std::ostream& AdditiveBinaryEffect::print(std::ostream& os) const {
	const ProblemInfo& info = Problem::getInfo();
	os << info.getVariableName(_affected) << " := " << info.getVariableName(_scope[0])  << " + " << info.getVariableName(_scope[1]);
	return os;
}


SubtractiveUnaryEffect::SubtractiveUnaryEffect(VariableIdx relevant, VariableIdx affected, int value)
	: UnaryDirectEffect(relevant, affected, {value})
{}


Atom SubtractiveUnaryEffect::apply(ObjectIdx v1) const {
	assert(applicable(v1));
	return Atom(_affected, v1 - _parameters[0]);
}

std::ostream& SubtractiveUnaryEffect::print(std::ostream& os) const {
	const ProblemInfo& info = Problem::getInfo();
	os << info.getVariableName(_affected) << " := " << info.getVariableName(_scope[0])  << " - " << _parameters[0];
	return os;
}

InvSubtractiveUnaryEffect::InvSubtractiveUnaryEffect(VariableIdx relevant, VariableIdx affected, int value)
	: UnaryDirectEffect(relevant, affected, {value})
{}


Atom InvSubtractiveUnaryEffect::apply(ObjectIdx v1) const {
	assert(applicable(v1));
	return Atom(_affected, _parameters[0] - v1);
}

std::ostream& InvSubtractiveUnaryEffect::print(std::ostream& os) const {
	const ProblemInfo& info = Problem::getInfo();
	os << info.getVariableName(_affected) << " := " << _parameters[0] << " - " << info.getVariableName(_scope[0]);
	return os;
}


SubtractiveBinaryEffect::SubtractiveBinaryEffect(const VariableIdxVector& scope, VariableIdx affected)
	: BinaryDirectEffect(scope, affected, {})
{}

Atom SubtractiveBinaryEffect::apply(ObjectIdx v1, ObjectIdx v2) const {
	assert(applicable(v1, v2));
	return Atom(_affected, v1 - v2);
}

std::ostream& SubtractiveBinaryEffect::print(std::ostream& os) const {
	const ProblemInfo& info = Problem::getInfo();
	os << info.getVariableName(_affected) << " := " << info.getVariableName(_scope[0])  << " - " << info.getVariableName(_scope[1]);
	return os;
}

MultiplicativeUnaryEffect::MultiplicativeUnaryEffect(VariableIdx relevant, VariableIdx affected, int value)
	: UnaryDirectEffect(relevant, affected, {value})
{}


Atom MultiplicativeUnaryEffect::apply(ObjectIdx v1) const {
	assert(applicable(v1));
	return Atom(_affected, v1 * _parameters[0]);
}

std::ostream& MultiplicativeUnaryEffect::print(std::ostream& os) const {
	const ProblemInfo& info = Problem::getInfo();
	os << info.getVariableName(_affected) << " := " << info.getVariableName(_scope[0])  << " * " << _parameters[0];
	return os;
}

MultiplicativeBinaryEffect::MultiplicativeBinaryEffect(const VariableIdxVector& scope, VariableIdx affected)
	: BinaryDirectEffect(scope, affected, {})
{}

Atom MultiplicativeBinaryEffect::apply(ObjectIdx v1, ObjectIdx v2) const {
	assert(applicable(v1, v2));
	return Atom(_affected, v1 * v2);
}

std::ostream& MultiplicativeBinaryEffect::print(std::ostream& os) const {
	const ProblemInfo& info = Problem::getInfo();
	os << info.getVariableName(_affected) << " := " << info.getVariableName(_scope[0])  << " * " << info.getVariableName(_scope[1]);
	return os;
}

} // namespaces
