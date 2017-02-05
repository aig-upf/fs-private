
#include <applicability/match_tree.hxx>
#include <algorithm>
#include <aptk2/tools/logging.hxx>
#include <problem_info.hxx>
#include <utils/atom_index.hxx>
#include <actions/actions.hxx>
#include <state.hxx>
#include <languages/fstrips/language.hxx>
#include <languages/fstrips/scopes.hxx>

namespace fs0 {


NodeCreationContext::NodeCreationContext(    const std::vector<ActionIdx>& actions,
					const AtomIndex& tuple_index,
					const std::vector<std::vector<ActionIdx>>& app_index,
					const std::vector<std::vector<AtomIdx>>& rev_app_index)
: _actions( actions ), _tuple_index( tuple_index ), _app_index( app_index), _rev_app_index(rev_app_index), _seen(tuple_index.size(), false)
{}
		
    BaseNode::ptr
    BaseNode::create_tree(  NodeCreationContext& context ) {

    	if (context._actions.empty())
    		return new EmptyNode;

    	// If every item is done, then we create a leaf node
    	bool all_done = true;
    	for (unsigned i = 0; all_done && (i < context._actions.size()); ++i) {
    		if (!(action_done(context._actions[i], context))) {
    			all_done = false;
    		}
    	}

    	if (all_done)
    		return new LeafNode(context._actions);
		return new SwitchNode(context);

    }

    AtomIdx
    BaseNode::get_best_atom( NodeCreationContext& context ) {

    	// TODO: This fluents.size() stuff needs to change to the number of mutexes once they're computed

    	static std::vector< std::pair<int,int> > var_count = std::vector< std::pair<int,int> >(context._tuple_index.size());
        static bool initialised = false;


        // MRJ: The atom selection heuristic chooses the atom that appears on the most
        // action preconditions, breaking ties lexicographically (see the second field of
        // the pairs in var_count).
        if (!initialised) {
        	for (unsigned i = 0; i < context._tuple_index.size(); ++i)
        		var_count[i] = std::make_pair( context._app_index[i].size(), i);

        	std::sort(var_count.begin(), var_count.end());
            initialised = true;
        }

    	for (int i = var_count.size() - 1; i >= 0; --i) {
    		if (!context._seen[var_count[i].second]) {
    			//cout << "Best var " << var_count[i].second << " with a count of " << var_count[i].first << endl;
                // We return the atom index
    			return var_count[i].second;
    		}
    	}

    	assert(false);
    	return INVALID_TUPLE;
    }

    bool
    BaseNode::action_done( unsigned i, NodeCreationContext& context  ) {
    	for (unsigned j = 0; j < context._rev_app_index[i].size(); ++j)
    		if (!context._seen[context._rev_app_index[i][j]])
    			return false;

    	return true;
    }


    LeafNode::LeafNode( const std::vector<AtomIdx>& actions ) {
    	_applicable_items.assign(actions.begin(),actions.end());
    }

    void LeafNode::generate_applicable_items( const State&, const AtomIndex& tuple_index, std::vector<ActionIdx>& actions ) {
        actions.insert( actions.end(), _applicable_items.begin(), _applicable_items.end() );
    }

	SwitchNode::~SwitchNode() {
		delete _default_child;
		for (BaseNode* child:_children) delete child;
	}


    void SwitchNode::generate_applicable_items( const State& s, const AtomIndex& tuple_index, std::vector<ActionIdx>& actions ) {
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

    SwitchNode::SwitchNode( NodeCreationContext& context ) {

        _pivot = get_best_atom(context);

        std::vector< std::vector<ActionIdx> > value_items;
        // MRJ: default_items contains the "false" branches
        std::vector<ActionIdx> default_items;

        // MRJ: Atoms can be either false or true, so actions
        // either feature or not those atoms
        value_items.push_back( std::vector<ActionIdx>() );


        // Sort out the regression items
        for (unsigned i = 0; i < context._actions.size(); i++) {
            if (action_done(context._actions[i], context)) {
                _immediate_items.push_back(context._actions[i]);
                continue;
            }
            const auto& required = context._rev_app_index[ context._actions[i] ];
            if ( std::find( required.begin(), required.end(), _pivot ) != required.end() )  {
                value_items[0].push_back(context._actions[i]);
                continue;
            }
            default_items.push_back(context._actions[i]);
        }


        context._seen[_pivot] = true;

        // Create the switch generators
        NodeCreationContext true_context( value_items[0], context._tuple_index, context._app_index, context._rev_app_index );
        true_context._seen = std::move(context._seen);
        _children.push_back(create_tree(true_context));
        context._seen = std::move(true_context._seen);

        // Create the default generator
        NodeCreationContext false_context( default_items, context._tuple_index, context._app_index, context._rev_app_index );
        false_context._seen = std::move(context._seen);
        _default_child = create_tree(false_context);
        context._seen = std::move(false_context._seen);

		context._seen[_pivot] = false;
    }

    int SwitchNode::count() const {
        int total = 0;
        for (unsigned i = 0; i < _children.size(); ++i)
            total += _children[i]->count();
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


    MatchTreeActionManager::MatchTreeActionManager( const std::vector<const GroundAction*>& actions,
                                                    const fs::Formula* state_constraints,
                                                    const AtomIndex& tuple_idx,
                                                    const BasicApplicabilityAnalyzer& analyzer)
        : NaiveActionManager(actions, state_constraints),
        _tuple_idx(tuple_idx),
        _app_index(analyzer.getApplicable()),
        _rev_app_index(),
        _tree(nullptr)
    {
        const ProblemInfo& info = ProblemInfo::getInstance();
        // MRJ: This code below builds the reverse applicability index
        _rev_app_index.resize(actions.size());


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
    				_rev_app_index[i].push_back(tup);

    			} else { // Prec is of the form X!=x
    				assert(neq);
    				ObjectIdx value = _extract_constant_val(neq->lhs(), neq->rhs());
    				for (ObjectIdx v2:values) {
    					if (v2 != value) {
    						AtomIdx tup = _tuple_idx.to_index(relevant, v2);
    						_rev_app_index[i].push_back(tup);
    					}
    				}
    			}
    		}
        }


        // MRJ: This ugly looking Microsoft API like class comes in handy to avoid having to
        // lots of methods with absurdly long signatures. The idea is to progressively move
        // towards a more visitor/creator like implementation, but I don't want to depart too
        // much from Chris' original implementation to help with debugging.
        std::vector<ActionIdx> action_indices(_actions.size());
        std::iota( action_indices.begin(), action_indices.end(), 0);
        NodeCreationContext helper(action_indices, _tuple_idx, _app_index, _rev_app_index );
        _tree = BaseNode::create_tree( helper );

        LPT_INFO("main", "Match Tree created");
        std::stringstream buffer;
        #ifdef DEBUG
        _tree->print( buffer, "", *this );
        LPT_DEBUG("main", "\n" << buffer.str() );
        #endif

    }


    std::vector<ActionIdx> MatchTreeActionManager::compute_whitelist(const State& state) const {
    	std::vector<ActionIdx> result;
        _tree->generate_applicable_items( state, _tuple_idx, result );
    	return result;
    }


}
