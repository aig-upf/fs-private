

#include <constraints/direct/csp_handler.hxx>


namespace fs0 {


DirectCSPHandler::DirectCSPHandler(const std::vector<DirectConstraint::cptr>& constraints)
	: _constraints(constraints), _relevant(indexRelevantVariables(constraints)) {
	initialize();
}

//! Precompute some of the structures that we'll need later on.
void DirectCSPHandler::initialize() {
	// Index the different constraints by arity
	indexConstraintsByArity();

	// Initialize the worklist
	initializeAC3Worklist(binary_constraints, AC3Worklist);
}

//! Indexes pointers to the constraints in three different vectors: unary, binary and n-ary constraints.
void DirectCSPHandler::indexConstraintsByArity() {
	for (const DirectConstraint::cptr ctr:_constraints) {
		FilteringType filtering = ctr->filteringType();
		if (filtering == FilteringType::Unary) {
			unary_constraints.push_back(ctr);
		} else if (filtering == FilteringType::ArcReduction) {
			binary_constraints.push_back(ctr);
		} else {
			n_ary_constraints.push_back(ctr);
		}
	}
}

//! Initializes a worklist. `constraints` is expected to have only binary constraints.
void DirectCSPHandler::initializeAC3Worklist(const std::vector<DirectConstraint::cptr>& constraints, ArcSet& worklist) {
	for (DirectConstraint::cptr ctr:constraints) {
		assert(ctr->getArity() == 2);
		worklist.insert(std::make_pair(ctr, 0));
		worklist.insert(std::make_pair(ctr, 1));
	}
}


FilteringOutput DirectCSPHandler::filter(const DomainMap& domains) const {
	if (_constraints.empty()) return FilteringOutput::Unpruned; // Safety check

	FilteringOutput result = unaryFiltering(domains);
	if (result == FilteringOutput::Failure ||
		unary_constraints.size() == _constraints.size()) { // If all constraints are unary, there's no need to go on.
		return result;
	}

	// We copy the constraint worklist, but only if it is not empty. Copying empty boost containers produces
	// a nasty memory leak in some boost versions, as per https://svn.boost.org/trac/boost/ticket/9166
	ArcSet worklist;
	if (!AC3Worklist.empty()) worklist = AC3Worklist;
	

	// Pre-load the non-unary constraints
	loadConstraintDomains(domains, binary_constraints);
	loadConstraintDomains(domains, n_ary_constraints);

	// First apply both types of filtering
	FilteringOutput b_result = binaryFiltering(worklist);
	if (b_result == FilteringOutput::Failure) {
		// Empty the non-unary constraints
		emptyConstraintDomains(binary_constraints);
		emptyConstraintDomains(n_ary_constraints);
		return b_result;
	}

	FilteringOutput g_result = globalFiltering();
	if (g_result == FilteringOutput::Failure) {
		// Empty the non-unary constraints
		emptyConstraintDomains(binary_constraints);
		emptyConstraintDomains(n_ary_constraints);
		return g_result;
	}

	// The global result won't be affected: if it was "Pruned", it'll continue to be prune regardless of what happens inside the loop.
	if (b_result == FilteringOutput::Pruned || g_result == FilteringOutput::Pruned) result = FilteringOutput::Pruned;

	// Keep pruning until we reach a fixpoint.
	while (b_result == FilteringOutput::Pruned && g_result == FilteringOutput::Pruned) {
		// Each type of pruning (global or binary) needs only be performed
		// if the other type of pruning actually modified some domain.
		b_result = binaryFiltering(worklist);
		if (b_result == FilteringOutput::Pruned) g_result = globalFiltering();
	}

	// Empty the non-unary constraints
	emptyConstraintDomains(binary_constraints);
	emptyConstraintDomains(n_ary_constraints);

	return result;
}


void DirectCSPHandler::emptyConstraintDomains(const std::vector<DirectConstraint::cptr>& constraints) const {
	for (DirectConstraint::cptr constraint:constraints) {
		constraint->emptyDomains();
	}
}

void DirectCSPHandler::loadConstraintDomains(const DomainMap& domains, const std::vector<DirectConstraint::cptr>& constraints) const {
	for (DirectConstraint::cptr constraint:constraints) {
		constraint->loadDomains(domains);
	}
}

FilteringOutput DirectCSPHandler::unaryFiltering(const DomainMap& domains) const {
	return unaryFiltering(unary_constraints, domains);
}

FilteringOutput DirectCSPHandler::unaryFiltering(const std::vector<DirectConstraint::cptr>& constraints, const DomainMap& domains) {
	FilteringOutput output = FilteringOutput::Unpruned;

	for (DirectConstraint::cptr ctr:constraints) {
		assert(ctr->getArity() == 1);
		FilteringOutput o = ctr->filter(domains);
		if (o == FilteringOutput::Pruned) {
			output = FilteringOutput::Pruned;
		} else if (o == FilteringOutput::Failure) {
			return o;  // Early termination
		}
	}
	return output;
}

//! AC3 filtering
FilteringOutput DirectCSPHandler::binaryFiltering(ArcSet& worklist) const {

	FilteringOutput result = FilteringOutput::Unpruned;

	// 1. Analyse pending arcs until the worklist is empty
	while (!worklist.empty()) {
		Arc a = select(worklist);
		DirectConstraint::cptr constraint = a.first;
		unsigned variable = a.second;  // The index 0 or 1 of the relevant variable.
		assert(variable == 0 || variable == 1);

		// 2. Arc-reduce the constraint with respect to the variable `variable`
		FilteringOutput o = constraint->filter(variable);
		if (o == FilteringOutput::Failure) return o;


		// 3. If we have removed some element from the domain, we insert the related constraints into the worklist in order to reconsider them again.
		if (o == FilteringOutput::Pruned) {
			result = FilteringOutput::Pruned;
			VariableIdx pruned = constraint->getScope()[variable];  // This is the index of the state variable whose domain we have pruned
			for (DirectConstraint::cptr ctr:binary_constraints) {
				if (ctr == constraint) continue;  // No need to reinsert the same constraint we just used.

				// Only if the constraint has overlapping scope, we insert in the worklist the constraint paired with _the other_ variable, to be analysed later.
				const VariableIdxVector& scope = ctr->getScope();
				assert(scope.size() == 2);

				if (pruned == scope[0]) worklist.insert(std::make_pair(ctr, 1));
				else if (pruned == scope[1]) worklist.insert(std::make_pair(ctr, 0));
				else continue;
			}
		}
	}

	return result;
}


FilteringOutput DirectCSPHandler::globalFiltering() const {
	FilteringOutput output = FilteringOutput::Unpruned;
	for (DirectConstraint::cptr constraint:n_ary_constraints) {
		FilteringOutput o = constraint->filter();
		if (o == FilteringOutput::Failure) return o;
		else if (o == FilteringOutput::Pruned) output = o;
	}
	return output;
}


bool DirectCSPHandler::checkConsistency(const DomainMap& domains) {
	for (const auto& domain:domains) {
		if (domain.second->size() == 0) return false; // If any pruned domain is empty, the CSP has no solution.
	}
	return true;
}


//! We select an arbitrary arc, indeed the first according to the order between pairs of procedure IDs and variable IDs.
//! and remove it from the worklist
DirectCSPHandler::Arc DirectCSPHandler::select(ArcSet& worklist) const {
	assert(!worklist.empty());
	auto it = worklist.end();
	--it;
	auto elem = *(it);
	worklist.erase(it);
	return elem;
}

VariableIdxVector DirectCSPHandler::indexRelevantVariables(const std::vector<DirectConstraint::cptr>& constraints) {
	boost::container::flat_set<VariableIdx> relevant;
	for (const DirectConstraint::cptr constraint:constraints) {
		for (VariableIdx variable:constraint->getScope()) {
			relevant.insert(variable);
		}
	}
	return VariableIdxVector(relevant.begin(), relevant.end());
}



} // namespaces
