#pragma once

#include <fs_types.hxx>

#include <cstdio>
#include <lib/rapidjson/rapidjson.h>
#include <lib/rapidjson/document.h>
#include <lib/rapidjson/filewritestream.h>
#include <lib/rapidjson/filereadstream.h>
#include <lib/rapidjson/writer.h>

namespace fs0 {

    class State;

    class JSONArchive {
    public:
        static void
        store( rapidjson::Value& parent, rapidjson::Document::AllocatorType& allocator, const State& s );
        static void
        write_to_file( std::string filename, const rapidjson::Document& doc );

    protected:
    };

}
