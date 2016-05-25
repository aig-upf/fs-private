
#pragma once

#include <constraints/direct/constraint.hxx>
#include <constraints/direct/effect.hxx>


namespace fs0 {

class ProblemInfo;

// X < Y
class LTConstraint : public BinaryDirectConstraint {
public:
	LTConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) : BinaryDirectConstraint(scope, parameters)
	{ assert(parameters.empty()); }

	bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const override { return o1 < o2; }
	
	FilteringOutput filter(unsigned variable) const override;
	
	DirectConstraint* compile(const ProblemInfo& problemInfo) const override { return nullptr; }
	
	std::ostream& print(std::ostream& os) const override;
};

// X <= Y
class LEQConstraint : public BinaryDirectConstraint {
public:
	LEQConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) : BinaryDirectConstraint(scope, parameters)
	{ assert(parameters.empty()); }

	bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const override { return o1 <= o2; }
	
	FilteringOutput filter(unsigned variable) const override;
	
	DirectConstraint* compile(const ProblemInfo& problemInfo) const override { return nullptr; }
	
	std::ostream& print(std::ostream& os) const override;
};


// X = Y
class EQConstraint : public BinaryDirectConstraint {
public:
	EQConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) : BinaryDirectConstraint(scope, parameters)
	{ assert(parameters.empty()); }

	bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const override { return o1 == o2; }
	
	FilteringOutput filter(unsigned variable) const override;
	
	DirectConstraint* compile(const ProblemInfo& problemInfo) const override { return nullptr; }
	
	std::ostream& print(std::ostream& os) const override;
};

// X != Y
class NEQConstraint : public BinaryDirectConstraint {
public:
	NEQConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) : BinaryDirectConstraint(scope, parameters)
	{ assert(parameters.empty()); }

	bool isSatisfied(ObjectIdx o1, ObjectIdx o2) const override { return o1 != o2; }
	
	FilteringOutput filter(unsigned variable) const override;
	
	DirectConstraint* compile(const ProblemInfo& problemInfo) const override { return nullptr; }
	
	std::ostream& print(std::ostream& os) const override;
};

// X = c
class EQXConstraint : public UnaryDirectConstraint {
public:
	EQXConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) : UnaryDirectConstraint(scope, parameters)
	{ assert(parameters.size() == 1); }

	bool isSatisfied(ObjectIdx o) const override { return o == _parameters[0]; }
	
	FilteringOutput filter(const DomainMap& domains) const override;
	
	// No compilation
	DirectConstraint* compile(const ProblemInfo& problemInfo) const override { return nullptr; }
	
	std::ostream& print(std::ostream& os) const override;
};

// X != c
class NEQXConstraint : public UnaryDirectConstraint {
public:
	NEQXConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) : UnaryDirectConstraint(scope, parameters)
	{ assert(parameters.size() == 1); }

	bool isSatisfied(ObjectIdx o) const override { return o != _parameters[0]; }
	
	FilteringOutput filter(const DomainMap& domains) const override;
	
	// No compilation
	DirectConstraint* compile(const ProblemInfo& problemInfo) const override { return nullptr; }
	
	std::ostream& print(std::ostream& os) const override;
};

// X < c
class LTXConstraint : public UnaryDirectConstraint {
public:
	LTXConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) : UnaryDirectConstraint(scope, parameters)
	{ assert(parameters.size() == 1); }

	bool isSatisfied(ObjectIdx o) const override { return o < _parameters[0]; }
	
	FilteringOutput filter(const DomainMap& domains) const override;
	
	// No compilation
	DirectConstraint* compile(const ProblemInfo& problemInfo) const override { return nullptr; }
	
	std::ostream& print(std::ostream& os) const override;
};


// X <= c
class LEQXConstraint : public UnaryDirectConstraint {
public:
	LEQXConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) : UnaryDirectConstraint(scope, parameters)
	{ assert(parameters.size() == 1); }

	bool isSatisfied(ObjectIdx o) const override { return o <= _parameters[0]; }
	
	FilteringOutput filter(const DomainMap& domains) const override;
	
	// No compilation
	DirectConstraint* compile(const ProblemInfo& problemInfo) const override { return nullptr; }
	
	std::ostream& print(std::ostream& os) const override;
};

// X > c
class GTXConstraint : public UnaryDirectConstraint {
public:
	GTXConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) : UnaryDirectConstraint(scope, parameters)
	{ assert(parameters.size() == 1); }

	bool isSatisfied(ObjectIdx o) const override { return o > _parameters[0]; }
	
	FilteringOutput filter(const DomainMap& domains) const override;
	
	// No compilation
	DirectConstraint* compile(const ProblemInfo& problemInfo) const override { return nullptr; }
	
	std::ostream& print(std::ostream& os) const override;
};

// X > c
class GEQXConstraint : public UnaryDirectConstraint {
public:
	GEQXConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters) : UnaryDirectConstraint(scope, parameters)
	{ assert(parameters.size() == 1); }

	bool isSatisfied(ObjectIdx o) const override { return o >= _parameters[0]; }
	
	FilteringOutput filter(const DomainMap& domains) const override;
	
	// No compilation
	DirectConstraint* compile(const ProblemInfo& problemInfo) const override { return nullptr; }
	
	std::ostream& print(std::ostream& os) const override;
};

// X := c, where X os the affected variable, and c is the only parameter
class ValueAssignmentEffect : public ZeroaryDirectEffect {
public:
	ValueAssignmentEffect(VariableIdx affected, ObjectIdx value);
	
	Atom apply() const override;
	
	std::ostream& print(std::ostream& os) const override;
};

// X := Y
class VariableAssignmentEffect : public UnaryDirectEffect {
public:
	VariableAssignmentEffect(VariableIdx relevant, VariableIdx affected);
	
	Atom apply(ObjectIdx v1) const override;
	
	std::ostream& print(std::ostream& os) const override;
};

// X = Y + c (with X and Y possibly being the same variable)
class AdditiveUnaryEffect : public UnaryDirectEffect {
public:
	AdditiveUnaryEffect(VariableIdx relevant, VariableIdx affected, int value);

	Atom apply(ObjectIdx v1) const override;

	std::ostream& print(std::ostream& os) const override;
};

// X = Y + Z (with X and Y possibly being the same variable)
class AdditiveBinaryEffect : public BinaryDirectEffect {
public:
	AdditiveBinaryEffect(const VariableIdxVector& scope, VariableIdx affected);

	Atom apply(ObjectIdx v1, ObjectIdx v2) const override;

	std::ostream& print(std::ostream& os) const override;
};

// X = Y - c (with X and Y possibly being the same variable)
class SubtractiveUnaryEffect : public UnaryDirectEffect {
public:
	SubtractiveUnaryEffect(VariableIdx relevant, VariableIdx affected, int value);

	Atom apply(ObjectIdx v1) const override;

	std::ostream& print(std::ostream& os) const override;
};

// X = c - Y (with X and Y possibly being the same variable)
class InvSubtractiveUnaryEffect : public UnaryDirectEffect {
public:
	InvSubtractiveUnaryEffect(VariableIdx relevant, VariableIdx affected, int value);

	Atom apply(ObjectIdx v1) const override;

	std::ostream& print(std::ostream& os) const override;
};

// X = Y - Z (with X and Y possibly being the same variable)
class SubtractiveBinaryEffect : public BinaryDirectEffect {
public:
	SubtractiveBinaryEffect(const VariableIdxVector& scope, VariableIdx affected);

	Atom apply(ObjectIdx v1, ObjectIdx v2) const override;

	std::ostream& print(std::ostream& os) const override;
};

// X = Y * c (with X and Y possibly being the same variable)
class MultiplicativeUnaryEffect : public UnaryDirectEffect {
public:
	MultiplicativeUnaryEffect(VariableIdx relevant, VariableIdx affected, int value);

	Atom apply(ObjectIdx v1) const override;

	std::ostream& print(std::ostream& os) const override;
};

// X = Y * Z (with X and Y possibly being the same variable)
class MultiplicativeBinaryEffect : public BinaryDirectEffect {
public:
	MultiplicativeBinaryEffect(const VariableIdxVector& scope, VariableIdx affected);

	Atom apply(ObjectIdx v1, ObjectIdx v2) const override;

	std::ostream& print(std::ostream& os) const override;
};

} // namespaces

