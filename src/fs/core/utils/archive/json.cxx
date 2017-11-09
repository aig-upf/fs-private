#include <fs/core/utils/archive/json.hxx>
#include <fs/core/state.hxx>
#include <fs/core/problem_info.hxx>

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

    void
    JSONArchive::write_to_file( std::string filename, const rapidjson::Document& doc ) {
        FILE* fp = fopen(filename.c_str(), "wb"); // non-Windows use "w"
        char writeBuffer[65536];
        FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
        Writer<FileWriteStream> writer(os);
        doc.Accept(writer);
        fclose(fp);
    }

}
