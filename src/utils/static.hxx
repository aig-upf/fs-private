
#pragma once

#include <fs_types.hxx>
#include <utils/serializer.hxx>

namespace fs0 {

class ProblemInfo;

class StaticExtension {
public:
	virtual ~StaticExtension() = default;
	virtual Function get_function() const = 0;
	
	//! Factory method
	static std::unique_ptr<StaticExtension> load_static_extension(const std::string& name, const ProblemInfo& info);
};


class ZeroaryFunction : public StaticExtension {
protected:
	object_id _data;
	
public:
	ZeroaryFunction(object_id data) : _data(data) {}
	
	object_id value() const { return _data; }
	
	Function get_function() const override {
		auto& data = _data;
		return [&data](const ValueTuple& parameters){
			assert(parameters.size() == 0);
			return data;
		};
	}
};

class UnaryFunction : public StaticExtension {
protected:
	Serializer::BoostUnaryMap _data;
	
public:
	UnaryFunction(Serializer::BoostUnaryMap&& data) : _data(std::move(data)) {}
	
	object_id value(object_id x) const { return _data.at(x); }
	
	Function get_function() const override {
		auto& data = _data;
		return [&data](const ValueTuple& parameters){
			assert(parameters.size() == 1);
			return data.at(parameters[0]);
		};
	}
};

class UnaryPredicate : public StaticExtension {
protected:
	Serializer::BoostUnarySet _data;
	
public:
	UnaryPredicate(Serializer::BoostUnarySet&& data) : _data(std::move(data)) {}
	
	bool value(object_id x) const { return _data.find(x) != _data.end(); }
	
	Function get_function() const override {
		auto& data = _data;
		return [&data](const ValueTuple& parameters){
			assert(parameters.size() == 1);
			return make_obj(data.find(parameters[0]) != data.end());
		};
	}
};


class BinaryFunction : public StaticExtension {
protected:
	Serializer::BoostBinaryMap _data;
	
public:
	BinaryFunction(Serializer::BoostBinaryMap&& data) : _data(std::move(data)) {}
	
	object_id value(object_id x, object_id y) const { return _data.at({x, y}); }
	
	Function get_function() const override {
		auto& data = _data;
		return [&data](const ValueTuple& parameters){
			assert(parameters.size() == 2);
			return data.at({parameters[0], parameters[1]});
		};
	}
	
	const Serializer::BoostBinaryMap& get_data() const { return _data; }
};

class BinaryPredicate : public StaticExtension {
protected:
	Serializer::BoostBinarySet _data;
	
public:
	BinaryPredicate(Serializer::BoostBinarySet&& data) : _data(std::move(data)) {}
	
	bool value(object_id x, object_id y) const { return _data.find({x,y}) != _data.end(); }
	
	Function get_function() const override {
		auto& data = _data;
		return [&data](const ValueTuple& parameters){
			assert(parameters.size() == 2);
			return make_obj(data.find({parameters[0], parameters[1]}) != data.end());
		};
	}
};

class Arity3Function : public StaticExtension {
protected:
	Serializer::BoostArity3Map _data;
	
public:
	Arity3Function(Serializer::BoostArity3Map&& data) : _data(std::move(data)) {}
	
	object_id value(object_id x0, object_id x1, object_id x2) const { return _data.at(std::make_tuple(x0,x1,x2)); }
	
	Function get_function() const override {
		auto& data = _data;
		return [&data](const ValueTuple& parameters){
			assert(parameters.size() == 3);
			return data.at(std::make_tuple(parameters[0], parameters[1], parameters[2]));
		};
	}
};

class Arity3Predicate : public StaticExtension {
protected:
	Serializer::BoostArity3Set _data;
	
public:
	Arity3Predicate(Serializer::BoostArity3Set&& data) : _data(std::move(data)) {}
	
	bool value(object_id x0, object_id x1, object_id x2) const { return _data.find(std::make_tuple(x0,x1,x2)) != _data.end(); }
	
	Function get_function() const override {
		auto& data = _data;
		return [&data](const ValueTuple& parameters){
			assert(parameters.size() == 3);
			return make_obj(data.find(std::make_tuple(parameters[0], parameters[1], parameters[2])) != data.end());
		};
	}
};

class Arity4Function : public StaticExtension {
protected:
	Serializer::BoostArity4Map _data;
	
public:
	Arity4Function(Serializer::BoostArity4Map&& data) : _data(std::move(data)) {}
	
	object_id value(object_id x0, object_id x1, object_id x2, object_id x3) const { return _data.at(std::make_tuple(x0,x1,x2,x3)); }
	
	Function get_function() const override {
		auto& data = _data;
		return [&data](const ValueTuple& parameters){
			assert(parameters.size() == 4);
			return data.at(std::make_tuple(parameters[0], parameters[1], parameters[2], parameters[3]));
		};
	}
};

class Arity4Predicate : public StaticExtension {
protected:
	Serializer::BoostArity4Set _data;
	
public:
	Arity4Predicate(Serializer::BoostArity4Set&& data) : _data(std::move(data)) {}
	
	bool value(object_id x0, object_id x1, object_id x2, object_id x3) const { return _data.find(std::make_tuple(x0,x1,x2,x3)) != _data.end(); }
	
	Function get_function() const override {
		auto& data = _data;
		return [&data](const ValueTuple& parameters){
			assert(parameters.size() == 4);
			return make_obj(data.find(std::make_tuple(parameters[0], parameters[1], parameters[2], parameters[3])) != data.end());
		};
	}
};


} // namespaces