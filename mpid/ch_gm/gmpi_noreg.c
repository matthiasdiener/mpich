/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#include "gmpi.h"


static gmpi_bounce_buffer *bounce_free = NULL;
static gmpi_bounce_buffer *bounce_used = NULL;
static gmpi_bounce_buffer bounce_buffers[GMPI_BOUNCE_BUFFERS];

void 
gmpi_bounce_buffer_init(void)
{
  gmpi_bounce_buffer *previous_buffer = NULL;
  unsigned int i;

  gmpi_debug_assert(GM_PAGE_LEN != 0);
  if ((bounce_free == NULL) && (bounce_used == NULL))
    {
      gm_bzero(bounce_buffers, 
	       GMPI_BOUNCE_BUFFERS * sizeof(gmpi_bounce_buffer));
      
      for (i=0; i<GMPI_BOUNCE_BUFFERS; i++)
	{
	  bounce_buffers[i].next = previous_buffer;
	  bounce_buffers[i].addr = 
	    (unsigned long) gmpi_dma_alloc(GMPI_MAX_PUT_LENGTH);
	  gmpi_malloc_assert((void *)(bounce_buffers[i].addr),
			     "gmpi_bounce_buffer_init",
			     "gmpi_dma_alloc: bounce buffer");
	  GMPI_DEBUG_DMA_MEMORY_ACQUIRE(GMPI_MAX_PUT_LENGTH);
	  GMPI_DEBUG_DMA_MEMORY_USE(GMPI_MAX_PUT_LENGTH);
	  previous_buffer = &(bounce_buffers[i]);
	}
      
      bounce_free = &(bounce_buffers[GMPI_BOUNCE_BUFFERS-1]);
    }
}


void
gmpi_bounce_buffer_finish(void)
{
  unsigned int i;
  
  if ((bounce_free != NULL) || (bounce_used != NULL))
    {
      for (i=0; i<GMPI_BOUNCE_BUFFERS; i++)
	{
	  gmpi_dma_free((void *)(bounce_buffers[i].addr));
	  GMPI_DEBUG_DMA_MEMORY_RELEASE(GMPI_MAX_PUT_LENGTH);
	  GMPI_DEBUG_DMA_MEMORY_UNUSE(GMPI_MAX_PUT_LENGTH);
	}
      gm_bzero(bounce_buffers, 
	       GMPI_BOUNCE_BUFFERS * sizeof(gmpi_bounce_buffer));
      bounce_free = NULL;
      bounce_used = NULL;
    }
}


unsigned long 
gmpi_allocate_bounce_buffer(unsigned long data_addr,
			    unsigned int length)
{
  gmpi_bounce_buffer * bounce_buffer;
  
  GMPI_DEBUG_REG_CACHE_PRINT1("Allocate bounce buffer", "length", length);
  gmpi_debug_assert(bounce_buffers[0].addr != 0);
  
  if (bounce_free != NULL)
    {
      gmpi_debug_assert(bounce_free->status == GMPI_BOUNCE_SEGMENT_FREE);
      bounce_buffer = bounce_free;
      bounce_buffer->data_addr = data_addr;
      bounce_buffer->length = length;
      bounce_buffer->status = GMPI_BOUNCE_SEGMENT_USED;
      bounce_free = bounce_buffer->next;
      bounce_buffer->next = bounce_used;
      bounce_used = bounce_buffer;
      
      GMPI_DEBUG_REG_CACHE_PRINT2("Allocate bounce buffer",
				  "data_addr", data_addr,
				  "bounce_addr", bounce_buffer->addr); 
      return (bounce_buffer->addr);
    }
  else
    {	
      GMPI_DEBUG_REG_CACHE_PRINT0("Allocate bounce buffer: no luck");
      return 0;
    }

}


void gmpi_free_bounce_buffer(unsigned long data_addr,
			     unsigned int length)
{
  gmpi_bounce_buffer * bounce_buffer;
  gmpi_bounce_buffer * previous_bounce_buffer;
  
  GMPI_DEBUG_REG_CACHE_PRINT2("Free bounce buffer", "data_addr",
			      data_addr, "length", length);
  
  previous_bounce_buffer = NULL;
  bounce_buffer = bounce_used;
  while (bounce_buffer != NULL)
    {
      if (bounce_buffer->data_addr == data_addr)
	{
	  gmpi_debug_assert(bounce_buffer->length == length);
	  gmpi_debug_assert(bounce_buffer->status == GMPI_BOUNCE_SEGMENT_USED);
	  bounce_buffer->status = GMPI_BOUNCE_SEGMENT_FREE;
	  if (previous_bounce_buffer != NULL)
	    {
	      previous_bounce_buffer->next = bounce_buffer->next;
	    }
	  else
	    {
	      bounce_used = bounce_buffer->next;
	    }
	  bounce_buffer->next = bounce_free;
	  bounce_free = bounce_buffer;
	  return;
	}
      else
	{
	  previous_bounce_buffer = bounce_buffer;
	  bounce_buffer = bounce_buffer->next;
	}
    }
  gmpi_debug_assert(0);
}
