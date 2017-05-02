
#pragma once

#include <constraints/direct/component.hxx>
#include <atom.hxx>


namespace fs0 {

class State;

//! A generic action effect.
class DirectEffect : public DirectComponent {
protected:
	//! The state variable affected by the effect.
	const VariableIdx _affected;

public:
	DirectEffect(const VariableIdxVector& scope, VariableIdx affected, const std::vector<ObjectIdx>& parameters);
	virtual ~DirectEffect() = default;

	inline VariableIdx getAffected() const { return _affected; }

	//! The generic appllicable / apply methods, to be overriden by concrete subclasses.
	//! In the case of applicable, it is only of use for conditional effects.
	virtual bool applicable(const ObjectIdxVector& values) const { return true; };
	virtual Atom apply(const ObjectIdxVector& values) const = 0;

	//! The 6 methods below are specializations for 0-ary / unary / binary effects that exist only for performance reasons. A bit hackish, but works.
	virtual bool applicable() const { throw std::runtime_error("This method can only be used by 0-ary effects"); };
	virtual Atom apply() const { throw std::runtime_error("This method can only be used by 0-ary effects"); }

	virtual bool applicable(ObjectIdx value) const { throw std::runtime_error("This method can only be used by unary effects"); };
	virtual Atom apply(ObjectIdx value) const { throw std::runtime_error("This method can only be used by unary effects"); }

	virtual bool applicable(ObjectIdx v1, ObjectIdx v2) const { throw std::runtime_error("This method can only be used by binary effects"); };
	virtual Atom apply(ObjectIdx v1, ObjectIdx v2) const { throw std::runtime_error("This method can only be used by binary effects"); }

	//! A small helper
	virtual Atom apply(const State& s) const;

	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const DirectEffect& o) { return o.print(os); }
	virtual std::ostream& print(std::ostream& os) const {
		os << "<unnamed effect>";
		return os;
	}
};

//! We specialize this class for performance reasons, since it is so common.
class ZeroaryDirectEffect : public DirectEffect
{
public:
	ZeroaryDirectEffect(VariableIdx affected, const std::vector<ObjectIdx>& parameters);
	virtual ~ZeroaryDirectEffect() {}

	//! Effects are by default applicable, i.e. condition-less.
	virtual bool applicable() const override { return true; }

	//! 0-ary effects should use the specialized version
	Atom apply(const ObjectIdxVector& values) const override { throw std::runtime_error("0-ary effects are expected not to use this method"); };

	//! To be overriden by the concrete effect.
	virtual Atom apply() const override = 0;

	//! A small helper
	inline Atom apply(const State& s) const override { return this->apply(); }
};

//! We specialize this class for performance reasons, since it is so common.
class UnaryDirectEffect : public DirectEffect
{
public:
	UnaryDirectEffect(VariableIdx relevant, VariableIdx affected, const std::vector<ObjectIdx>& parameters);
	virtual ~UnaryDirectEffect() {}

	//! Effects are by default applicable, i.e. condition-less.
	bool applicable(ObjectIdx value) const override { return true; };

	//! Unary effects should use the specialized version
	Atom apply(const ObjectIdxVector& values) const override { throw std::runtime_error("Unary effects are expected not to use this method"); };

	//! To be overriden by the concrete effect.
	virtual Atom apply(ObjectIdx value) const override = 0;

	//! A small helper
	Atom apply(const State& s) const override;
};

//! We specialize this class for performance reasons, since it is so common.
class BinaryDirectEffect : public DirectEffect
{
public:
	BinaryDirectEffect(const VariableIdxVector& scope, VariableIdx affected, const std::vector<ObjectIdx>& parameters);
	virtual ~BinaryDirectEffect() {}

	//! Effects are by default applicable, i.e. condition-less.
	virtual bool applicable(ObjectIdx v1, ObjectIdx v2) const override { return true; };

	//! This might be necessary in some cases.
	Atom apply(const ObjectIdxVector& values) const override {
		assert(values.size() == 2);
		return apply(values[0], values[1]);
	};

	//! To be overriden by the concrete effect.
	virtual Atom apply(ObjectIdx v1, ObjectIdx v2) const override = 0;

	//! A small helper
	Atom apply(const State& s) const override;
};

} // namespaces
