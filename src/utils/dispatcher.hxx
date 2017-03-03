
#pragma once

#include <utility>
#include <unordered_map>
#include <typeindex>
#include <boost/functional/hash.hpp>

////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2001 by Andrei Alexandrescu
// This code accompanies the book:
// Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design 
//     Patterns Applied". Copyright (c) 2001. Addison-Wesley.
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The author or Addison-Wesley Longman make no representations about the 
//     suitability of this software for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

namespace fs0 { namespace utils {

////////////////////////////////////////////////////////////////////////////////
// class template BasicDispatcher
// Implements a logarithmic double dispatcher for functors (or functions)
// Doesn't offer automated casts or symmetry
////////////////////////////////////////////////////////////////////////////////

template
<
	class BaseLhs,
	class BaseRhs = BaseLhs,
	typename ResultType = void,
	typename CallbackType = ResultType (*)(BaseLhs&, BaseRhs&)
>
class BasicDispatcher
{
	typedef std::pair<std::type_index, std::type_index> KeyType;
	
	struct KeyHasher { std::size_t operator()(const KeyType& key) const {
		std::size_t seed = 0;
		boost::hash_combine(seed, key.first.hash_code());
		boost::hash_combine(seed, key.second.hash_code());
		return seed;
	} };
	
	typedef CallbackType MappedType;
	typedef std::unordered_map<KeyType, MappedType, KeyHasher> MapType;
	MapType callbackMap_;
	
	void DoAdd(std::type_index lhs, std::type_index rhs, CallbackType fun);
	bool DoRemove(std::type_index lhs, std::type_index rhs);
	
public:
	template <class SomeLhs, class SomeRhs>
	void Add(CallbackType fun)
	{
		DoAdd(std::type_index(typeid(SomeLhs)), std::type_index(typeid(SomeRhs)), fun);
	}
	
	template <class SomeLhs, class SomeRhs>
	bool Remove()
	{
		return DoRemove(std::type_index(typeid(SomeLhs)), std::type_index(typeid(SomeRhs)));
	}
	
	ResultType Go(BaseLhs& lhs, BaseRhs& rhs);
};

// Non-inline to reduce compile time overhead...
template <class BaseLhs, class BaseRhs, 
	typename ResultType, typename CallbackType>
void BasicDispatcher<BaseLhs,BaseRhs,ResultType,CallbackType>
		::DoAdd(std::type_index lhs, std::type_index rhs, CallbackType fun)
{
	callbackMap_[KeyType(lhs, rhs)] = fun;
}
	
template <class BaseLhs, class BaseRhs, 
	typename ResultType, typename CallbackType>
bool BasicDispatcher<BaseLhs,BaseRhs,ResultType,CallbackType>
		::DoRemove(std::type_index lhs, std::type_index rhs)
{
	return callbackMap_.erase(KeyType(lhs, rhs)) == 1;
}

template <class BaseLhs, class BaseRhs, 
	typename ResultType, typename CallbackType>
ResultType BasicDispatcher<BaseLhs,BaseRhs,ResultType,CallbackType>
			::Go(BaseLhs& lhs, BaseRhs& rhs)
{
	typename MapType::key_type k(std::type_index(typeid(lhs)),std::type_index(typeid(rhs)));
	typename MapType::iterator i = callbackMap_.find(k);
	if (i == callbackMap_.end())
	{
			throw std::runtime_error("Function not found");
	}
	return (i->second)(lhs, rhs);
}





////////////////////////////////////////////////////////////////////////////////
// class template StaticCaster
// Implementation of the CastingPolicy used by FunctorDispatcher
////////////////////////////////////////////////////////////////////////////////

template <class To, class From>
struct StaticCaster
{
	static To& Cast(From& obj)
	{
		return static_cast<To&>(obj);
	}
};

////////////////////////////////////////////////////////////////////////////////
// class template DynamicCaster
// Implementation of the CastingPolicy used by FunctorDispatcher
////////////////////////////////////////////////////////////////////////////////

template <class To, class From>
struct DynamicCaster
{
	static To& Cast(From& obj)
	{
		return dynamic_cast<To&>(obj);
	}
};

////////////////////////////////////////////////////////////////////////////////
// class template Private::FnDispatcherHelper
// Implements trampolines and argument swapping used by FnDispatcher
////////////////////////////////////////////////////////////////////////////////

namespace Private
{
	template <class BaseLhs, class BaseRhs,
	class SomeLhs, class SomeRhs,
		typename ResultType,
		class CastLhs, class CastRhs,
		ResultType (*Callback)(SomeLhs&, SomeRhs&)>
	struct FnDispatcherHelper
	{
		static ResultType Trampoline(BaseLhs& lhs, BaseRhs& rhs)
		{
			return Callback(CastLhs::Cast(lhs), CastRhs::Cast(rhs));
		}
		static ResultType TrampolineR(BaseRhs& rhs, BaseLhs& lhs)
		{
			return Trampoline(lhs, rhs);
		}
	};
}

////////////////////////////////////////////////////////////////////////////////
// class template FnDispatcher
// Implements an automatic logarithmic double dispatcher for functions
// Features automated conversions
////////////////////////////////////////////////////////////////////////////////

template <class BaseLhs, class BaseRhs = BaseLhs,
			typename ResultType = void,
			template <class, class> class CastingPolicy = DynamicCaster,
			template <class, class, class, class>
			class DispatcherBackend = BasicDispatcher>
class FnDispatcher
{
	DispatcherBackend<BaseLhs, BaseRhs, ResultType, 
		ResultType (*)(BaseLhs&, BaseRhs&)> backEnd_;
	
public:
	template <class SomeLhs, class SomeRhs>
	void Add(ResultType (*pFun)(BaseLhs&, BaseRhs&))
	{
		return backEnd_.template Add<SomeLhs, SomeRhs>(pFun);
	}        
	
	template <class SomeLhs, class SomeRhs,
		ResultType (*callback)(SomeLhs&, SomeRhs&)>
	void Add()
	{
	typedef Private::FnDispatcherHelper<
				BaseLhs, BaseRhs, 
				SomeLhs, SomeRhs,
				ResultType,
				CastingPolicy<SomeLhs,BaseLhs>, 
				CastingPolicy<SomeRhs,BaseRhs>, 
				callback> Local;

		Add<SomeLhs, SomeRhs>(&Local::Trampoline);
	}
	
	template <class SomeLhs, class SomeRhs,
		ResultType (*callback)(SomeLhs&, SomeRhs&),
		bool symmetric>
	void Add(bool = true) // [gcc] dummy bool
	{
	typedef Private::FnDispatcherHelper<
				BaseLhs, BaseRhs, 
				SomeLhs, SomeRhs,
				ResultType,
				CastingPolicy<SomeLhs,BaseLhs>, 
				CastingPolicy<SomeRhs,BaseRhs>, 
				callback> Local;

		Add<SomeLhs, SomeRhs>(&Local::Trampoline);
		if (symmetric)
		{
			Add<SomeRhs, SomeLhs>(&Local::TrampolineR);
		}
	}
	
	template <class SomeLhs, class SomeRhs>
	void Remove()
	{
		backEnd_.template Remove<SomeLhs, SomeRhs>();
	}

	ResultType Go(BaseLhs& lhs, BaseRhs& rhs)
	{
		return backEnd_.Go(lhs, rhs);
	}
};

    
} } // namespaces
