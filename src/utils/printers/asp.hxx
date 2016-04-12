
#pragma once

#include <fs_types.hxx>
#include <ostream>

namespace Gringo { struct Value; }

namespace fs0 { namespace print {

//! Prints an ASP model
class asp_model {
	protected:
		const std::vector<Gringo::Value>& _model;
	public:
		asp_model(const std::vector<Gringo::Value>& model) : _model(model) {}
		
		friend std::ostream& operator<<(std::ostream &os, const asp_model& o) { return o.print(os); }
		std::ostream& print(std::ostream& os) const;
};

} } // namespaces
