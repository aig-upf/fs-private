#pragma once

#include <fs/core/fs_types.hxx>

#include <cstdio>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/writer.h>

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
