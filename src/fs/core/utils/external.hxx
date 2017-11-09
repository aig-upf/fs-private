
#pragma once

namespace fs0 { namespace asp { class LPHandler; }}

namespace fs0 {

//! An interface for _external_ singleton objects, to be subclassed if needed by particular 
//! domain-specific External classes.
class ExternalI  {
public:
	
	virtual ~ExternalI() = default;

	//! By default, we register no component
	virtual void registerComponents() const {}

	//! To be subclassed in case the particular problem has a ASP handler
	virtual asp::LPHandler* get_asp_handler() const { return nullptr; }
};


} // namespaces
