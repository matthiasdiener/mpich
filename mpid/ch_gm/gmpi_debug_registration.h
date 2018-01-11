/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#ifndef _gmpi_debug_registration_h
#define _gmpi_debug_registration_h


typedef struct _reg_segment
{
  struct _reg_segment * prev;
  struct _reg_segment * next;
  unsigned long start;
  unsigned long length;
} reg_segment;



#if GMPI_DEBUG_REGISTRATION


void gmpi_debug_registration_use_segment(unsigned long start,
					 unsigned long length);
void gmpi_debug_registration_unuse_segment(unsigned long start,
					   unsigned long length);
void gmpi_debug_registration_check_segment(unsigned long start,
					   unsigned long length,
					   char * context);
void gmpi_debug_registration_clear_segment(unsigned long start,
					   unsigned long length);
void gmpi_debug_registration_clear_all_segments(void);
void gmpi_debug_registration_check_align(char * msg, unsigned long start,
					 unsigned long length);

     
#define GMPI_DEBUG_REGISTRATION_USE_SEGMENT(start,length)       \
gmpi_debug_registration_use_segment(start,length)
#define GMPI_DEBUG_REGISTRATION_UNUSE_SEGMENT(start,length)     \
gmpi_debug_registration_unuse_segment(start,length)
#define GMPI_DEBUG_REGISTRATION_CHECK_SEGMENT(start,length,msg) \
gmpi_debug_registration_check_segment(start,length,msg)
#define GMPI_DEBUG_REGISTRATION_CLEAR_SEGMENT(start,length)     \
gmpi_debug_registration_clear_segment(start,length)
#define GMPI_DEBUG_REGISTRATION_CLEAR_ALL_SEGMENTS()            \
gmpi_debug_registration_clear_all_segments()
#define GMPI_DEBUG_REGISTRATION_CHECK_ALIGN(msg,ptr,len)        \
gmpi_debug_registration_check_align(msg,ptr,len)
#else
#define GMPI_DEBUG_REGISTRATION_USE_SEGMENT(start,length)
#define GMPI_DEBUG_REGISTRATION_UNUSE_SEGMENT(start,length)
#define GMPI_DEBUG_REGISTRATION_CHECK_SEGMENT(start,length,msg)
#define GMPI_DEBUG_REGISTRATION_CLEAR_SEGMENT(start,length)
#define GMPI_DEBUG_REGISTRATION_CLEAR_ALL_SEGMENTS()
#define GMPI_DEBUG_REGISTRATION_CHECK_ALIGN(msg,ptr,len)
#endif

#endif
