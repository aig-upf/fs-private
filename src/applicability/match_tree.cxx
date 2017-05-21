
#include <numeric>

#include <applicability/match_tree.hxx>
#include <algorithm>
#include <lapkt/tools/logging.hxx>
#include <problem_info.hxx>
#include <utils/atom_index.hxx>
#include <actions/actions.hxx>
#include <state.hxx>
#include <languages/fstrips/language.hxx>
#include <languages/fstrips/scopes.hxx>
#include <utils/system.hxx>
#include <utils/printers/vector.hxx>
#include <problem.hxx>

namespace fs0 {

    BaseNode::ptr
    BaseNode::create_tree(std::vector<ActionIdx>&& actions, NodeCreationContext& context) {

    	if (actions.empty()) return new EmptyNode;

    	// Only if every item is done, then we create a leaf node
    	bool all_done = true;
    	for (unsigned i = 0; all_done && (i < actions.size()); ++i) {
    		if (!(action_done(actions[i], context))) {
    			all_done = false;
    		}
    	}

    	if (all_done) return new LeafNode(std::move(actions));
		else return new SwitchNode(actions, context);
    }

    VariableIdx
    BaseNode::get_best_variable( const NodeCreationContext& context ) {

		// Return the most-frequent atom that has not yet been seen
		for (VariableIdx var:context._sorted_variables) {
			if (!context._seen[var]) {
				return var;
			}
		}

		throw std::runtime_error("Match-Tree runtime error - No best variable found");
    }

    bool
    BaseNode::action_done( unsigned i, const NodeCreationContext& context ) {
		const AtomIndex& index = context._tuple_index;
		for (const AtomIdx atom_idx:context._rev_app_index[i]) {
			const Atom& atom = index.to_atom(atom_idx);
			if (!context._seen[atom.getVariable()]) return false;
		}
    	return true;
    }


    void LeafNode::generate_applicable_items( const State&, const AtomIndex& tuple_index, std::vector<ActionIdx>& actions ) const {
        actions.insert( actions.end(), _applicable_items.begin(), _applicable_items.end() );
    }

	SwitchNode::~SwitchNode() {
		delete _default_child;
 		for (BaseNode* child:_children) delete child;
	}


    void SwitchNode::generate_applicable_items( const State& s, const AtomIndex& tuple_index, std::vector<ActionIdx>& actions ) const {
        actions.insert( actions.end(), _immediate_items.begin(), _immediate_items.end() );

        // TODO: Change this when mutex's are done proper
        //_children[s.value_for_var(_pivot)]->generate_applicable_items( s, actions );
        // MRJ: Note that here we're dealing with a more general representation than
        // SAS+, where the only atoms that occur on preconditions are of the form
        // X=v.

		ObjectIdx val = s.getValue(_pivot);
		assert((val == 0 || val == 1) && "Match Tree not yet prepared for multivalued variables");

// 		LPT_INFO( "cout", "[Match Tree] Branching on atom " << Atom(_pivot, val) << "");

		// TODO This won't work for a multi-valued match-tree. For that, we'll either need to change 'children' into a map
		// (bad for time performance), or change it into a huge vector mapping any possible value of the variable into the
		// actual node. This could be quite harmful when dealing with large-domain integer variables, etc.
		_children[val]->generate_applicable_items(s, tuple_index, actions);

        _default_child->generate_applicable_items( s, tuple_index, actions );
    }

    SwitchNode::SwitchNode(const std::vector<ActionIdx>& actions, NodeCreationContext& context) :
		_pivot(get_best_variable(context))
	{
		const std::vector<AtomIdx>& pivot_atoms = context._tuple_index.all_variable_atoms(_pivot); // All the atoms that can be derived from the pivot variable
		if (pivot_atoms.size() > 2) throw std::runtime_error("Match Tree only ready for propositional domains yet");
		// if (context._tuple_index.to_atom(pivot_atoms[0]).getValue() != 1) throw std::runtime_error("Match Tree only ready for propositional domains yet");
		
		// 'actions_split_by_pivot_value[i]' will contain all actions whose precondition requires the 'i'-th
		// possible atom that can be derived from the pivot variable to hold
		std::vector<std::vector<ActionIdx>> actions_split_by_pivot_value(pivot_atoms.size());

		std::vector<ActionIdx> dont_care_actions; // All actions for which the value of the pivot variable is irrelevant
			
		// Classify all actions
		for (ActionIdx action:actions) {

			if (action_done(action, context)) {
				_immediate_items.push_back(action);

			} else {

				const std::unordered_set<AtomIdx>& required = context._rev_app_index[action]; // 'required' contains all atoms relevant to the precondition of the inspected action
				bool is_relevant = false; // Will be true iff the pivot variable is relevant to the precondition of the inspected action

				for (unsigned i = 0; i < pivot_atoms.size(); ++i) {
					AtomIdx atom = pivot_atoms[i];
					
					assert(context._tuple_index.to_atom(atom).getValue() == 0 || context._tuple_index.to_atom(atom).getValue() == 1); // Not yet ready for multivalued match tree... soon!
					if (required.find(atom) != required.end()) {
						is_relevant = true;
						actions_split_by_pivot_value[i].push_back(action);
					}
				}

				if (!is_relevant) dont_care_actions.push_back(action);
			}
		}

		// 		auto printer = [](const unsigned& action, std::ostream& os) {
		// 			os << *(Problem::getInstance().getGroundActions()[action]) << "(" << action << ")";
		// 		};
		// 		LPT_INFO("cout", "Creating a switch node on pivot: " << context._tuple_index.to_atom(_pivot) << " with a set of " << actions.size() << " actions");
		// 		LPT_INFO("cout", "Actions which are done: " << print::container(_immediate_items, printer));
		// 		LPT_INFO("cout", "Actions which are relevant(" << actions_split_by_pivot_value[0].size() << "): " << print::container(actions_split_by_pivot_value[0], printer));
		// 		LPT_INFO("cout", "(Index of) Actions which are irrelevant: " << print::container(dont_care_actions));
		// 		LPT_INFO("cout", "(Number of) Actions which are irrelevant: " << dont_care_actions.size());
		// 		if (_immediate_items.size() > 0) throw std::runtime_error("YES");


		context._seen[_pivot] = true;

		// Create the switch generators
		
		if (actions_split_by_pivot_value.size() == 1) {
			// We need to account for the X=0 value
			std::vector<ActionIdx> empty;
			actions_split_by_pivot_value.insert(actions_split_by_pivot_value.begin(), empty);
		}
		for (unsigned i = 0; i < actions_split_by_pivot_value.size(); i++) {
			_children.push_back(create_tree(std::move(actions_split_by_pivot_value[i]), context));
		}

		_default_child = create_tree(std::move(dont_care_actions), context); // Create the default generator

		context._seen[_pivot] = false;
    }

    unsigned SwitchNode::count() const {
        unsigned total = 0;
		for (const auto child:_children) {
            total += child->count();
		}
        total += _default_child->count();
        total += _immediate_items.size();
        return total;
    }

    unsigned SwitchNode::count_nodes() const {
        unsigned total = 1 + _default_child->count_nodes();
		for (const auto child:_children) {
            total += child->count_nodes();
		}
        return total;
    }

	void SwitchNode::count_nodes(unsigned& sw, unsigned& leaf, unsigned& empty) const {
		++sw;
		_default_child->count_nodes(sw, leaf, empty);
		for (const auto child:_children) {
			child->count_nodes(sw, leaf, empty);
		}
	}


    void EmptyNode::print( std::stringstream& stream, std::string indent, const MatchTreeActionManager& manager ) const {
    	stream << indent << "<empty>" << std::endl;
    }

    void LeafNode::print( std::stringstream& stream, std::string indent, const MatchTreeActionManager& manager ) const {
    	for (unsigned i = 0; i < _applicable_items.size(); ++i)
    		stream << indent << *manager._actions[_applicable_items[i]] << std::endl;
    }

    void SwitchNode::print( std::stringstream& stream, std::string indent, const MatchTreeActionManager& manager ) const {
        Atom a = manager._tuple_idx.to_atom( _pivot );
        stream << indent << "switch on " << a << std::endl;
        stream << indent << "immediately:" << std::endl;
        for (unsigned i = 0; i < _immediate_items.size(); ++i)
            stream << indent << *manager._actions[_immediate_items[i]] << std::endl;
        stream << indent << "always:" << std::endl;
        _default_child->print(stream, indent + "  ", manager);

        for (unsigned i = 0; i < _children.size(); ++i) {
            stream << indent << "case " << i << ":" << std::endl;
            _children[i]->print(stream, indent + "  ", manager);
        }
    }

    void
    MatchTreeActionManager::check_match_tree_can_be_used(const ProblemInfo& info) {
		for (unsigned var = 0; var < info.getNumVariables(); ++var) {
			TypeIdx type = info.getVariableType(var);
			if (info.getGenericType(type) == ProblemInfo::ObjectType::INT) {
				LPT_INFO("cout", "ERROR - Match Tree cannot be used with FSTRIPS encodings that contain bound integers. Try the \"naive\" successor generation instead");
				throw std::runtime_error("ERROR - Match Tree cannot be used with FSTRIPS encodings that contain bound integers. Try the \"naive\" successor generation instead");
			}
		}
	}


    MatchTreeActionManager::MatchTreeActionManager( const std::vector<const GroundAction*>& actions,
                                                    const fs::Formula* state_constraints,
                                                    const AtomIndex& tuple_idx)
        : NaiveActionManager(actions, state_constraints),
        _tuple_idx(tuple_idx),
        _tree(nullptr)
    {
		const ProblemInfo& info = ProblemInfo::getInstance();

		check_match_tree_can_be_used(info);

		LPT_INFO("cout", "Mem. usage before applicabilty-analyzer construction: " << get_current_memory_in_kb() << "kB. / " << get_peak_memory_in_kb() << " kB.");

		BasicApplicabilityAnalyzer analyzer(actions, tuple_idx);
		analyzer.build(false);

		LPT_INFO("cout", "Mem. usage after applicabilty-analyzer construction: " << get_current_memory_in_kb() << "kB. / " << get_peak_memory_in_kb() << " kB.");


		// MRJ: This ugly looking Microsoft API like class comes in handy to avoid having to
		// lots of methods with absurdly long signatures. The idea is to progressively move
		// towards a more visitor/creator like implementation, but I don't want to depart too
		// much from Chris' original implementation to help with debugging.
		std::vector<ActionIdx> all_actions(_actions.size());
		std::iota( all_actions.begin(), all_actions.end(), 0);


		std::vector<bool> seen(info.getNumVariables(), false);
		std::vector<VariableIdx> sorted_vars = sort_variables(analyzer.getVariableRelevance());

		NodeCreationContext helper(_tuple_idx, sorted_vars, analyzer.getRevApplicable(), seen);

// 		LPT_INFO("cout", "(K1) Mem. usage: " << get_current_memory_in_kb() << "kB. / " << get_peak_memory_in_kb() << " kB.");
		_tree = new SwitchNode(all_actions, helper);
// 		LPT_INFO("cout", "(K2) Mem. usage: " << get_current_memory_in_kb() << "kB. / " << get_peak_memory_in_kb() << " kB.");
		LPT_INFO("cout", "Match Tree created");


		unsigned sw = 0, leaf = 0, empty = 0;
		_tree->count_nodes(sw, leaf, empty);
		LPT_INFO("cout", "TOTAL NODE COUNT: " <<_tree->count_nodes());
		LPT_INFO("cout", "\tSWITCH: " << sw);
		LPT_INFO("cout", "\tLEAF: " << leaf);
		LPT_INFO("cout", "\tEMPTY: " << empty);
    }


    std::vector<VariableIdx> MatchTreeActionManager::sort_variables(const std::vector<unsigned>& variable_relevance) const {
		const ProblemInfo& info = ProblemInfo::getInstance();


		// This will contain pairs (x,y), where 'x' is the number of times that variable with index 'y'
		// appears on some (distinct) action precondition
		 std::vector<std::pair<unsigned, VariableIdx>> count;
		for (unsigned var = 0; var < info.getNumVariables(); ++var) {
			count.push_back(std::make_pair(variable_relevance[var], var));
		}

		// This will sort by count, breaking ties lexicographically by variable index.
		std::sort(count.begin(), count.end());

		std::vector<VariableIdx> indexes_only;
		indexes_only.reserve(count.size());

// 		LPT_INFO( "cout", "[Match Tree] Size of Variable Selection Heuristic array: " << count.size() );
// 		LPT_INFO( "cout", "[Match Tree] Variables ordered by frequency:");

		for (int i = count.size()-1; i >= 0; --i) {
            if ( count[i].first == 0 ) continue; // the variable is not relevant to any action precondition
			indexes_only.push_back(count[i].second);
// 			LPT_INFO( "cout", info.getVariableName(count[i].second) << " (" << count[i].first << ")");
		}

		assert(!indexes_only.empty());
		return indexes_only;
	}


    std::vector<ActionIdx> MatchTreeActionManager::compute_whitelist(const State& state) const {
    	std::vector<ActionIdx> result;
        _tree->generate_applicable_items( state, _tuple_idx, result );
    	return result;
    }


}
