/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#ifdef WIN32
/* Windows doesn't use the aux funtions. */
int foo;
#else
#include <unistd.h>
#include <unistd.h>
#include <sys/mman.h>

#include "gmpi.h"

#if !defined(__linux__) && !defined(__APPLE__)
/* loic: at least on 64bits archs, it is important that sbrk
   has the right prototype sbrk altough quite UNIX universal
   is a non-official function, so might not be in headers
   if this definition conflicts with yours, remove the OS
   from the def condition
 */
void *sbrk();
#endif


void *gmpi_sbrk(int inc)
{
  if (inc < 0)
    {
      long oldp = (long)sbrk(0);
      
      GMPI_DEBUG_REG_CACHE_PRINT2("gmpi_sbrk", "oldp", oldp, "inc", inc);
      GMPI_DEBUG_REGISTRATION_CHECK_ALIGN("gmpi_sbrk", 
					  (unsigned long)(oldp+inc), (-inc));
      gmpi_clear_interval((unsigned long)(oldp+inc), -inc);
    }
  return sbrk(inc);
}


int gmpi_munmap(void *start, size_t length)
{
  GMPI_DEBUG_REG_CACHE_PRINT2("gmpi_munmap", "start", start, "length", length);
  GMPI_DEBUG_REGISTRATION_CHECK_ALIGN("gmpi_munmap", (unsigned long)(start),
				      length);
  gmpi_clear_interval((unsigned long)start, length);
  return munmap(start, length);
}
#endif
