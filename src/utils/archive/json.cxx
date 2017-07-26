#include <utils/archive/json.hxx>
#include <state.hxx>
#include <problem_info.hxx>

using namespace rapidjson;

namespace fs0 {

    void
    JSONArchive::store( Value& object, Document::AllocatorType& allocator, const State& s ) {
        const ProblemInfo& info = ProblemInfo::getInstance();
        for ( VariableIdx x = 0; x < info.getNumVariables(); x++ ) {
            Value name;
            name.SetString( StringRef( info.getVariableName(x).c_str() ));
            Value value;
            object_id o = s.getValue(x);
            if ( o_type(o) == type_id::bool_t )
                value = Value( fs0::value<bool>(o) );
            else if ( o_type(o) == type_id::int_t )
                value = Value( fs0::value<int>(o) );
            else if ( o_type(o) == type_id::float_t )
                value = Value( fs0::value<float>(o) );
            else {
                std::string _literal = info.object_name(o);
                value.SetString( _literal.c_str(), _literal.size(), allocator ) ;
            }
            object.AddMember( name, value.Move(), allocator );
        }
    }

}
