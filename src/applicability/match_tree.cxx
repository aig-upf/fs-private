
#include <applicability/match_tree.hxx>
#include <algorithm>
#include <aptk2/tools/logging.hxx>
#include <problem_info.hxx>
#include <utils/atom_index.hxx>
#include <actions/actions.hxx>
#include <state.hxx>
#include <languages/fstrips/language.hxx>
#include <languages/fstrips/scopes.hxx>
#include <utils/system.hxx>

namespace fs0 {

    BaseNode::ptr
    BaseNode::create_tree(std::vector<ActionIdx>&& actions, NodeCreationContext& context) {

    	if (actions.empty()){
            return new EmptyNode;
        }

    	// If every item is done, then we create a leaf node
    	bool all_done = true;
    	for (unsigned i = 0; all_done && (i < actions.size()); ++i) {
    		if (!(action_done(actions[i], context))) {
    			all_done = false;
    		}
    	}
    	
    	if (all_done) return new LeafNode(std::move(actions));
		else return new SwitchNode(actions, context);
    }

    AtomIdx
    BaseNode::get_best_atom( const NodeCreationContext& context ) {

		// Return the most-frequent atom that has not yet been seen
		for (AtomIdx atom:context._sorted_atoms) {
			if (!context._seen[atom]) {
				return atom;
			}
		}
		
		throw std::runtime_error("Shouldn't get here");
    }

    bool
    BaseNode::action_done( unsigned i, const NodeCreationContext& context  ) {
    	for (unsigned j = 0; j < context._rev_app_index[i].size(); ++j) {
    		if (!context._seen[context._rev_app_index[i][j]]) {
    			return false;
			}
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

        Atom pivot_atom = tuple_index.to_atom( _pivot );
        if (s.contains(pivot_atom))
            _children[0]->generate_applicable_items( s, tuple_index, actions );

        _default_child->generate_applicable_items( s, tuple_index, actions );
    }

    SwitchNode::SwitchNode(const std::vector<ActionIdx>& actions,  NodeCreationContext& context) {
		static unsigned __count = 0;
		if (__count++ % 100000 == 0) std::cout << "SwitchNode::SwitchNode count " << __count << std::endl;
        _pivot = get_best_atom(context);

        // MRJ: default_items contains the "false" branches
        std::vector<ActionIdx> default_items;

        // MRJ: Atoms can be either false or true, so actions
        // either feature or not those atoms
		std::vector<std::vector<ActionIdx>> value_items;
		value_items.push_back( std::vector<ActionIdx>() );


        // Sort out the regression items
		for (ActionIdx action:actions) {
            if (action_done(action, context)) {
                _immediate_items.push_back(action);
                continue;
            }
            const std::vector<AtomIdx>& required = context._rev_app_index[action];
			// std::cout << "std::find() on vector of size: " << required.size() << std::endl;
            if ( std::find( required.begin(), required.end(), _pivot ) != required.end() )  {
                value_items[0].push_back(action);
                continue;
            }
            default_items.push_back(action);
        }


        context._seen[_pivot] = true;

        _children.push_back(create_tree(std::move(value_items[0]), context)); // Create the switch generators
        _default_child = create_tree(std::move(default_items), context); // Create the default generator
		
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
		
        LPT_INFO("cout", "[Match Tree]  MatchTreeActionManager::MatchTreeActionManager() starts...");
        LPT_INFO("cout", "Current mem. usage: " << get_current_memory_in_kb() << " kB.");


		// MRJ: This code below builds the reverse applicability index, mapping action indices into sets of atoms making up their preconditions
        std::vector<std::vector<AtomIdx>>    rev_app_index(actions.size());

        for (unsigned i = 0; i < _actions.size(); ++i) {
    		const GroundAction& action = *_actions[i];


    		const fs::Conjunction* precondition = dynamic_cast<const fs::Conjunction*>(action.getPrecondition());
    		if (!precondition) { // If the precondition is not a conjunction, we cannot say much, so we consider the action as always potentially applicable
    			continue;
    		}


    		std::set<VariableIdx> referenced; // The state variables already made reference to by some precondition
    		for (const fs::AtomicFormula* conjunct:precondition->getConjuncts()) {

    			const fs::RelationalFormula* rel = dynamic_cast<const fs::RelationalFormula*>(conjunct);
    			const fs::EQAtomicFormula* eq = dynamic_cast<const fs::EQAtomicFormula*>(conjunct);
    			const fs::NEQAtomicFormula* neq = dynamic_cast<const fs::NEQAtomicFormula*>(conjunct);
    			unsigned nestedness = conjunct->nestedness();
    			std::vector<VariableIdx> all_relevant = fs::ScopeUtils::computeDirectScope(conjunct);

    			// This implements a very rudimentary test that indexes only preconditions of the form X = x or X != x,
    			// furthermore assuming that there are no two preconditions making reference to the same state variable
    			if (nestedness > 0 || all_relevant.size() != 1 || !(eq || neq)) continue;

    			const fs::StateVariable* sv = dynamic_cast<const fs::StateVariable*>(rel->lhs());
    			if (!sv) continue;

    // 			std::cout << "Processing action #" << i << ": " << action << std::endl;
    // 			std::cout << "Processing conjunct: " << *conjunct << std::endl;

    			VariableIdx relevant = all_relevant[0];
    			const std::vector<ObjectIdx>& values = info.getVariableObjects(relevant);

    			if (!referenced.insert(relevant).second) {
    				throw std::runtime_error("BasicApplicabilityAnalyzer requires that no two preconditions make reference to the same state variable");
    			}

    			if (eq) { // Prec is of the form X=x
    				ObjectIdx value = _extract_constant_val(eq->lhs(), eq->rhs());
    				AtomIdx tup = _tuple_idx.to_index(relevant, value);
    				rev_app_index[i].push_back(tup);

    			} else { // Prec is of the form X!=x
    				assert(neq);
    				ObjectIdx value = _extract_constant_val(neq->lhs(), neq->rhs());
    				for (ObjectIdx v2:values) {
    					if (v2 != value) {
    						AtomIdx tup = _tuple_idx.to_index(relevant, v2);
    						rev_app_index[i].push_back(tup);
    					}
    				}
    			}
    		}
        }
        LPT_INFO( "cout", "[Match Tree]  Atom -> Action Index built...");
        LPT_INFO("cout", "Current mem. usage: " << get_current_memory_in_kb() << " kB.");


        BasicApplicabilityAnalyzer analyzer(actions, tuple_idx);
		analyzer.build();
        LPT_INFO( "cout", "[Match Tree]  Basic Analyzer built...");
        LPT_INFO("cout", "Current mem. usage: " << get_current_memory_in_kb() << " kB.");

        // MRJ: This ugly looking Microsoft API like class comes in handy to avoid having to
        // lots of methods with absurdly long signatures. The idea is to progressively move
        // towards a more visitor/creator like implementation, but I don't want to depart too
        // much from Chris' original implementation to help with debugging.
        std::vector<ActionIdx> all_actions(_actions.size());
        std::iota( all_actions.begin(), all_actions.end(), 0);
		
		
		std::vector<bool> seen(_tuple_idx.size(), false);
		std::vector<unsigned> sorted_atoms = sort_atom_idxs(analyzer.getApplicable());
		
        NodeCreationContext helper(_tuple_idx, sorted_atoms, rev_app_index, seen);
		
		LPT_INFO("cout", "(K1) Mem. usage: " << get_current_memory_in_kb() << "kB. / " << get_peak_memory_in_kb() << " kB.");
		_tree = new SwitchNode(all_actions, helper);
		LPT_INFO("cout", "(K2) Mem. usage: " << get_current_memory_in_kb() << "kB. / " << get_peak_memory_in_kb() << " kB.");
        LPT_INFO("cout", "Match Tree created");
		
		/*
        #ifdef EDEBUG
        std::stringstream buffer;
        _tree->print( buffer, "", *this );
        LPT_EDEBUG("main", "\n" << buffer.str() );
        #endif
        */
    }
    
    
    std::vector<unsigned> MatchTreeActionManager::sort_atom_idxs(const std::vector<std::vector<ActionIdx>>& applicability_idx) const {
		
		// This will contain pairs (x,y), where 'x' is the number of times that atom with index 'y'
		// appears on some action precondition
		 std::vector<std::pair<int, unsigned>> atom_count(_tuple_idx.size());
		for (unsigned i = 0; i < _tuple_idx.size(); ++i) {
			atom_count[i] = std::make_pair(applicability_idx[i].size(), i);
		}

		// This will sort by count, breaking ties lexicographically by atom index.
		std::sort(atom_count.begin(), atom_count.end());
		
		LPT_INFO( "cout", "[Match Tree] Size of Variable Selection Heuristic array: " << atom_count.size() );
		LPT_INFO("cout", "(B2) Mem. usage: " << get_current_memory_in_kb() << "kB. / " << get_peak_memory_in_kb() << " kB.");
			
		std::vector<unsigned> indexes_only;
		indexes_only.reserve(atom_count.size());
		
		for (int i = atom_count.size()-1; i >= 0; --i) {
			indexes_only.push_back(atom_count[i].second);
		}
		return indexes_only;
	}


    std::vector<ActionIdx> MatchTreeActionManager::compute_whitelist(const State& state) const {
    	std::vector<ActionIdx> result;
        _tree->generate_applicable_items( state, _tuple_idx, result );
    	return result;
    }


}
