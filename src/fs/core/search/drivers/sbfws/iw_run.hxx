
#pragma once

#include <unordered_set>

#include <lapkt/tools/resources_control.hxx>
#include <lapkt/tools/logging.hxx>

#include <fs/core/problem.hxx>

#include <fs/core/search/drivers/sbfws/base.hxx>
#include <fs/core/search/drivers/sbfws/iw_run_config.hxx>
#include <fs/core/search/drivers/sbfws/stats.hxx>
#include <fs/core/search/drivers/sbfws/simulation_evaluators.hxx>

#include <fs/core/search/drivers/sbfws/relevant_atomset.hxx>
#include <utility>
#include <fs/core/utils/printers/vector.hxx>
#include <fs/core/utils/printers/actions.hxx>
#include <lapkt/search/components/open_lists.hxx>
#include <fs/core/utils/config.hxx>


namespace fs0::bfws {

template <typename StateT, typename ActionType>
class IWRunNode {
public:
    using ActionT = ActionType;
    using PT = std::shared_ptr<IWRunNode<StateT, ActionT>>;

    //! The state in this node
    StateT state;

    //! The action that led to this node
    typename ActionT::IdType action;

    //! The parent node
    PT parent;

    //! Accummulated cost
    unsigned g;

// 	bool satisfies_subgoal; // Whether the node satisfies some subgoal

    //! The novelty  of the state
    unsigned char _w;

    //! The indexes of the variables whose atoms form the set 1(s), which contains all atoms in 1(parent(s)) not deleted by the action that led to s, plus those
    //! atoms in s with novelty 1.
// 	std::vector<unsigned> _nov1atom_idxs;

    //! Implicit encoding of the atoms that contribute novelty 1 to the state
// 	boost::dynamic_bitset<> _B_of_s;

    //! Whether the path-novely of the node is one
// 	bool _path_novelty_is_1;

    std::vector<std::pair<AtomIdx, AtomIdx>> _nov2_pairs;

    //! The generation order, uniquely identifies the node
    //! NOTE We're assuming we won't generate more than 2^32 ~ 4.2 billion nodes.
    uint32_t _gen_order;


    IWRunNode() = default;
    ~IWRunNode() = default;
    IWRunNode(const IWRunNode&) = default;
    IWRunNode(IWRunNode&&) = delete;
    IWRunNode& operator=(const IWRunNode&) = delete;
    IWRunNode& operator=(IWRunNode&&) = delete;

    //! Constructor with full copying of the state (expensive)
    IWRunNode(const StateT& s, unsigned long gen_order) : IWRunNode(StateT(s), ActionT::invalid_action_id, nullptr, gen_order) {}

    //! Constructor with move of the state (cheaper)
    IWRunNode(StateT&& _state, typename ActionT::IdType _action, PT _parent, uint32_t gen_order) :
        state(std::move(_state)),
//		feature_valuation(0),
        action(_action),
        parent(_parent),
        g(parent ? parent->g+1 : 0),
        _w(std::numeric_limits<unsigned char>::max()),
// 		_path_novelty_is_1(false),
// 		_B_of_s(state.numAtoms()),
        _gen_order(gen_order)
    {
        assert(_gen_order > 0); // Very silly way to detect overflow, in case we ever generate > 4 billion nodes :-)
    }


    bool has_parent() const { return parent != nullptr; }

    //! Print the node into the given stream
    friend std::ostream& operator<<(std::ostream &os, const IWRunNode<StateT, ActionT>& object) { return object.print(os); }
    std::ostream& print(std::ostream& os) const {
// 		const Problem& problem = Problem::getInstance();
        os << "{@ = " << this;
        os << ", #=" << _gen_order ;
        os << ", s = " << state ;
        os << ", g=" << g ;
        //os << ", w=" << (_evaluated ? (_w == std::numeric_limits<unsigned char>::max() ? "INF" : std::to_string(_w)) : "?") ;
        os << ", w=" << (_w == std::numeric_limits<unsigned char>::max() ? "INF" : std::to_string(_w));

        os << ", act=" << action ;
// 		if (action < std::numeric_limits<unsigned>::max()) {
// 			os << ", act=" << *(problem.getGroundActions()[action]) ;
// 			os << fs0::print::full_action(*(problem.getGroundActions()[action]));
// 		} else {
// 			os << ", act=" << "NONE" ;
// 		}

        os << ", parent = " << (parent ? "#" + std::to_string(parent->_gen_order) : "None");
        return os;
    }

    bool operator==( const IWRunNode<StateT, ActionT>& o ) const { return state == o.state; }

    std::size_t hash() const { return state.hash(); }
};


//! A single IW run (with parametrized max. width) that runs until (independent)
//! satisfaction of each of the provided goal atoms, and computes the set
//! of atoms R that is relevant for the achievement of at least one atom.
//! R is computed treating the actions as a black-box. For this, an atom is considered
//! relevant for a certain goal atom if that atom is true in at least one of the states
//! that lies on the path between the seed node and the first node where that goal atom is
//! satisfied.
template <typename NodeT,
          typename StateModel,
          typename NoveltyEvaluatorT,
          typename FeatureSetT
>
class IWRun
{
public:
    using ActionT = typename StateModel::ActionType;
    using StateT = typename StateModel::StateT;

    using ActionIdT = typename StateModel::ActionType::IdType;
    using NodePT = std::shared_ptr<NodeT>;

    using FeatureValueT = typename NoveltyEvaluatorT::FeatureValueT;

    using OpenListT = lapkt::SimpleQueue<NodeT>;


protected:
    //! The search model
    const StateModel& _model;

    //! The simulation configuration
    IWRunConfig _config;

    //!
    std::vector<NodePT> _optimal_paths;

    //! '_unreached' contains the indexes of all those goal atoms that have yet not been reached.
    std::unordered_set<unsigned> _unreached;

    //! Contains the indexes of all those goal atoms that were already reached in the seed state
    std::vector<bool> _in_seed;

    //! A single novelty evaluator will be in charge of evaluating all nodes
    std::unique_ptr<SimulationEvaluatorI<NodeT>> _evaluator;

    //! Some node counts
    uint32_t _generated;
    uint32_t _w1_nodes_expanded;
    uint32_t _w2_nodes_expanded;
    uint32_t _w1_nodes_generated;
    uint32_t _w2_nodes_generated;
    uint32_t _w_gt2_nodes_generated;

    //! The general statistics of the search
    BFWSStats& _stats;

    //! Whether to print some useful extra information or not
    bool _verbose;

public:

    //! Constructor
    IWRun(const StateModel& model, const FeatureSetT& featureset, NoveltyEvaluatorT* evaluator, IWRunConfig config, BFWSStats& stats, bool verbose) :
        _model(model),
        _config(std::move(config)),
        _optimal_paths(model.num_subgoals()),
        _unreached(),
        _in_seed(),
        _evaluator(),
        _generated(1),
        _w1_nodes_expanded(0),
        _w2_nodes_expanded(0),
        _w1_nodes_generated(0),
        _w2_nodes_generated(0),
        _w_gt2_nodes_generated(0),
        _stats(stats),
        _verbose(verbose)
    {
        if (_config._use_achiever_evaluator) {
            const auto& actions = _model.getTask().getGroundActions();
            std::vector<SASPlusOperator> operators;
            operators.reserve(actions.size());
            for (const auto& a:actions) operators.emplace_back(compile_action_to_plan_operator(*a));

            AchieverNoveltyConfiguration ach_config(
                _config.global.template getOption<unsigned long>("sim.max_table_size", 9999999),
                _config.global.template getOption<bool>("sim.early_break", false)
            );
            _evaluator = create_achiever_evaluator<NodeT, FeatureSetT, NoveltyEvaluatorT>(
                    _model.getTask(), featureset, operators, ach_config
            );

        } else {
            using SimEvaluatorT = SimulationEvaluator<NodeT, FeatureSetT, NoveltyEvaluatorT>;
            _evaluator = std::make_unique<SimEvaluatorT>(featureset, evaluator);
        }
    }

    void reset() {
        std::vector<NodePT> _(_optimal_paths.size(), nullptr);
        _optimal_paths.swap(_);
        _generated = 1;
        _w1_nodes_expanded = 0;
        _w2_nodes_expanded = 0;
        _w1_nodes_generated = 0;
        _w2_nodes_generated = 0;
        _w_gt2_nodes_generated = 0;
        _evaluator->reset();
    }

    ~IWRun() = default;

    // Disallow copy, but allow move
    IWRun(const IWRun&) = delete;
    IWRun(IWRun&&) = default;
    IWRun& operator=(const IWRun&) = delete;
    IWRun& operator=(IWRun&&) = default;


    //! Mark all atoms in the path to some goal. 'seed_nodes' contains all nodes satisfying some subgoal.
    void mark_atoms_in_path_to_subgoal(const std::vector<NodePT>& seed_nodes, std::vector<bool>& atoms) const {
        const AtomIndex& index = _model.getTask().get_tuple_index();
        std::unordered_set<NodePT> all_visited;
        assert(atoms.size() == index.size());

        for (NodePT node:seed_nodes) {

            NodePT root = node;
            // We ignore s0
            while (node->has_parent()) {
                // If the node has already been processed, no need to do it again, nor to process the parents,
                // which will necessarily also have been processed.
                auto res = all_visited.insert(node);
                if (!res.second) break;

                const StateT& state = node->state;
                const StateT& parent_state = node->parent->state;
                _unused(state);

                for (const auto& p_q:node->_nov2_pairs) {
                    Atom p = index.to_atom(p_q.first);
                    Atom q = index.to_atom(p_q.second);

                    assert(state.contains(p));
                    assert(state.contains(q));
                    assert(!(parent_state.contains(p) && parent_state.contains(q))); // Otherwise the tuple couldn't be new


                    std::vector<AtomIdx> to_mark;
                    if (parent_state.contains(p)) {
                        if (int(p.getValue()) != 0) {
                            atoms[p_q.first] = true; // TODO THIS WON'T GENERALIZE WELL TO FSTRIPS DOMAINS
// 							std::cout << "ATTENTION : Marking atom " << p << std::endl;
                        }
                    }
                    else if (parent_state.contains(q)) {
                        if (int(q.getValue()) != 0) {
                            atoms[p_q.second] = true;
// 							std::cout << "ATTENTION : Marking atom " << q << std::endl;
                        }
                    }
                    else { // The parent state contains none
// 						std::cout << "ATTENTION : Would Mark pair " << p << ", " << q << std::endl;
                        if (int(p.getValue()) != 0) atoms[p_q.first] = true;
                        if (int(q.getValue()) != 0) atoms[p_q.second] = true;
                    }

                }

                /*
                const StateT& state = node->state;
                for (unsigned var = 0; var < state.numAtoms(); ++var) {
                    if (state.getValue(var) == 0) continue; // TODO THIS WON'T GENERALIZE WELL TO FSTRIPS DOMAINS
                    AtomIdx atom = index.to_index(var, state.getValue(var));
                    atoms[atom] = true;
                }
                */

                node = node->parent;
            }
        }
    }

    std::vector<bool> mark_all_atoms_in_path_to_subgoal(const std::vector<NodePT>& seed_nodes) const {
        const AtomIndex& index = _model.getTask().get_tuple_index();
        std::vector<bool> atoms(index.size(), false);
        std::unordered_set<NodePT> all_visited;
        assert(atoms.size() == index.size());

        for (NodePT node:seed_nodes) {
            // We ignore s0
            while (node->has_parent()) {
                // If the node has already been processed, no need to do it again, nor to process the parents,
                // which will necessarily also have been processed.
                auto res = all_visited.insert(node);
                if (!res.second) break;

                const StateT& state = node->state;
                for (unsigned var = 0; var < state.numAtoms(); ++var) {
                    object_id val = state.getValue(var);
                    if (unsigned (val) != 0) {
                        atoms[index.to_index(var, val)] = true;
                    }
                }

                node = node->parent;
            }
        }
        return atoms;
    }

    void report_simulation_stats(float simt0) {
        _stats.simulation();
        _stats.sim_add_time(aptk::time_used() - simt0);
        _stats.sim_add_expanded_nodes(_w1_nodes_expanded+_w2_nodes_expanded);
        _stats.sim_add_generated_nodes(_w1_nodes_generated+_w2_nodes_generated+_w_gt2_nodes_generated);
        _stats.reachable_subgoals( _model.num_subgoals() - _unreached.size());
    }

    std::vector<bool> compute_R_all() {
        const AtomIndex& index = _model.getTask().get_tuple_index();
        _stats.r_type(1);
        std::vector<bool> all(index.size(), false);
        for (unsigned i = 0; i < all.size(); ++i) {
            const Atom& atom = index.to_atom(i);
            if (unsigned(atom.getValue())!=0) all[i] = true;
        }
        LPT_INFO("search", "Simulation - Computed R_All set with " << std::count(all.cbegin(), all.cend(), true) << " atoms");
        return all;
    }

    std::vector<bool> compute_R(const StateT& seed) {

        if (_config._force_R_all) {
            if (_verbose) LPT_INFO("cout", "Simulation - R=R[All] is the user-preferred option");
            return compute_R_all();
        }

        if (_config._r_g_prime) {
            if (_verbose) LPT_INFO("cout", "Simulation - R=R'_G is the user-preferred option");
            return compute_R_g_prime(seed);
        }

        if (_config._max_width == 1){
            return compute_plain_R1(seed);
        } else if (_config._max_width == 2){
            return compute_plain_RG2(seed);
        } else {
            throw std::runtime_error("Simulation max_width too high");
        }
    }

    std::vector<bool> compute_coupled_features( const std::vector<bool>& ) {
        return std::vector<bool>();
    }

    std::vector<bool> compute_plain_RG2(const StateT& seed) {
        assert(_config._max_width == 2);
        auto simt0 = aptk::time_used();
        run(seed, _config._max_width);
        report_simulation_stats(simt0);

        LPT_INFO("search", "Simulation - IW(" << _config._max_width << ") run reached " << _model.num_subgoals() - _unreached.size() << " goals");
        return extract_R_G(true);
    }


    std::vector<bool> compute_plain_R1(const StateT& seed) {
        assert(_config._max_width == 1);

        auto simt0 = aptk::time_used();
        run(seed, _config._max_width);
        report_simulation_stats(simt0);

        if (_config._goal_directed && _unreached.size() == 0) {
            LPT_INFO("search", "Simulation - IW(" << _config._max_width << ") reached all subgoals, computing R_G[" << _config._max_width << "]");
            return extract_R_G_1();
        }

        // Else, compute the goal-unaware version of R containing all atoms seen during the IW run
        return extract_R_1();
    }

    std::vector<bool> extract_R_1() {
        std::vector<bool> R = _evaluator->reached_atoms();
        LPT_INFO("search", "Simulation - IW(" << _config._max_width << ") run reached " << _model.num_subgoals() - _unreached.size() << " goals");
        if (_verbose) {
            unsigned c = std::count(R.begin(), R.end(), true);
            LPT_INFO("search", "Simulation - |R[1]| = " << c);
            _stats.relevant_atoms(c);
        }
        return R;
    }

    std::vector<bool> compute_R_g_prime(const StateT& seed) {
		auto simt0 = aptk::time_used();
        run(seed, 1);
		report_simulation_stats(simt0);

        if (_unreached.size() == 0) {
            std::vector<NodePT> seed_nodes = extract_seed_nodes();
            std::vector<bool> R_G = mark_all_atoms_in_path_to_subgoal(seed_nodes);
            unsigned R_G_size = std::count(R_G.begin(), R_G.end(), true);
            LPT_INFO("cout", "Simulation - IW(1) run reached all goals");
            LPT_INFO("cout", "Simulation - |R_G'[1]| = " << R_G_size << " (computed from " << seed_nodes.size() << " subgoal-reaching nodes)");
            _stats.relevant_atoms(R_G_size);
            return R_G;
        }

        LPT_INFO("cout", "Simulation - IW(1) run did not reach all goals");

        if (_config._max_width == 1) {
            LPT_INFO("cout", "Simulation - Max. simulation width set to 1, falling back to R=R_all");
            return compute_R_all();
        }

        if (_config._gr_actions_cutoff < std::numeric_limits<unsigned>::max()) {
            unsigned num_actions =  _model.getTask().getGroundActions().size();
            if (num_actions > _config._gr_actions_cutoff) { // Too many actions to compute IW(2)
                LPT_INFO("cout", "Simulation - Number of actions (" << num_actions << " > " << _config._gr_actions_cutoff << ") considered too high to run IW(2).");
                return compute_R_all();
            } else {
                    LPT_INFO("cout", "Simulation - Number of actions (" << num_actions << " <= " << _config._gr_actions_cutoff << ") considered low enough to run IW(2).");
            }
        }
        LPT_INFO("cout", "Simulation - Throwing IW(2) simulation");
		reset();
        run(seed, 2);
		report_simulation_stats(simt0);
		_stats.reachable_subgoals( _model.num_subgoals() - _unreached.size());

        if (_unreached.size() == 0) {
            std::vector<NodePT> seed_nodes = extract_seed_nodes();
            std::vector<bool> R_G = mark_all_atoms_in_path_to_subgoal(seed_nodes);
            unsigned R_G_size = std::count(R_G.begin(), R_G.end(), true);
            LPT_INFO("cout", "Simulation - IW(2) run reached all goals");
            LPT_INFO("cout", "Simulation - |R_G'[2]| = " << R_G_size << " (computed from " << seed_nodes.size() << " subgoal-reaching nodes)");
            _stats.relevant_atoms(R_G_size);
            return R_G;
        }

        LPT_INFO("cout", "Simulation - IW(2) run did not reach all goals, falling back to R=R_all");
        return compute_R_all();
    }

    //! Extracts the goal-oriented set of relevant atoms after a simulation run
    std::vector<bool> extract_R_G(bool r_all_fallback) {
        const AtomIndex& index = _model.getTask().get_tuple_index();
        /*
        for (unsigned subgoal_idx = 0; subgoal_idx < _all_paths.size(); ++subgoal_idx) {
            const std::vector<NodePT>& paths = _all_paths[subgoal_idx];
            assert(_in_seed[subgoal_idx] || !paths.empty());
            seed_nodes.insert(seed_nodes.end(), paths.begin(), paths.end());
        }
        */


        if (r_all_fallback) {
            unsigned num_subgoals = _model.num_subgoals();
            unsigned initially_reached = std::count(_in_seed.begin(), _in_seed.end(), true);
            unsigned reached_by_simulation = num_subgoals - _unreached.size() - initially_reached;
            if (_verbose) LPT_INFO("cout", "Simulation - " << reached_by_simulation << " subgoals were newly reached by the simulation.");
            bool decide_r_all = (reached_by_simulation < (0.5*num_subgoals));
            decide_r_all = _unreached.size() != 0; // XXX Use R_All is any non-reached
            if (decide_r_all) {
                if (_verbose) LPT_INFO("cout", "Simulation - Falling back to R=R[All]");
                _stats.r_type(1);
                return compute_R_all();
            } else {
                if (_verbose) LPT_INFO("cout", "Simulation - Computing R_G");
            }
        }


        std::vector<NodePT> seed_nodes = extract_seed_nodes();
        std::vector<bool> R_G(index.size(), false);
        mark_atoms_in_path_to_subgoal(seed_nodes, R_G);

        unsigned R_G_size = std::count(R_G.begin(), R_G.end(), true);
        if (_verbose) {
            LPT_INFO("cout", "Simulation - |R_G[" << _config._max_width << "]| = " << R_G_size << " (computed from " << seed_nodes.size() << " subgoal-reaching nodes)");
            if (R_G_size) {
                LPT_INFO("cout", "Simulation - R_G:");
                std::cout << "\t\t";
                for (unsigned i = 0; i < R_G.size(); ++i) {
                    if (R_G[i]) std::cout << index.to_atom(i) << ", ";
                }
                std::cout << std::endl;
            }
        }
        _stats.relevant_atoms(R_G_size);
        _stats.r_type(2);

        return R_G;
    }

    std::vector<NodePT> extract_seed_nodes() {
        std::vector<NodePT> seed_nodes;
        for (unsigned subgoal_idx = 0; subgoal_idx < _optimal_paths.size(); ++subgoal_idx) {
            if (!_in_seed[subgoal_idx] && _optimal_paths[subgoal_idx] != nullptr) {
                seed_nodes.push_back(_optimal_paths[subgoal_idx]);
            }
        }
        return seed_nodes;
    }

    std::vector<bool> extract_R_G_1() {
        std::vector<NodePT> seed_nodes = extract_seed_nodes();
        std::vector<bool> R_G = mark_all_atoms_in_path_to_subgoal(seed_nodes);

        unsigned R_G_size = std::count(R_G.begin(), R_G.end(), true);
        if (_verbose) {
            LPT_INFO("cout", "Simulation - |R_G[" << _config._max_width << "]| = " << R_G_size << " (computed from " << seed_nodes.size() << " subgoal-reaching nodes)");
        }
        _stats.relevant_atoms(R_G_size);
        _stats.r_type(2);

        return R_G;
    }

    bool run(const StateT& seed, unsigned max_width) {
        if (_verbose) LPT_INFO("cout", "Simulation - Starting IW(" << max_width << ") Simulation");

        NodePT root = std::make_shared<NodeT>(seed, _generated++);
        mark_seed_subgoals(root);

        auto nov =_evaluator->evaluate(*root);
        assert(nov==1);
        update_novelty_counters_on_generation(nov);

// 		LPT_DEBUG("cout", "Simulation - Seed node: " << *root);
        OpenListT open;

        open.insert(root);
        auto simt0 = aptk::time_used();

//        std::cout << "Novelty of root node: " << _evaluator->evaluate(*root) << std::endl;

        // Note that we don't used any closed list / duplicate detection of any kind, but let the novelty engine take care of that
        while (!open.empty()) {
            NodePT current = open.next();

            // Expand the node
            update_novelty_counters_on_expansion(current->_w);

            for (const auto& a : _model.applicable_actions(current->state)) {
                StateT s_a = _model.next(current->state, a);
                NodePT successor = std::make_shared<NodeT>(std::move(s_a), a, current, _generated++);

                successor->_w = _evaluator->evaluate(*successor);
                update_novelty_counters_on_generation(successor->_w);

//                 LPT_INFO("cout", "Simulation - Node generated with w=" << (unsigned) successor->_w << ": "  << std::endl<< *successor << std::endl);

                if (_model.goal(successor->state)) LPT_INFO("cout", "Simulation - Goal state reached during simulation");

                if (process_node(successor)) {  // i.e. all subgoals have been reached before reaching the bound
                    report("All subgoals reached", max_width);
//                    _evaluator->info();
                    return true;
                }

                if (successor->_w <= max_width) {
                    open.insert(successor);
                }

                if (_generated % 1000 == 0) {
                    auto rate = _generated*1.0 / (aptk::time_used() - simt0);
                    LPT_INFO("cout", "IW run: Node generation rate after " << _generated / 1000 << "K generations (nodes/sec.): " << rate);
//                    _evaluator->info();
                }
            }
        }

        report("State space exhausted", max_width);
        _evaluator->info();
        return false;
    }


	void update_novelty_counters_on_expansion(unsigned char novelty) {
		if (novelty == 1) ++_w1_nodes_expanded;
		else if (novelty== 2) ++_w2_nodes_expanded;
	}

    void update_novelty_counters_on_generation(unsigned char novelty) {
        if (novelty==1) ++_w1_nodes_generated;
        else if (novelty==2)  ++_w2_nodes_generated;
        else ++_w_gt2_nodes_generated;
    }

    void report(const std::string& result, unsigned max_width) const {
        if (!_verbose) return;
        float perc_reached_subgoals = float(_model.num_subgoals() - _unreached.size()) / _model.num_subgoals();
        auto reachedv = _evaluator->reached_atoms();
        const auto& atom_idx = Problem::getInstance().get_tuple_index();
        unsigned reached = std::accumulate(reachedv.begin(), reachedv.end(), 0);
        unsigned total_atoms = atom_idx.size();
        float perc_reached_atoms = reached / (float) total_atoms;

        LPT_INFO("cout", "Simulation - Finished IW(" << max_width << ") Simulation. Fraction reached subgoals: " << std::fixed << std::setprecision(2) << perc_reached_subgoals);
        LPT_INFO("cout", "Simulation - Reached atoms: " << reached << "/" << total_atoms << " (" << perc_reached_atoms << ")");
        LPT_INFO("cout", "Simulation - " << result);
        LPT_INFO("cout", "Simulation - Reached " << (_model.num_subgoals() - _unreached.size()) << " / " << _model.num_subgoals() << " subgoals");
        LPT_INFO("cout", "Simulation - Expanded nodes with w=1 " << _w1_nodes_expanded);
        LPT_INFO("cout", "Simulation - Expanded nodes with w=2 " << _w2_nodes_expanded);
        LPT_INFO("cout", "Simulation - Generated nodes with w=1 " << _w1_nodes_generated);
        LPT_INFO("cout", "Simulation - Generated nodes with w=2 " << _w2_nodes_generated);
        LPT_INFO("cout", "Simulation - Generated nodes with w>2 " << _w_gt2_nodes_generated);
    }

protected:

    //! Returns true iff all goal atoms have been reached in the IW search
    bool process_node(NodePT& node) {
        const StateT& state = node->state;

        // We iterate through the indexes of all those goal atoms that have not yet been reached in the IW search
        // to check if the current node satisfies any of them - and if it does, we mark it appropriately.
        for (auto it = _unreached.begin(); it != _unreached.end(); ) {
            unsigned subgoal_idx = *it;

            if (_model.goal(state, subgoal_idx)) {
// 				node->satisfies_subgoal = true;
// 				_all_paths[subgoal_idx].push_back(node);
                if (!_optimal_paths[subgoal_idx]) _optimal_paths[subgoal_idx] = node;
                it = _unreached.erase(it);
            } else {
                ++it;
            }
        }
        // As soon as all nodes have been processed, we return true so that we can stop the search
        return _unreached.empty();
    }

    void mark_seed_subgoals(const NodePT& node) {
        std::vector<bool> _(_model.num_subgoals(), false);
        _in_seed.swap(_);
        _unreached.clear();
        for (unsigned i = 0; i < _model.num_subgoals(); ++i) {
            if (_model.goal(node->state, i)) {
                _in_seed[i] = true;
            } else {
                _unreached.insert(i);
            }
        }
    }

// public:
// 	const std::unordered_set<NodePT>& get_relevant_nodes() const { return _visited; }
};

} // namespaces
