/*
 * Nexus-MPI Abstract Device/2 Implementation
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 * 
 * Authors: George K. Thiruvathukal and Jonathan Geisler
 *
 * Version: $Id: adi2pack.c,v 1.2 1997/04/01 19:36:18 thiruvat Exp $
 *
 * Nexus-MPI is an application being developed using the Globus Communications
 * Specification. For more information about the Globus project, its
 * objectives, and the status of Nexus-MPI (as well as the latest and 
 * greatest distributions), please check our Web Site at
 *    http://www.globus.org/
 */

#include <stdio.h>
#include "mpid.h"
#include "mpidmpi.h"
#include "nexuspriv.h"

#define void_pointer_displace(vp, displacement) \
   do { \
      vp = (char *)(vp) + (displacement); \
   } while (0);

void MPID_Msg_rep(struct MPIR_COMMUNICATOR *comm,
		  int partner,
		  struct MPIR_DATATYPE *datatype,
		  MPID_Msgrep_t *msgrep,
		  MPID_Msg_pack_t *msgact)
{
    *msgrep = remote_formats[comm->lrank_to_grank[partner]];
    *msgact = MPID_MSG_OK;
}

int MPID_Msgrep_from_comm(struct MPIR_COMMUNICATOR *comm)
{
    return NEXUS_DC_FORMAT_LOCAL;
}

void MPID_Msg_act(struct MPIR_COMMUNICATOR *comm,
		  int partner,
		  struct MPIR_DATATYPE *datatype,
		  MPID_Msgrep_t msgrep,
		  MPID_Msg_pack_t *msgact)
{
    if (msgrep != MPID_MSGREP_SENDER)
    {
	nexus_warning("WARNING - unrecognized msgrep %d\n", msgrep);
    }
    *msgact = MPID_MSG_OK;
}

void MPID_Pack_buffer_preprocess(int count,
				 struct MPIR_DATATYPE *datatype,
				 int *num_elements,
				 int *buf_size)
{
#ifdef USE_DIRECT
/* stuff */
#else
*num_elements = 0;
#endif
}

void MPID_Pack_exact_size(int count,
			  struct MPIR_DATATYPE *datatype,
			  int remote_format,
			  int *pack_size,
			  int *remote_size)
{
    int tmp_pack, tmp_remote;
    int i;

    switch(datatype->dte_type)
    {
      case MPIR_CHAR:
	*pack_size += nexus_sizeof_char(count);
	*remote_size += nexus_dc_sizeof_remote_char(count, remote_format);
	break;
      case MPIR_UCHAR:
	*pack_size += nexus_sizeof_u_char(count);
	*remote_size += nexus_dc_sizeof_remote_u_char(count, remote_format);
	break;
      case MPIR_PACKED:
      case MPIR_BYTE:
	*pack_size += nexus_sizeof_byte(count);
	*remote_size += nexus_dc_sizeof_remote_byte(count, remote_format);
	break;
      case MPIR_SHORT:
	*pack_size += nexus_sizeof_short(count);
	*remote_size += nexus_dc_sizeof_remote_short(count, remote_format);
	break;
      case MPIR_USHORT:
	*pack_size += nexus_sizeof_u_short(count);
	*remote_size += nexus_dc_sizeof_remote_u_short(count, remote_format);
	break;
      case MPIR_INT:
	*pack_size += nexus_sizeof_int(count);
	*remote_size += nexus_dc_sizeof_remote_int(count, remote_format);
	break;
      case MPIR_UINT:
	*pack_size += nexus_sizeof_u_int(count);
	*remote_size += nexus_dc_sizeof_remote_u_int(count, remote_format);
	break;
      case MPIR_LONG:
	*pack_size += nexus_sizeof_long(count);
	*remote_size += nexus_dc_sizeof_remote_long(count, remote_format);
	break;
      case MPIR_ULONG:
	*pack_size += nexus_sizeof_u_long(count);
	*remote_size += nexus_dc_sizeof_remote_u_long(count, remote_format);
	break;
      case MPIR_FLOAT:
	*pack_size += nexus_sizeof_float(count);
	*remote_size += nexus_dc_sizeof_remote_float(count, remote_format);
	break;
      case MPIR_COMPLEX:
	*pack_size += nexus_sizeof_float(2 * count);
	*remote_size += nexus_dc_sizeof_remote_float(2 * count, remote_format);
	break;
      case MPIR_DOUBLE:
	*pack_size += nexus_sizeof_double(count);
	*remote_size += nexus_dc_sizeof_remote_double(count, remote_format);
	break;
      case MPIR_DOUBLE_COMPLEX:
	*pack_size += nexus_sizeof_double(2 * count);
	*remote_size += nexus_dc_sizeof_remote_double(2 * count, remote_format);
	break;
      case MPIR_CONTIG:
        MPID_Pack_exact_size(count * datatype->count,
		       	     datatype->old_type,
			     remote_format,
			     pack_size,
			     remote_size);
	break;
      case MPIR_VECTOR:
      case MPIR_HVECTOR:
	tmp_pack = tmp_remote = 0;
        MPID_Pack_exact_size(datatype->blocklen,
    		       	     datatype->old_type,
			     remote_format,
			     &tmp_pack,
			     &tmp_remote);
        *pack_size += tmp_pack * count * datatype->count;
	*remote_size += tmp_remote * count * datatype->count;
        break;
      case MPIR_INDEXED:
      case MPIR_HINDEXED:
	tmp_pack = tmp_remote = 0;
        for (i = 0; i < datatype->count; i++)
        {
	    MPID_Pack_exact_size(datatype->blocklens[i],
		           	 datatype->old_type,
				 remote_format,
				 &tmp_pack,
				 &tmp_remote);
	}
	*pack_size += tmp_pack * count;
	*remote_size += tmp_remote * count;
        break;
      case MPIR_STRUCT:
	tmp_pack = tmp_remote = 0;
        for (i = 0; i < datatype->count; i++)
        {
	    MPID_Pack_exact_size(datatype->blocklens[i],
		           	 datatype->old_types[i],
				 remote_format,
				 &tmp_pack,
				 &tmp_remote);
	}
	*pack_size += tmp_pack * count;
	*remote_size += tmp_remote * count;
	break;
      /*
       * I think these are 0 sized things for some purpose.  I need to
       * see what I really should be doing here.
       */
      case MPIR_UB:
      case MPIR_LB:
	break;
      case MPIR_LONGDOUBLE:
      case MPIR_LOGICAL:
      default:
	nexus_warning("That type is not supported for packing\n");
        break;
    }
}

void MPID_Pack_size(int count,
		    struct MPIR_DATATYPE *datatype,
		    MPID_Msg_pack_t msgact,
		    int *size)
{
    int tmp;
    int i;

    switch(datatype->dte_type)
    {
      case MPIR_CHAR:
	*size += nexus_sizeof_char(count);
	break;
      case MPIR_UCHAR:
	*size += nexus_sizeof_u_char(count);
	break;
      case MPIR_PACKED:
      case MPIR_BYTE:
	*size += nexus_sizeof_byte(count);
	break;
      case MPIR_SHORT:
	*size += nexus_sizeof_short(count);
	break;
      case MPIR_USHORT:
	*size += nexus_sizeof_u_short(count);
	break;
      case MPIR_INT:
	*size += nexus_sizeof_int(count);
	break;
      case MPIR_UINT:
	*size += nexus_sizeof_u_int(count);
	break;
      case MPIR_LONG:
	*size += nexus_sizeof_long(count);
	break;
      case MPIR_ULONG:
	*size += nexus_sizeof_u_long(count);
	break;
      case MPIR_FLOAT:
	*size += nexus_sizeof_float(count);
	break;
      case MPIR_COMPLEX:
	*size += nexus_sizeof_float(2 * count);
	break;
      case MPIR_DOUBLE:
	*size += nexus_sizeof_double(count);
	break;
      case MPIR_DOUBLE_COMPLEX:
	*size += nexus_sizeof_double(2 * count);
	break;
      case MPIR_CONTIG:
        MPID_Pack_size(count * datatype->count,
		       datatype->old_type,
		       msgact,
		       size);
	break;
      case MPIR_VECTOR:
      case MPIR_HVECTOR:
        tmp = 0;
        MPID_Pack_size(datatype->blocklen,
    		       datatype->old_type,
		       msgact,
		       &tmp);
        *size += tmp * count * datatype->count;
        break;
      case MPIR_INDEXED:
      case MPIR_HINDEXED:
        tmp = 0;
        for (i = 0; i < datatype->count; i++)
        {
            int tmp2 = 0;

	    MPID_Pack_size(datatype->blocklens[i],
		           datatype->old_type,
			   msgact,
			   &tmp2);
            tmp += tmp2;
	}
	*size += tmp * count;
        break;
      case MPIR_STRUCT:
        tmp = 0;
        for (i = 0; i < datatype->count; i++)
        {
	    int tmp2 = 0;

	    MPID_Pack_size(datatype->blocklens[i],
		           datatype->old_types[i],
			   msgact,
			   &tmp2);
	    tmp += tmp2;
	}
	*size += tmp * count;
	break;
      /*
       * I think these are 0 sized things for some purpose.  I need to
       * see what I really should be doing here.
       */
      case MPIR_UB:
      case MPIR_LB:
	break;
      case MPIR_LONGDOUBLE:
      case MPIR_LOGICAL:
      default:
	nexus_warning("That type is not supported for packing\n");
        break;
    }
}

void MPID_Pack_buffer_elements(int count,
		   	       struct MPIR_DATATYPE *datatype,
		   	       nexus_buffer_t *send_buffer,
		   	       int *num_elements)
{
    int tmp;
    int i, j;

    switch(datatype->dte_type)
    {
      case MPIR_CHAR:
      case MPIR_UCHAR:
      case MPIR_BYTE:
      case MPIR_PACKED:
      case MPIR_SHORT:
      case MPIR_USHORT:
      case MPIR_INT:
      case MPIR_UINT:
      case MPIR_LONG:
      case MPIR_ULONG:
      case MPIR_FLOAT:
      case MPIR_COMPLEX:
      case MPIR_DOUBLE:
      case MPIR_DOUBLE_COMPLEX:
	(*num_elements)++;
      case MPIR_CONTIG:
        MPID_Pack_buffer_elements(count * datatype->count,
		           	  datatype->old_type,
			   	  send_buffer,
			   	  num_elements);
        break;
      case MPIR_VECTOR:
      case MPIR_HVECTOR:
        tmp = 0;
        for (i = 0; i < datatype->count; i++)
        {
	    int tmp2 = 0;
	    MPID_Pack_buffer_elements(datatype->blocklen,
			       	      datatype->old_type,
			       	      send_buffer,
			       	      &tmp2);
	    tmp += tmp2;
	}
	*num_elements += tmp * count;
	break;
      case MPIR_INDEXED:
      case MPIR_HINDEXED:
        tmp = 0;
        for (i = 0; i < datatype->count; i++)
        {
	    int tmp2 = 0;
	    MPID_Pack_buffer_elements(datatype->blocklens[i],
			       	      datatype->old_type,
			       	      send_buffer,
			       	      &tmp2);
	    tmp += tmp2;
	}
	*num_elements += tmp * count;
	break;
      case MPIR_STRUCT:
        tmp = 0;
        for (i = 0; i < datatype->count; i++)
        {
	    int tmp2 = 0;
	    MPID_Pack_buffer_elements(datatype->blocklens[i],
			       	  datatype->old_types[i],
			       	  send_buffer,
			       	  &tmp2);
	    tmp += tmp2;
	}
	*num_elements += tmp * count;
	break;
      default:
        break;
    }
}

void MPID_Pack_buffer(void *src,
		      int count,
		      struct MPIR_DATATYPE *datatype,
		      nexus_buffer_t *send_buf)
{
    void *tmp_buf;
    int i, j;

    if (count == 0 || datatype->size == 0)
    {
	return ;
    }

    switch (datatype->dte_type)
    {
      /* 
       * These are basic datatypes that Nexus directly knows how to
       * deal with.  Just put them into the send buffer and return.
       */
      case MPIR_CHAR:
#ifdef USE_DIRECT
	nexus_direct_put_char(send_buf, (char *)src, count);
#else
	nexus_put_char(send_buf, (char *)src, count);
#endif
	break;
      case MPIR_UCHAR:
#ifdef USE_DIREcT
	nexus_direct_put_u_char(send_buf, (u_char *)src, count);
#else
	nexus_put_u_char(send_buf, (u_char *)src, count);
#endif
	break;
      case MPIR_PACKED:
#ifdef USE_DIRECT
	nexus_direct_put_user(send_buf, (nexus_byte_t *)src, count);
#else
	nexus_put_user(send_buf, (nexus_byte_t *)src, count);
#endif
	break;
      case MPIR_BYTE:
#ifdef USE_DIRECT
	nexus_direct_put_byte(send_buf, (nexus_byte_t *)src, count);
#else
	nexus_put_byte(send_buf, (nexus_byte_t *)src, count);
#endif
	break;
      case MPIR_SHORT:
#ifdef USE_DIRECT
	nexus_direct_put_short(send_buf, (short *)src, count);
#else
	nexus_put_short(send_buf, (short *)src, count);
#endif
	break;
      case MPIR_USHORT:
#ifdef USE_DIRECT
	nexus_direct_put_u_short(send_buf, (u_short *)src, count);
#else
	nexus_put_u_short(send_buf, (u_short *)src, count);
#endif
	break;
      case MPIR_INT:
#ifdef USE_DIRECT
	nexus_direct_put_int(send_buf, (int *)src, count);
#else
	nexus_put_int(send_buf, (int *)src, count);
#endif
	break;
      case MPIR_UINT:
#ifdef USE_DIRECT
	nexus_direct_put_u_int(send_buf, (u_int *)src, count);
#else
	nexus_put_u_int(send_buf, (u_int *)src, count);
#endif
	break;
      case MPIR_LONG:
#ifdef USE_DIRECT
	nexus_direct_put_long(send_buf, (long *)src, count);
#else
	nexus_put_long(send_buf, (long *)src, count);
#endif
	break;
      case MPIR_ULONG:
#ifdef USE_DIRECT
	nexus_direct_put_u_long(send_buf, (u_long *)src, count);
#else
	nexus_put_u_long(send_buf, (u_long *)src, count);
#endif
	break;
      case MPIR_FLOAT:
#ifdef USE_DIRECT
	nexus_direct_put_float(send_buf, (float *)src, count);
#else
	nexus_put_float(send_buf, (float *)src, count);
#endif
	break;
      case MPIR_COMPLEX:
#ifdef USE_DIRECT
	nexus_direct_put_float(send_buf, (float *)src, 2 * count);
#else
	nexus_put_float(send_buf, (float *)src, 2 * count);
#endif
	break;
      case MPIR_DOUBLE:
#ifdef USE_DIRECT
	nexus_direct_put_double(send_buf, (double *)src, count);
#else
	nexus_put_double(send_buf, (double *)src, count);
#endif
	break;
      case MPIR_DOUBLE_COMPLEX:
#ifdef USE_DIRECT
	nexus_direct_put_double(send_buf, (double *)src, 2 * count);
#else
	nexus_put_double(send_buf, (double *)src, 2 * count);
#endif
	break;
      /* 
       * These are complex datatypes that Nexus does not know how to
       * deal with directly.  Break them down into their component parts
       * and put the more basic elements by recursively calling
       * MPI_Pack_buffer().  Note that is allows complex structures like
       * an MPIR_STRUCT inside an MPIR_VECTOR, etc.
       */
      case MPIR_CONTIG:
	MPID_Pack_buffer(src,
			 count * datatype->count,
			 datatype->old_type,
			 send_buf);
	break;
      case MPIR_VECTOR:
      case MPIR_HVECTOR:
	tmp_buf = src;
	for (i = 0; i < count; i++)
	{
	    src = tmp_buf;
	    for (j = 0; j < datatype->count; j++)
	    {
		MPID_Pack_buffer(src,
				 datatype->blocklen,
				 datatype->old_type,
				 send_buf);
		void_pointer_displace(src, datatype->stride)
	    }
	    void_pointer_displace(tmp_buf, datatype->extent);
	}
	break;
      case MPIR_INDEXED:
      case MPIR_HINDEXED:
	for (i = 0; i < count; i++)
	{
	    for (j = 0; j < datatype->count; j++)
	    {
		tmp_buf = (char *)src + datatype->indices[j];
		MPID_Pack_buffer(tmp_buf,
				 datatype->blocklens[j],
				 datatype->old_type,
				 send_buf);
	    }
            void_pointer_displace(src, datatype->extent);
	}
	break;
      case MPIR_STRUCT:
	for (i = 0; i < count; i++)
	{
	    for (j = 0; j < datatype->count; j++)
	    {
		tmp_buf = (char *)src + datatype->indices[j];
		MPID_Pack_buffer(tmp_buf,
				 datatype->blocklens[j],
				 datatype->old_types[j],
				 send_buf);
	    }
	    void_pointer_displace(src,datatype->extent);
	}
	break;
      /*
       * I think these are 0 sized things for some purpose.  I need to
       * see what I really should be doing here.
       */
      case MPIR_UB:
      case MPIR_LB:
	break;
      /* 
       * Nexus cannot deal with these types directly, and they cannot be
       * broken down into more simple parts.  Return an error to the
       * calling procedure to indicate the problem.
       */
      case MPIR_LOGICAL:
      case MPIR_LONGDOUBLE:
	/* *error_code = MPI_ERR_TYPE; */
	return ;
      /* 
       * This is not a legal type.  Return an error code.
       */
      default:
	/* *error_code = MPI_ERR_INTERN; */
	return ;
    }
}

void MPID_Unpack_buffer(void *dest,
			int count,
			struct MPIR_DATATYPE *datatype,
			nexus_buffer_t *recv_buf)
{
    void *tmp_buf;
    int i, j;

    switch (datatype->dte_type)
    {
      /* 
       * These are basic datatypes that Nexus directly knows how to
       * deal with.  Just put them into the send buffer and return.
       */
      case MPIR_CHAR:
#ifdef USE_DIRECT
	nexus_direct_get_char(recv_buf, (char *)dest, count);
#else
	nexus_get_char(recv_buf, (char *)dest, count);
#endif
	break;
      case MPIR_UCHAR:
#ifdef USE_DIRECT
	nexus_direct_get_u_char(recv_buf, (u_char *)dest, count);
#else
	nexus_get_u_char(recv_buf, (u_char *)dest, count);
#endif
	break;
      case MPIR_PACKED:
#ifdef USE_DIRECT
	nexus_direct_get_user(recv_buf, (nexus_byte_t *)dest, count);
#else
	nexus_get_user(recv_buf, (nexus_byte_t *)dest, count);
#endif
	break;
      case MPIR_BYTE:
#ifdef USE_DIRECT
	nexus_direct_get_byte(recv_buf, (nexus_byte_t *)dest, count);
#else
	nexus_get_byte(recv_buf, (nexus_byte_t *)dest, count);
#endif
	break;
      case MPIR_SHORT:
#ifdef USE_DIRECT
	nexus_direct_get_short(recv_buf, (short *)dest, count);
#else
	nexus_get_short(recv_buf, (short *)dest, count);
#endif
	break;
      case MPIR_USHORT:
#ifdef USE_DIRECT
	nexus_direct_get_u_short(recv_buf, (u_short *)dest, count);
#else
	nexus_get_u_short(recv_buf, (u_short *)dest, count);
#endif
	break;
      case MPIR_INT:
#ifdef USE_DIRECT
	nexus_direct_get_int(recv_buf, (int *)dest, count);
#else
	nexus_get_int(recv_buf, (int *)dest, count);
#endif
	break;
      case MPIR_UINT:
#ifdef USE_DIRECT
	nexus_direct_get_u_int(recv_buf, (u_int *)dest, count);
#else
	nexus_get_u_int(recv_buf, (u_int *)dest, count);
#endif
	break;
      case MPIR_LONG:
#ifdef USE_DIRECT
	nexus_direct_get_long(recv_buf, (long *)dest, count);
#else
	nexus_get_long(recv_buf, (long *)dest, count);
#endif
	break;
      case MPIR_ULONG:
#ifdef USE_DIRECT
	nexus_direct_get_u_long(recv_buf, (u_long *)dest, count);
#else
	nexus_get_u_long(recv_buf, (u_long *)dest, count);
#endif
	break;
      case MPIR_FLOAT:
#ifdef USE_DIRECT
	nexus_direct_get_float(recv_buf, (float *)dest, count);
#else
	nexus_get_float(recv_buf, (float *)dest, count);
#endif
	break;
      case MPIR_COMPLEX:
#ifdef USE_DIRECT
	nexus_direct_get_float(recv_buf, (float *)dest, 2 * count);
#else
	nexus_get_float(recv_buf, (float *)dest, 2 * count);
#endif
	break;
      case MPIR_DOUBLE:
#ifdef USE_DIRECT
	nexus_direct_get_double(recv_buf, (double *)dest, count);
#else
	nexus_get_double(recv_buf, (double *)dest, count);
#endif
	break;
      case MPIR_DOUBLE_COMPLEX:
#ifdef USE_DIRECT
	nexus_direct_get_double(recv_buf, (double *)dest, 2 * count);
#else
	nexus_get_double(recv_buf, (double *)dest, 2 * count);
#endif
	break;
      /* 
       * These are complex datatypes that Nexus does not know how to
       * deal with directly.  Break them down into their component parts
       * and put the more basic elements by recursively calling
       * MPI_Pack_buffer().  Note that is allows complex structures like
       * an MPIR_STRUCT inside an MPIR_VECTOR, etc.
       */
      case MPIR_CONTIG:
	MPID_Unpack_buffer(dest,
			   count * datatype->count,
			   datatype->old_type,
			   recv_buf);
	break;
      case MPIR_VECTOR:
      case MPIR_HVECTOR:
	tmp_buf = dest;
	for (i = 0; i < count; i++)
	{
	    dest = tmp_buf;
	    for (j = 0; j < datatype->count; j++)
	    {
		MPID_Unpack_buffer(dest,
				   datatype->blocklen,
				   datatype->old_type,
				   recv_buf);
		void_pointer_displace(dest, datatype->stride);
	    }
	    void_pointer_displace(tmp_buf, datatype->extent);
	}
	break;
      case MPIR_INDEXED:
      case MPIR_HINDEXED:
	for (i = 0; i < count; i++)
	{
	    for (j = 0; j < datatype->count; j++)
	    {
		tmp_buf = (char *)dest + datatype->indices[j];
		MPID_Unpack_buffer(tmp_buf,
				   datatype->blocklens[j],
				   datatype->old_type,
				   recv_buf);
	    }
	    void_pointer_displace(dest, datatype->extent);
	}
	break;
      case MPIR_STRUCT:
	for (i = 0; i < count; i++)
	{
	    for (j = 0; j < datatype->count; j++)
	    {
		tmp_buf = (char *)dest + datatype->indices[j];
		MPID_Unpack_buffer(tmp_buf,
				  datatype->blocklens[j],
				  datatype->old_types[j],
				  recv_buf);
	    }
	    void_pointer_displace(dest, datatype->extent);
	}
	break;
      /*
       * I think these are 0 sized things for some purpose.  I need to
       * see what I really should be doing here.
       */
      case MPIR_UB:
      case MPIR_LB:
	break;
      /* 
       * Nexus cannot deal with these types directly, and they cannot be
       * broken down into more simple parts.  Return an error to the
       * calling procedure to indicate the problem.
       */
      case MPIR_LOGICAL:
      case MPIR_LONGDOUBLE:
	/* *error_code = MPI_ERR_TYPE; */
	return ;
      /* 
       * This is not a legal type.  Return an error code.
       */
      default:
	/* *error_code = MPI_ERR_INTERN; */
	return ;
    }
}

void MPID_Pack(void *src,
	       int count,
	       struct MPIR_DATATYPE *datatype,
	       void *dest,
	       int maxcount,
	       int *position,
	       struct MPIR_COMMUNICATOR *comm,
	       int partner,
	       MPID_Msgrep_t msgrep,
	       MPID_Msg_pack_t msgact,
	       int *error_code)
{
    nexus_byte_t *dest_start, *dest_end;
    void *tmp_buf;
    int i, j;

    if (count == 0 || datatype->size == 0)
    {
	return ;
    }

    dest_start = dest_end = (nexus_byte_t *)dest + *position;

    switch (datatype->dte_type)
    {
      /* 
       * These are basic datatypes that Nexus directly knows how to
       * deal with.  Just put them into the send buffer and return.
       */
      case MPIR_CHAR:
	nexus_user_put_char(&dest_end, (char *)src, count);
	*position += dest_end - dest_start;
	break;
      case MPIR_UCHAR:
	nexus_user_put_u_char(&dest_end, (u_char *)src, count);
	*position += dest_end - dest_start;
	break;
      case MPIR_PACKED:
	nexus_warning("You shouldn't be packing a packed datatype\n");
      case MPIR_BYTE:
	nexus_user_put_byte(&dest_end, (nexus_byte_t *)src, count);
	*position += dest_end - dest_start;
	break;
      case MPIR_SHORT:
	nexus_user_put_short(&dest_end, (short *)src, count);
	*position += dest_end - dest_start;
	break;
      case MPIR_USHORT:
	nexus_user_put_u_short(&dest_end, (u_short *)src, count);
	*position += dest_end - dest_start;
	break;
      case MPIR_INT:
	nexus_user_put_int(&dest_end, (int *)src, count);
	*position += dest_end - dest_start;
	break;
      case MPIR_UINT:
	nexus_user_put_u_int(&dest_end, (u_int *)src, count);
	*position += dest_end - dest_start;
	break;
      case MPIR_LONG:
	nexus_user_put_long(&dest_end, (long *)src, count);
	*position += dest_end - dest_start;
	break;
      case MPIR_ULONG:
	nexus_user_put_u_long(&dest_end, (u_long *)src, count);
	*position += dest_end - dest_start;
	break;
      case MPIR_FLOAT:
	nexus_user_put_float(&dest_end, (float *)src, count);
	*position += dest_end - dest_start;
	break;
      case MPIR_COMPLEX:
	nexus_user_put_float(&dest_end, (float *)src, 2 * count);
	*position += dest_end - dest_start;
	break;
      case MPIR_DOUBLE:
	nexus_user_put_double(&dest_end, (double *)src, count);
	*position += dest_end - dest_start;
	break;
      case MPIR_DOUBLE_COMPLEX:
	nexus_user_put_double(&dest_end, (double *)src, 2 * count);
	*position += dest_end - dest_start;
	break;
      /* 
       * These are complex datatypes that Nexus does not know how to
       * deal with directly.  Break them down into their component parts
       * and put the more basic elements by recursively calling
       * MPI_Pack_buffer().  Note that is allows complex structures like
       * an MPIR_STRUCT inside an MPIR_VECTOR, etc.
       */
      case MPIR_CONTIG:
	MPID_Pack(src,
		  count * datatype->count,
		  datatype->old_type,
		  dest,
		  maxcount,
		  position,
		  comm,
		  partner,
		  msgrep,
		  msgact,
		  error_code);
	break;
      case MPIR_VECTOR:
      case MPIR_HVECTOR:
	tmp_buf = src;
	for (i = 0; i < count; i++)
	{
	    src = tmp_buf;
	    for (j = 0; j < datatype->count; j++)
	    {
		MPID_Pack(src,
			  datatype->blocklen,
			  datatype->old_type,
			  dest,
			  maxcount,
			  position,
			  comm,
			  partner,
			  msgrep,
			  msgact,
			  error_code);

		void_pointer_displace(src, datatype->stride);
	    }
	    void_pointer_displace(tmp_buf, datatype->extent);
	}
	break;
      case MPIR_INDEXED:
      case MPIR_HINDEXED:
	for (i = 0; i < count; i++)
	{
	    for (j = 0; j < datatype->count; j++)
	    {
		tmp_buf = (char *)src + datatype->indices[j];
		MPID_Pack(tmp_buf,
			  datatype->blocklens[j],
			  datatype->old_type,
			  dest,
			  maxcount,
			  position,
			  comm,
			  partner,
			  msgrep,
			  msgact,
			  error_code);
	    }
	    void_pointer_displace(src, datatype->extent);
	}
	break;
      case MPIR_STRUCT:
	for (i = 0; i < count; i++)
	{
	    for (j = 0; j < datatype->count; j++)
	    {
		tmp_buf = (char *)src + datatype->indices[j];
		MPID_Pack(tmp_buf,
			  datatype->blocklens[j],
			  datatype->old_types[j],
			  dest,
			  maxcount,
			  position,
			  comm,
			  partner,
			  msgrep,
			  msgact,
			  error_code);
	    }
	    void_pointer_displace(src, datatype->extent);
	}
	break;
      /*
       * I think these are 0 sized things for some purpose.  I need to
       * see what I really should be doing here.
       */
      case MPIR_UB:
      case MPIR_LB:
	break;
      /* 
       * Nexus cannot deal with these types directly, and they cannot be
       * broken down into more simple parts.  Return an error to the
       * calling procedure to indicate the problem.
       */
      case MPIR_LOGICAL:
      case MPIR_LONGDOUBLE:
	*error_code = MPI_ERR_TYPE;
	return ;
      /* 
       * This is not a legal type.  Return an error code.
       */
      default:
	*error_code = MPI_ERR_INTERN;
	return ;
    }
}

void MPID_Unpack(void *src,
		 int maxcount,
		 MPID_Msgrep_t msgrep,
		 int *in_position,
		 void *dest,
		 int count,
		 struct MPIR_DATATYPE *datatype,
		 int *out_position,
		 struct MPIR_COMMUNICATOR *comm,
		 int partner,
		 int *error_code)
{
    nexus_byte_t *dest_end, *dest_start;
    nexus_byte_t *src_end, *src_start;
    void *tmp_buf;
    int i, j;

    dest_start = dest_end = (nexus_byte_t *)dest + *out_position;
    src_start = src_end = (nexus_byte_t *)src + *in_position;

    switch (datatype->dte_type)
    {
      /* 
       * These are basic datatypes that Nexus directly knows how to
       * deal with.  Just put them into the send buffer and return.
       */
      case MPIR_CHAR:
	nexus_user_get_char(&src_end,
			    (char *)dest_start,
			    count,
			    msgrep);
	dest_end = (nexus_byte_t *)((char *)dest_start + count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_UCHAR:
	nexus_user_get_u_char(&src_end,
			      (u_char *)dest_start,
			      count,
			      msgrep);
	dest_end = (nexus_byte_t *)((u_char *)dest_start + count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_PACKED:
	/* nexus_warning("You shouldn't be packing a packed datatype\n"); */
      case MPIR_BYTE:
	nexus_user_get_byte(&src_end, dest_start, count, msgrep);
	dest_end = dest_start + count;
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_SHORT:
	nexus_user_get_short(&src_end,
			     (short *)dest_start,
			     count,
			     msgrep);
	dest_end = (nexus_byte_t *)((short *)dest_start + count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_USHORT:
	nexus_user_get_u_short(&src_end,
			       (u_short *)dest_start,
			       count,
			       msgrep);
	dest_end = (nexus_byte_t *)((u_short *)dest_start + count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_INT:
	nexus_user_get_int(&src_end,
			   (int *)dest_start,
			   count,
			   msgrep);
	dest_end = (nexus_byte_t *)((int *)dest_start + count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_UINT:
	nexus_user_get_u_int(&src_end,
			     (u_int *)dest_start,
			     count,
			     msgrep);
	dest_end = (nexus_byte_t *)((u_int *)dest_start + count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_LONG:
	nexus_user_get_long(&src_end,
			    (long *)dest_start,
			    count,
			    msgrep);
	dest_end = (nexus_byte_t *)((long *)dest_start + count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_ULONG:
	nexus_user_get_u_long(&src_end,
			      (u_long *)dest_start,
			      count,
			      msgrep);
	dest_end = (nexus_byte_t *)((u_long *)dest_start + count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_FLOAT:
	nexus_user_get_float(&src_end,
			     (float *)dest_start,
			     count,
			     msgrep);
	dest_end = (nexus_byte_t *)((float *)dest_start + count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_COMPLEX:
	nexus_user_get_float(&src_end,
			     (float *)dest_start,
			     2 * count,
			     msgrep);
	dest_end = (nexus_byte_t *)((float *)dest_start + 2 * count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_DOUBLE:
	nexus_user_get_double(&src_end,
			      (double *)dest_start,
			      count,
			      msgrep);
	dest_end = (nexus_byte_t *)((double *)dest_start + count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_DOUBLE_COMPLEX:
	nexus_user_get_double(&src_end,
			      (double *)dest_start,
			      2 * count,
			      msgrep);
	dest_end = (nexus_byte_t *)((double *)dest_start + 2 * count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      /* 
       * These are complex datatypes that Nexus does not know how to
       * deal with directly.  Break them down into their component parts
       * and put the more basic elements by recursively calling
       * MPI_Pack_buffer().  Note that is allows complex structures like
       * an MPIR_STRUCT inside an MPIR_VECTOR, etc.
       */
      case MPIR_CONTIG:
	MPID_Unpack(src,
		    maxcount,
		    msgrep,
		    in_position,
		    dest,
		    count * datatype->count,
		    datatype->old_type,
		    out_position,
		    comm,
		    partner,
		    error_code);
	break;
      case MPIR_VECTOR:
      case MPIR_HVECTOR:
	tmp_buf = dest;
	for (i = 0; i < count; i++)
	{
	    dest = tmp_buf;
	    for (j = 0; j < datatype->count; j++)
	    {
		int tmp_int = 0;

		MPID_Unpack(src,
			    maxcount,
			    msgrep,
			    in_position,
			    dest,
			    datatype->blocklen,
			    datatype->old_type,
			    &tmp_int,
			    comm,
			    partner,
			    error_code);

		*out_position += tmp_int;
		void_pointer_displace(dest, datatype->stride);
	    }
	    void_pointer_displace(tmp_buf, datatype->extent);
	}
	break;
      case MPIR_INDEXED:
      case MPIR_HINDEXED:
	for (i = 0; i < count; i++)
	{
	    for (j = 0; j < datatype->count; j++)
	    {
		int tmp_int = 0;

		tmp_buf = (char *)dest + datatype->indices[j];
		MPID_Unpack(src,
			    maxcount,
			    msgrep,
			    in_position,
			    tmp_buf,
			    datatype->blocklens[j],
			    datatype->old_type,
			    &tmp_int,
			    comm,
			    partner,
			    error_code);
		
		*out_position += tmp_int;
	    }
	    void_pointer_displace(dest, datatype->extent);
	}
	break;
      case MPIR_STRUCT:
	for (i = 0; i < count; i++)
	{
	    for (j = 0; j < datatype->count; j++)
	    {
		int tmp_int = 0;

		tmp_buf = (char *)dest + datatype->indices[j];
		MPID_Unpack(src,
			    maxcount,
			    msgrep,
			    in_position,
			    tmp_buf,
			    datatype->blocklens[j],
			    datatype->old_types[j],
			    &tmp_int,
			    comm,
			    partner,
			    error_code);

		*out_position += tmp_int;
	    }
	    void_pointer_displace(dest, datatype->extent);
	}
	break;
      /*
       * I think these are 0 sized things for some purpose.  I need to
       * see what I really should be doing here.
       */
      case MPIR_UB:
      case MPIR_LB:
	break;
      /* 
       * Nexus cannot deal with these types directly, and they cannot be
       * broken down into more simple parts.  Return an error to the
       * calling procedure to indicate the problem.
       */
      case MPIR_LOGICAL:
      case MPIR_LONGDOUBLE:
	*error_code = MPI_ERR_TYPE;
	return ;
      /* 
       * This is not a legal type.  Return an error code.
       */
      default:
	*error_code = MPI_ERR_INTERN;
	return ;
    }
}
