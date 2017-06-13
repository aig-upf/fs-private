
#pragma once

#include <fs_types.hxx>


namespace fs0 { namespace asp { class LPHandler; }}

namespace fs0 {

//! An interface for _external_ singleton objects, to be subclassed if needed by particular 
//! domain-specific External classes.
class ExternalI  {
public:
	
	virtual ~ExternalI() = default;

	//! By default, we register no component
	virtual void registerComponents() const {throw std::runtime_error("Must be subclassed");}

	//! To be subclassed in case the particular problem has a ASP handler
	virtual asp::LPHandler* get_asp_handler() const { return nullptr; }
	
    //! To be subclassed
	virtual std::vector<ObjectIdx> get_offending_configurations(ObjectIdx confb, ObjectIdx arm_traj, ObjectIdx held_o, ObjectIdx gtype_holding, ObjectIdx gtype_obj) const {
		throw std::runtime_error("Must be subclassed");
	}
	
};


} // namespaces
