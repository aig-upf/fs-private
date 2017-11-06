

// Looks like "DEBUG" is something inside soplex..
// TODO Check this.

#ifdef DEBUG
#undef DEBUG
#include <soplex.h>
#define DEBUG
#else
#include <soplex.h>
#endif // DEBUG
