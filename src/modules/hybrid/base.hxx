
#pragma once

#define FS_ABORT(msg) \
( \
    (std::cerr << "Critical error in file " << __FILE__ \
               << ", line " << __LINE__ << ": " << std::endl \
               << (msg) << std::endl), \
    (abort()), \
    (void)0 \
)


//!
#ifdef FS_USE_SOPLEX

    #define FS_LP_METHOD(X) X;
    #define FS_LP_ATTRIB(X) X;

#else

    #define FS_LP_METHOD(X) [[ noreturn ]] X { \
            FS_ABORT("You need to compile the planner with LP support in order to use this method."); \
    }
    #define FS_LP_ATTRIB(X)

#endif



