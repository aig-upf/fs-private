#include <heuristics/g0.hxx>

#include <languages/fstrips/language.hxx>
#include <languages/fstrips/scopes.hxx>
#include <languages/fstrips/operations/basic.hxx>

#include <constraints/soplex/lp.hxx>

#include <problem_info.hxx>
#include <problem.hxx>
#include <heuristics/l2_norm.hxx>
#include <lapkt/tools/logging.hxx>

#include <utils/archive/json.hxx>

namespace fs0 { namespace hybrid {

    G0Heuristic::G0Heuristic( std::shared_ptr<State> sG) : _sG(sG) {}

    G0Heuristic::G0Heuristic( G0Heuristic&& o )
        : _scopes( std::move(o._scopes) ), _breakpoints(std::move(o._breakpoints)){
        _sG = o._sG;
        o._sG = nullptr;
    }


    unsigned
    G0Heuristic::evaluate( const State& s ) const {
        unsigned h = 0;
        for ( unsigned k = 0; k < _scopes.size(); k++ ) {
            h += L2Norm::geodesic_index( _scopes[k], s, *_sG, _breakpoints[k] );
        }
        return h;
    }

    std::shared_ptr<G0Heuristic>
    G0Heuristic::create_relative_to_goal() {
        spx::LinearProgram   poly;
        const Problem& problem = Problem::getInstance();
        std::set<VariableIdx> goal_relevant_vars;
        for ( auto formula : fs::all_relations( *problem.getGoalConditions())) {
            poly.add_constraint(formula);
            std::set<VariableIdx> condS;
            fs::ScopeUtils::computeFullScope(formula, condS);
            for ( auto x : condS ) {
                goal_relevant_vars.insert(x);
            }
		}

        poly.setup();
        poly.solve();

        std::shared_ptr<State> s0 = std::make_shared<State>(problem.getInitialState());
        std::shared_ptr<State> sG = std::make_shared<State>(problem.getInitialState());
        sG->accumulate(poly.solution());

        LPT_INFO("heuristic", "Constructing G0 Heuristic: ");
        const ProblemInfo& info = ProblemInfo::getInstance();
        std::shared_ptr<G0Heuristic> the_h = std::make_shared<G0Heuristic>(sG);

        std::vector<VariableIdx> tmp( goal_relevant_vars.begin(), goal_relevant_vars.end());
        for ( unsigned i = 0; i < tmp.size(); i++ ) {
            for ( unsigned j  = i+1; j < tmp.size(); j ++ ) {
                VariableIdx x0 = tmp[i];
                VariableIdx x1 = tmp[j];
                LPT_INFO( "heuristic", "\t Variables: " << info.getVariableName(x0) << ", " << info.getVariableName(x1));
                std::vector<VariableIdx> input = { x0, x1};
                float c = L2Norm::measure(input, *s0, *sG );
                LPT_INFO( "heuristic", "\t\t c(s0,sG) = " << c );
                std::vector<float> bands;
                unsigned num_levels = 10;
                float delta_c = c / (float) num_levels;
                float c_k = c;
                while (c_k > delta_c) {
                    bands.push_back(c_k);
                    c_k -= delta_c;
                }
                the_h->_scopes.push_back( input );
                the_h->_breakpoints.push_back(bands);
            }
        }
        return the_h;
    }

    void
    G0Heuristic::print_JSON( std::string filename ) const {
        using namespace rapidjson;

        const ProblemInfo& info = ProblemInfo::getInstance();
        Document G0_details;
        Document::AllocatorType& allocator = G0_details.GetAllocator();
        G0_details.SetObject();
        Value domainName;
        domainName.SetString(StringRef(info.getDomainName().c_str()));
        G0_details.AddMember("domain", domainName.Move(), allocator );
        Value instanceName;
        instanceName.SetString(StringRef(info.getInstanceName().c_str()));
        G0_details.AddMember("instance", instanceName.Move(), allocator );

		Value elements(kArrayType);
        {
			for ( unsigned k = 0; k < _scopes.size(); k++ ) {
				Value entry(kArrayType); {
					Value tuple(kArrayType);{
						for ( unsigned l = 0; l < _scopes[k].size(); l++ ) {
							Value name;
	                        name.SetString( StringRef( info.getVariableName(_scopes[k][l]).c_str() ));
							tuple.PushBack(name.Move(),allocator);
						}
					}
					entry.PushBack(tuple.Move(),allocator);

                    Value breakpoints(kArrayType); {
                        for ( unsigned l = 0; l < _breakpoints[k].size(); l++ ) {
							Value b(_breakpoints[k][l]);
							tuple.PushBack(b.Move(),allocator);
						}
                    }
                    entry.PushBack(breakpoints.Move(),allocator);
				}
				elements.PushBack(entry.Move(), allocator);
			}
        }
        G0_details.AddMember("elements", elements, allocator);

		FILE* fp = fopen(filename.c_str(), "wb"); // non-Windows use "w"
        char writeBuffer[65536];
        FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
        Writer<FileWriteStream> writer(os);
        G0_details.Accept(writer);
        fclose(fp);

    }

} }
