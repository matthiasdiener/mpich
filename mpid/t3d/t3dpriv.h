/* #if 0 */
#ifdef   DEBUG_PRINT_MSG
#  undef DEBUG_PRINT_MSG
#endif
#define DEBUG_PRINT_MSG(x)    printf("[%d] %s\n",MPID_MyWorldRank,(x)); fflush(stdout)
/* #endif */

#ifndef T3D_Printf
#  define T3D_Printf  printf
#endif
#ifndef T3D_Error
#  define T3D_Error
#endif
