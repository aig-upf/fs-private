/**
 * Include the hybrid module conditionally upon the definition of the appropriate pre-compiler symbol.
 */

#ifdef FS_USE_SOPLEX


#include <modules/hybrid/lp.hxx>
#include <modules/hybrid/heuristics/l2_norm.hxx>


#endif // FS_USE_SOPLEX
