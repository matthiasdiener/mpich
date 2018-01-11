/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#ifndef _gmpi_debug_regcache_h
#define _gmpi_debug_regcache_h


#if GMPI_DEBUG_REG_CACHE
#define GMPI_DEBUG_REG_CACHE_PRINT0(msg) \
gmpi_debug_reg_cache_print0(msg)
#define GMPI_DEBUG_REG_CACHE_PRINT1(msg,label1,value1) \
gmpi_debug_reg_cache_print1(msg,label1,value1)
#define GMPI_DEBUG_REG_CACHE_PRINT2(msg,label1,value1,label2,value2) \
gmpi_debug_reg_cache_print2(msg,label1,value1,label2,value2)
#else
#define GMPI_DEBUG_REG_CACHE_PRINT0(msg)
#define GMPI_DEBUG_REG_CACHE_PRINT1(msg,label1,value1)
#define GMPI_DEBUG_REG_CACHE_PRINT2(msg,label1,value1,label2,value2)
#endif

#endif
