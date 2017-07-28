#include <heuristics/novelty/elliptical_2d.hxx>

#include <languages/fstrips/language.hxx>
#include <languages/fstrips/scopes.hxx>
#include <languages/fstrips/operations/basic.hxx>

#include <constraints/soplex/lp.hxx>

#include <problem_info.hxx>
#include <problem.hxx>
#include <heuristics/l2_norm.hxx>
#include <lapkt/tools/logging.hxx>
#include <utils/archive/json.hxx>
#include <memory>

namespace fs0 {

    EllipticalMapping2D::EllipticalMapping2D(   const std::vector<VariableIdx>& S,
                                                std::shared_ptr<State> x0,
                                                std::shared_ptr<State> xG,
                                                const std::vector<float>& bands )
        : Feature( S, type_id::int_t ), _start(x0), _goal(xG), _bands(bands) {

    }

    EllipticalMapping2D::~EllipticalMapping2D() {

    }

    FSFeatureValueT
    EllipticalMapping2D::evaluate( const State& s ) const {
    	//return fs0::raw_value<FSFeatureValueT>( make_object( hybrid::L2Norm::measure(scope(), start(), s)
        //                                                        + hybrid::L2Norm::measure(scope(), s, goal()) ));

        //return fs0::raw_value<FSFeatureValueT>( make_object( hybrid::L2Norm::measure(scope(), s, goal()) ));

        return fs0::raw_value<FSFeatureValueT>( make_object( (int)hybrid::L2Norm::geodesic_index( scope(), s, goal(), _bands)) );
    }

    void
    EllipticalMapping2D::make_goal_relative_features( std::vector< lapkt::novelty::NoveltyFeature<State>* >& features ) {
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

        LPT_INFO("features", "Constructing Elliptical Mapping features: ");
        const ProblemInfo& info = ProblemInfo::getInstance();


        std::vector<VariableIdx> tmp( goal_relevant_vars.begin(), goal_relevant_vars.end());
        for ( unsigned i = 0; i < tmp.size(); i++ ) {
            for ( unsigned j  = i+1; j < tmp.size(); j ++ ) {
                VariableIdx x0 = tmp[i];
                VariableIdx x1 = tmp[j];
                LPT_INFO( "features", "\t Variables: " << info.getVariableName(x0) << ", " << info.getVariableName(x1));
                std::vector<VariableIdx> input = { x0, x1};
                float c = hybrid::L2Norm::measure(input, *s0, *sG );
                LPT_INFO( "features", "\t\t c(s0,sG) = " << c );
                std::vector<float> bands;
                unsigned num_levels = (unsigned)10.0*std::max(std::log10(c),1.0f);
                LPT_INFO( "features", "\t\t levels = " << num_levels );
                float delta_c = c / (float) num_levels;
                float c_k = c;
                while (c_k > delta_c) {
                    bands.push_back(c_k);
                    c_k -= delta_c;
                }

                features.push_back( new EllipticalMapping2D( input, s0, sG, bands ));
            }
        }
    }

    void
    EllipticalMapping2D::print_to_JSON( std::string filename, const std::vector< lapkt::novelty::NoveltyFeature<State>* >& features ) {
        using namespace rapidjson;

        const ProblemInfo& info = ProblemInfo::getInstance();
        Document feature_data;
        Document::AllocatorType& allocator = feature_data.GetAllocator();
        feature_data.SetObject();
        Value domainName;
        domainName.SetString(StringRef(info.getDomainName().c_str()));
        feature_data.AddMember("domain", domainName.Move(), allocator );
        Value instanceName;
        instanceName.SetString(StringRef(info.getInstanceName().c_str()));
        feature_data.AddMember("instance", instanceName.Move(), allocator );

		Value feature_set(kArrayType);
        {
			for ( unsigned k = 0; k < features.size(); k++ ) {
                const EllipticalMapping2D* f_k = dynamic_cast<const EllipticalMapping2D*>(features[k]);
                if ( f_k == nullptr ) continue;
				Value entry(kObjectType); {
					Value tuple(kArrayType);{
						for ( unsigned l = 0; l < f_k->scope().size(); l++ ) {
							Value name;
	                        name.SetString( StringRef( info.getVariableName(f_k->scope()[l]).c_str() ));
							tuple.PushBack(name.Move(),allocator);
						}
					}
					entry.AddMember("scope", tuple.Move(),allocator);

                    Value breakpoints(kArrayType); {
                        for ( unsigned l = 0; l < f_k->_bands.size(); l++ ) {
							Value b(f_k->_bands[l]);
							tuple.PushBack(b.Move(),allocator);
						}
                    }
                    entry.PushBack(breakpoints.Move(),allocator);
				}
				feature_set.PushBack(entry.Move(), allocator);
			}
        }
        feature_data.AddMember("features", feature_set, allocator);

		FILE* fp = fopen(filename.c_str(), "wb"); // non-Windows use "w"
        char writeBuffer[65536];
        FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
        Writer<FileWriteStream> writer(os);
        feature_data.Accept(writer);
        fclose(fp);

    }

    std::ostream&
    EllipticalMapping2D::print(std::ostream& os) const {
        return os;
    }

}
