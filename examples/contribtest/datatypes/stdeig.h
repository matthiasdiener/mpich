/* 
   COPYRIGHT U.S. GOVERNMENT 
   
   This software is distributed without charge and comes with
   no warranty.

   Please feel free to send questions, comments, and problem reports
   to prism@super.org. 
*/

/* PURPOSE
   =======
   This is the stdeig.h header file.  It defines things that are included
   in most PRISM routines.  It is generally machine specific stuff.
*/

/* avoid multiple inclusion of this header file */
#ifndef STDEIG_H  
#define STDEIG_H  

#if PRISM_DELTA
#define Prism_STR_MACHINE "DELTA"
#if PRISM_NX
#include <mesh.h>
#endif
#endif

#if PRISM_PARAGON
#define Prism_STR_MACHINE "PARAGON"
#if PRISM_NX
#include <nx.h>
extern long mypart(long *, long *);
#endif
#endif

#if PRISM_SP
#define Prism_STR_MACHINE "SP"
#endif

#if PRISM_SUN
#define Prism_STR_MACHINE "SUN"
#endif

#if PRISM_MEIKO
#define Prism_STR_MACHINE "MEIKO"
#endif

#if PRISM_CM5
#define Prism_STR_MACHINE "CM5"
#endif

#if PRISM_AP1000
#define Prism_STR_MACHINE "AP1000"
#endif

#if PRISM_INDY
#define Prism_STR_MACHINE "INDY"
#endif

#ifndef Prism_STR_MACHINE
#define Prism_STR_MACHINE "UNKNOWN"
#endif

#if !PRISM_NX
#include "mpi.h"
#endif

#endif
