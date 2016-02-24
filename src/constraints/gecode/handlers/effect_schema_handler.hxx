
#pragma once

#include <constraints/gecode/handlers/action_schema_handler.hxx>

namespace fs0 { namespace gecode {

//! A CSP modeling and solving the effect of an action effect on a certain RPG layer
class EffectSchemaCSPHandler : public ActionSchemaCSPHandler {
public:
	typedef EffectSchemaCSPHandler* ptr;
	
	//! Factory method
	static std::vector<std::shared_ptr<BaseActionCSPHandler>> create(const std::vector<const ActionSchema*>& schemata, bool approximate, bool novelty);
	//! HACK
	static std::vector<std::shared_ptr<EffectSchemaCSPHandler>> create_derived(const std::vector<const ActionSchema*>& schemata, bool approximate, bool novelty);

	EffectSchemaCSPHandler(const ActionSchema& action, unsigned effect_idx, bool approximate, bool novelty);
	 
	~EffectSchemaCSPHandler() {}
	
protected:
	void log() const;
};


} } // namespaces
