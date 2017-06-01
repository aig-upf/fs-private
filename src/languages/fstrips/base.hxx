
#pragma once

#include <iostream>

#include <utils/visitor.hxx>

namespace fs0 { class ProblemInfo; } // TODO[LAMBDA] - REMOVE THIS DEPENDENCY

namespace fs0 { namespace language { namespace fstrips {

	
//! A common base class for both terms and formulas
class LogicalElement :
 	public Loki::BaseVisitable<void, Loki::ThrowCatchAll, true>
// 	public Loki::BaseVisitable<>
//	public fs0::utils::BaseVisitable<void>
{
public:
	virtual ~LogicalElement() = default;
	
	//! Clone idiom
	virtual LogicalElement* clone() const = 0;
	
	//! Prints a representation of the object to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const LogicalElement& o);
	virtual std::ostream& print(std::ostream& os, const fs0::ProblemInfo& info) const = 0;
};

} } } // namespaces

