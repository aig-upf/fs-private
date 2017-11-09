
#pragma once

namespace fs0 {
	//! The possible results of the filtering process
	enum class FilteringOutput {Failure, Pruned, Unpruned};
	
	/**
	 * The type of filtering offered by a particular constraint class.
	 *   - Unary: unary constraints filter directly the domain.
	 *   - ArcReduction: Binary constraints arc-reduce their domains wrt one of the two variables of their scope. This is used in the AC3 algorithm.
	 *   - Custom: Other types of constraints might apply any other filtering to ensure some type of local consistency.
	 */
	enum class FilteringType {Unary, ArcReduction, Custom};
} // namespaces