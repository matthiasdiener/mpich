/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#include "gmpi.h"


void gmpi_debug_reg_cache_print0(char *msg)
{
  fprintf(stderr, "[%d]: %s\n", MPID_MyWorldRank, msg);
}


void gmpi_debug_reg_cache_print1(char *msg, char *label1, unsigned long value1)
{
  fprintf(stderr, "[%d]: %s, %s=0x%lx\n", MPID_MyWorldRank,
	  msg, label1, value1);
}


void gmpi_debug_reg_cache_print2(char *msg, char *label1, unsigned long value1,
				 char *label2, unsigned long value2)
{
  fprintf(stderr, "[%d]: %s, %s=0x%lx, %s=0x%lx\n", MPID_MyWorldRank,
	  msg, label1, value1, label2, value2);
}

