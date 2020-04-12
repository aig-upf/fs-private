
#pragma once

#include <fs/core/fs_types.hxx>
#include <fs/core/utils/serializer.hxx>

namespace fs0 {

class ProblemInfo;

class StaticExtension {
public:
	virtual ~StaticExtension() = default;
	virtual Function get_function() const = 0;

	//! Factory method
	static std::unique_ptr<StaticExtension> load_static_extension(const std::string& name, const ProblemInfo& info);

	virtual std::ostream& print( std::ostream& os, const ProblemInfo& info ) const = 0;
};

std::ostream& operator<<( std::ostream& os, const StaticExtension& ex );

class ZeroaryFunction : public StaticExtension {
protected:
	object_id _data;

public:
	explicit ZeroaryFunction(const object_id& data) : _data(data) {}

	Function get_function() const override {
		auto& data = _data;
		return [&data](const ValueTuple& parameters){
			assert(parameters.empty());
			return data;
		};
	}

	std::ostream& print( std::ostream& os, const ProblemInfo& info ) const override;
};

class UnaryFunction : public StaticExtension {
protected:
	Serializer::UnaryMap _data;

public:
	explicit UnaryFunction(Serializer::UnaryMap&& data) : _data(std::move(data)) {}

	object_id value(const object_id& x) const { return _data.at(x); }

	Function get_function() const override {
		auto& data = _data;
		return [&data](const ValueTuple& parameters){
			assert(parameters.size() == 1);
			return data.at(parameters[0]);
		};
	}

	std::ostream& print( std::ostream& os, const ProblemInfo& info ) const override;
};

class UnaryPredicate : public StaticExtension {
protected:
	Serializer::UnarySet _data;

public:
	explicit UnaryPredicate(Serializer::UnarySet&& data) : _data(std::move(data)) {}

	Function get_function() const override {
		auto& data = _data;
		return [&data](const ValueTuple& parameters){
			assert(parameters.size() == 1);
			return make_object(data.find(parameters[0]) != data.end());
		};
	}

	std::ostream& print( std::ostream& os, const ProblemInfo& info ) const override;
};


class BinaryFunction : public StaticExtension {
protected:
	Serializer::BinaryMap _data;

public:
	explicit BinaryFunction(Serializer::BinaryMap&& data) : _data(std::move(data)) {}

	Function get_function() const override {
		auto& data = _data;
		return [&data](const ValueTuple& parameters){
			assert(parameters.size() == 2);
			return data.at({parameters[0], parameters[1]});
		};
	}

	std::ostream& print( std::ostream& os, const ProblemInfo& info ) const override;
};

class BinaryPredicate : public StaticExtension {
protected:
	Serializer::BinarySet _data;

public:
	explicit BinaryPredicate(Serializer::BinarySet&& data) : _data(std::move(data)) {}

	Function get_function() const override {
		auto& data = _data;
		return [&data](const ValueTuple& parameters){
			assert(parameters.size() == 2);
			return make_object(data.find({parameters[0], parameters[1]}) != data.end());
		};
	}

	std::ostream& print( std::ostream& os, const ProblemInfo& info ) const override;
};

class Arity3Function : public StaticExtension {
protected:
	Serializer::Arity3Map _data;

public:
	explicit Arity3Function(Serializer::Arity3Map&& data) : _data(std::move(data)) {}

	Function get_function() const override {
		auto& data = _data;
		return [&data](const ValueTuple& parameters){
			assert(parameters.size() == 3);
			return data.at(std::make_tuple(parameters[0], parameters[1], parameters[2]));
		};
	}

	std::ostream& print( std::ostream& os, const ProblemInfo& info ) const override;
};

class Arity3Predicate : public StaticExtension {
protected:
	Serializer::Arity3Set _data;

public:
	explicit Arity3Predicate(Serializer::Arity3Set&& data) : _data(std::move(data)) {}

	Function get_function() const override {
		auto& data = _data;
		return [&data](const ValueTuple& parameters){
			assert(parameters.size() == 3);
			return make_object(data.find(std::make_tuple(parameters[0], parameters[1], parameters[2])) != data.end());
		};
	}

	std::ostream& print( std::ostream& os, const ProblemInfo& info ) const override;
};

class ArbitraryArityFunction : public StaticExtension {
protected:
	Serializer::function_t _data;

public:
	explicit ArbitraryArityFunction(Serializer::function_t&& data) : _data(std::move(data)) {}

	Function get_function() const override {
		auto& data = _data;
		return [&data](const ValueTuple& parameters) { return data.at(parameters); };
	}

	std::ostream& print( std::ostream& os, const ProblemInfo& info ) const override;
};

class ArbitraryArityPredicate : public StaticExtension {
protected:
	Serializer::predicate_t _data;

public:
	explicit ArbitraryArityPredicate(Serializer::predicate_t&& data) : _data(std::move(data)) {}

	Function get_function() const override {
		auto& data = _data;
		return [&data](const ValueTuple& parameters){
			return make_object(data.find(parameters) != data.end());
		};
	}

	std::ostream& print( std::ostream& os, const ProblemInfo& info ) const override;
};


} // namespaces
