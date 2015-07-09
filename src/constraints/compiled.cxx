
#include <constraints/compiled.hxx>
#include <utils/projections.hxx>
#include <utils/utils.hxx>

namespace fs0 {

CompiledUnaryConstraint::CompiledUnaryConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters, ExtensionT&& extension) : 
	UnaryParametrizedScopedConstraint(scope, parameters), _extension(extension)
{}

CompiledUnaryConstraint::CompiledUnaryConstraint(const UnaryParametrizedScopedConstraint& constraint, const ProblemInfo& problemInfo) :
	CompiledUnaryConstraint(constraint.getScope(), constraint.getParameters(), _compile(constraint, problemInfo))
{}


std::unordered_set<CompiledUnaryConstraint::ElementT> CompiledUnaryConstraint::compile(const UnaryParametrizedScopedConstraint& constraint, const ProblemInfo& problemInfo) {
	VariableIdx relevant = constraint.getScope()[0];
	const ObjectIdxVector& all_values = problemInfo.getVariableObjects(relevant);
	
	std::unordered_set<ElementT> ordered;
	for(ObjectIdx value:all_values) {
		if (constraint.isSatisfied(value)) {
			ordered.insert(value);
		}
	}
	return ordered;
}


CompiledUnaryConstraint::ExtensionT CompiledUnaryConstraint::_compile(const UnaryParametrizedScopedConstraint& constraint, const ProblemInfo& problemInfo) {
	// Depending on the type of representation, we can directly return the compiled data structure or not.
	#ifdef EXTENSIONAL_REPRESENTATION_USES_VECTORS
		auto ordered = compile(constraint, problemInfo);
		return ExtensionT(ordered.begin(), ordered.end());
		
	#else
		return compile(constraint, problemInfo);
	#endif
}

bool CompiledUnaryConstraint::isSatisfied(ObjectIdx o) const {
	#ifdef EXTENSIONAL_REPRESENTATION_USES_VECTORS
	return std::binary_search(_extension.begin(), _extension.end(), o); // TODO - Change for a O(1) lookup in a std::unordered_set ?
	#else
	return _extension.find(o) != _extension.end();
	#endif
}

#ifndef EXTENSIONAL_REPRESENTATION_USES_VECTORS
namespace detail{

template <typename ExtensionType>
class	IntersectionTest
{
public:
	IntersectionTest( const ExtensionType& ext, Domain& dst ) 
	: _ext( ext ), _dst( dst ) {}

	void operator()( ObjectIdx o )  {
		if ( _ext.find( o ) == _ext.end() ) return;
		_dst.insert(o);
	}

	const ExtensionType& 	_ext;
	Domain&			_dst;
};

}
#endif

ScopedConstraint::Output CompiledUnaryConstraint::filter(const DomainMap& domains) const {
	
	DomainVector projection = Projections::project(domains, _scope);
	assert(projection.size() == 1);
	Domain& domain = *(projection[0]);
	Domain new_domain;

	#ifdef EXTENSIONAL_REPRESENTATION_USES_VECTORS
	std::set_intersection(domain.begin(), domain.end(), _extension.begin(), _extension.end(), std::inserter(new_domain, new_domain.end()));
	#else
	std::for_each( domain.begin(), domain.end(), detail::IntersectionTest<ExtensionT>( _extension, new_domain ) ); 
	#endif
	
	if (new_domain.size() == domain.size()) return Output::Unpruned;
	if (new_domain.size() == 0) return Output::Failure;
	
	// Otherwise the domain has necessarily been pruned
	domain = new_domain;  // Update the domain with the new values using the assignment operator
	return Output::Pruned;
}

CompiledBinaryConstraint::CompiledBinaryConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters, ExtensionT&& extension1, ExtensionT&& extension2) :
	BinaryParametrizedScopedConstraint(scope, parameters), _extension1(extension1),  _extension2(extension2)
{}

CompiledBinaryConstraint::CompiledBinaryConstraint(const BinaryParametrizedScopedConstraint& constraint, const ProblemInfo& problemInfo) :
	CompiledBinaryConstraint(constraint.getScope(), constraint.getParameters(), _compile(constraint, 0, problemInfo), _compile(constraint, 1, problemInfo))
{}


bool CompiledBinaryConstraint::isSatisfied(ObjectIdx o1, ObjectIdx o2) const {
	auto iter = _extension1.find(o1);
	assert(iter != _extension1.end());
	#ifdef EXTENSIONAL_REPRESENTATION_USES_VECTORS
	const ObjectIdxVector& D_y = iter->second; // iter->second contains all the elements y of the domain of the second variable such that <x, y> satisfies the constraint
	return std::binary_search(D_y.begin(), D_y.end(), o2); // TODO - Change for a O(1) lookup in a std::unordered_set ?
	#else
	const ObjectIdxHash& D_y = iter->second;
	return D_y.find( o2 ) != D_y.end();
	#endif
}

std::map<ObjectIdx, std::set<ObjectIdx>> CompiledBinaryConstraint::compile(const fs0::BinaryParametrizedScopedConstraint& constraint, unsigned int variable, const fs0::ProblemInfo& problemInfo) {
	assert(variable == 0 || variable == 1);
	unsigned other = (variable == 0) ? 1 : 0;
	VariableIdxVector scope = constraint.getScope();
	
	std::map<ObjectIdx, std::set<ObjectIdx>> ordered;
	
	const ObjectIdxVector& Dx = problemInfo.getVariableObjects(scope[variable]);
	const ObjectIdxVector& Dy = problemInfo.getVariableObjects(scope[other]);
	
	for (ObjectIdx x:Dx) {
		auto res = ordered.insert(std::make_pair(x, std::set<ObjectIdx>())); // We insert the empty vector (all elements will at least have it) and keep the reference.
		assert(res.second); // The element couldn't exist
		std::set<ObjectIdx>& set = res.first->second;
		
		for (ObjectIdx y:Dy) {
			if ((variable == 0 && constraint.isSatisfied(x, y)) || (variable == 1 && constraint.isSatisfied(y, x))) {
				set.insert(y);
			}
		}
	}
	return ordered;
}

CompiledBinaryConstraint::ExtensionT CompiledBinaryConstraint::_compile(const BinaryParametrizedScopedConstraint& constraint, unsigned variable, const ProblemInfo& problemInfo) {
	
	auto ordered = compile(constraint, variable, problemInfo);
	
	// Now we transform the ordered set into a (implicitly ordered) vector
	ExtensionT extension;
	for(const auto& elem:ordered) {
		#ifdef EXTENSIONAL_REPRESENTATION_USES_VECTORS
		extension.insert(std::make_pair(elem.first, ObjectIdxVector(elem.second.begin(), elem.second.end()) ));
		#else
		extension.insert( std::make_pair( elem.first, ObjectIdxHash( elem.second.begin(), elem.second.end() ) ));
		#endif
	}
	return extension;
}


ScopedConstraint::Output CompiledBinaryConstraint::filter(unsigned variable) {
	assert(projection.size() == 2);
	assert(variable == 0 || variable == 1);
	unsigned other = (variable == 0) ? 1 : 0;
	const ExtensionT& extension_map = (variable == 0) ? _extension1 : _extension2;
	
	Domain& domain = *(projection[variable]);
	Domain& other_domain = *(projection[other]);
	Domain new_domain;
	
	for (ObjectIdx x:domain) {
		auto iter = extension_map.find(x);
		assert(iter != extension_map.end());
		#ifdef EXTENSIONAL_REPRESENTATION_USES_VECTORS
		const ObjectIdxVector& D_y = iter->second; // iter->second contains all the elements y of the domain of the second variable such that <x, y> satisfies the constraint
		#else
		const ObjectIdxHash& D_y = iter->second;
		#endif
		if (!Utils::empty_intersection(other_domain.begin(), other_domain.end(), D_y.begin(), D_y.end())) {
			new_domain.insert(new_domain.cend(), x); //  x is an arc-consistent value. We will insert on the end of the container, as it is already sorted.
		}
	}
	if (new_domain.size() == domain.size()) return Output::Unpruned;
	if (new_domain.size() == 0) return Output::Failure;

	// Otherwise the domain has necessarily been pruned
	domain = new_domain; // Update the domain by using the assignment operator.
	return Output::Pruned;
}


CompiledUnaryEffect::ExtensionT CompiledUnaryEffect::compile(const fs0::UnaryScopedEffect& effect, const fs0::ProblemInfo& problemInfo) {
	VariableIdx relevant = effect.getScope()[0];
	const ObjectIdxVector& all_values = problemInfo.getVariableObjects(relevant);
	
	ExtensionT map;
	for(ObjectIdx value:all_values) {
		try {
			auto atom = effect.apply(value);
			assert(atom.getVariable() == effect.getAffected());
			map.insert(std::make_pair(value, atom.getValue()));
		} catch(const std::out_of_range& e) {  // TODO - Refactor this, too hacky...
			// If the effect produces an exception, we simply consider it non-applicable and go on.
		}
	}
	return map;
}

} // namespaces
