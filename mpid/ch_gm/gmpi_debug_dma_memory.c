/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#include <unistd.h>
#include "gmpi.h"

unsigned long mem_locked_total = 0;
unsigned long mem_locked_in_use = 0;


void gmpi_debug_dma_memory_alloc(void *ptr, unsigned long length)
{
  printf("[%d]: gm_dma_malloc ptr=0x%lx len=%ld\n",
	 MPID_MyWorldRank, (unsigned long)ptr, length);
}


void gmpi_debug_dma_memory_free(void *ptr)
{
  printf("[%d]: gm_dma_free ptr=0x%lx",
	 MPID_MyWorldRank, (unsigned long)ptr);
}


void gmpi_debug_dma_memory_acquire(unsigned long size)
{
  mem_locked_total += size;
}


void gmpi_debug_dma_memory_release(unsigned long size)
{
  mem_locked_total -= size;
}


void gmpi_debug_dma_memory_use(unsigned long size)
{
  mem_locked_in_use += size;
}


void gmpi_debug_dma_memory_unuse(unsigned long size)
{
  mem_locked_in_use -= size;
}


void gmpi_debug_dma_memory_final(unsigned long static_mem_size)
{
  usleep((MPID_MyWorldRank*100000)+1000000);
  fprintf(stderr, "[%d]: DMA Memory usage: Total  = %ld Bytes, "
	  "In use = %ld Bytes\n",
	  MPID_MyWorldRank, mem_locked_total - static_mem_size,
	  mem_locked_in_use - static_mem_size);
}

