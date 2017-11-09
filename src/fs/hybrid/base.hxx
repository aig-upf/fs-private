
#pragma once

#include <fs/core/fs_types.hxx>

//!
#ifdef FS_HYBRID

    #define FS_LP_METHOD(X) X;
    #define FS_LP_ATTRIB(X) X;

#else

    #define FS_LP_METHOD(X) [[ noreturn ]] X { \
            FS_ABORT("You need to compile the planner with LP support in order to use this method."); \
    }
    #define FS_LP_ATTRIB(X)

#endif



