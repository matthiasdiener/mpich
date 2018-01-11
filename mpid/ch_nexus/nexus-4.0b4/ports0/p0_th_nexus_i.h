/*
 * p0_th_nexus_i.h
 *
 * This ports0 thread module just redirects all thread calls
 * to the equivalent Nexus routines.
 * This allows a native Nexus thread library to be used with the
 * rest of ports0.
 *
 * rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_th_nexus_i.h,v 1.1 1995/03/30 17:59:39 tuecke Exp $";
 */


#define _p0_thread_self(Thread)  0
#define _p0_thread_id(Thread_ID) *(Thread_ID) = 0
