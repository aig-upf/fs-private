
#ifndef __APTK_CORE_JUSTIFIED_ACTION_HXX__
#define __APTK_CORE_JUSTIFIED_ACTION_HXX__

#include <cassert>
#include <iosfwd>
#include <actions.hxx>
#include <core_types.hxx>
#include <variable_cache.hxx>


namespace aptk { namespace core {

/**
 * A JustifiedApplicableEntity is a (grounded) applicable entity paired by the justification of its applicability,
 * along with some other useful information.
 */
class JustifiedApplicableEntity
{
public:
	typedef std::shared_ptr<JustifiedApplicableEntity> ptr;
	typedef std::shared_ptr<const JustifiedApplicableEntity> cptr;
	
	JustifiedApplicableEntity(const ApplicableEntity& entity) :
		_entity(entity) {}
		
	inline const ApplicableEntity& getEntity() const { return _entity; }
	
	inline const FactSet& getCauses() const { return _causes; }
	
	inline const VariableCache& getBinding() const { return _binding; }
	inline VariableCache& getBinding() { return _binding; }
	
	inline void addCause(const Fact& fact) { _causes.insert(fact); }
	
	void clear() {
		_causes.clear();
		_binding.clear();
	}
	
protected:

	//! The grounded entity
	const ApplicableEntity& _entity;
	
	//! The causes of applicability
	FactSet _causes; // or: PlainConjunctiveFact _causes;
	
	//! The values for the relevant variables that made the entity applicable.
	VariableCache _binding;
};

/**
 * A JustifiedAction is a (grounded) action paired by the justification of its applicability,
 * along with some other useful information.
 */
class JustifiedAction: public JustifiedApplicableEntity
{
public:
	typedef std::shared_ptr<JustifiedAction> ptr;
	typedef std::shared_ptr<const JustifiedAction> cptr;
	
	JustifiedAction(const CoreAction& action) : 
		JustifiedApplicableEntity(action) {}
		
	inline const CoreAction& getAction() const { return static_cast<const CoreAction&>(_entity); }
};

} } // namespaces

#endif
