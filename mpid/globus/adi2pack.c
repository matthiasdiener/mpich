/*
 * Nexus-MPI Abstract Device/2 Implementation
 *  (C) 1996 by Argonne National Laboratory and Mississipi State University.
 *      All rights reserved.  See COPYRIGHT in top-level directory.
 * 
 * Authors: George K. Thiruvathukal and Jonathan Geisler
 *
 * Version: $Id: adi2pack.c,v 1.6 1999/03/02 17:01:46 gropp Exp $
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

static void MPID_Pack_size_recursive(int count,
				    struct MPIR_DATATYPE *datatype,
				    MPID_Msg_pack_t msgact,
				    int *size);

#if 0 /* not needed by globus device */
void MPID_Msg_rep(struct MPIR_COMMUNICATOR *comm,
		  int partner,
		  struct MPIR_DATATYPE *datatype,
		  MPID_Msgrep_t *msgrep,
		  MPID_Msg_pack_t *msgact)
{
    *msgrep = remote_formats[comm->lrank_to_grank[partner]];
    *msgact = MPID_MSG_OK;
}
#endif

int MPID_Msgrep_from_comm(struct MPIR_COMMUNICATOR *comm)
{
/* globus_nexus_printf("NICK: MPID_Msgrep_from_comm() about to return %d\n", NEXUS_DC_FORMAT_LOCAL); */
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
	globus_nexus_warning("WARNING - unrecognized msgrep %d\n", msgrep);
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
	*pack_size += globus_nexus_sizeof_char(count);
	*remote_size += nexus_dc_sizeof_remote_char(count, remote_format);
	break;
      case MPIR_UCHAR:
	*pack_size += globus_nexus_sizeof_u_char(count);
	*remote_size += nexus_dc_sizeof_remote_u_char(count, remote_format);
	break;
      case MPIR_PACKED:
      case MPIR_BYTE:
	*pack_size += globus_nexus_sizeof_byte(count);
	*remote_size += nexus_dc_sizeof_remote_byte(count, remote_format);
	break;
      case MPIR_SHORT:
	*pack_size += globus_nexus_sizeof_short(count);
	*remote_size += nexus_dc_sizeof_remote_short(count, remote_format);
	break;
      case MPIR_USHORT:
	*pack_size += globus_nexus_sizeof_u_short(count);
	*remote_size += nexus_dc_sizeof_remote_u_short(count, remote_format);
	break;
      case MPIR_INT:
      case MPIR_LOGICAL: /* 'logical' in FORTRAN is always same as 'int' */
	*pack_size += globus_nexus_sizeof_int(count);
	*remote_size += nexus_dc_sizeof_remote_int(count, remote_format);
	break;
      case MPIR_UINT:
	*pack_size += globus_nexus_sizeof_u_int(count);
	*remote_size += nexus_dc_sizeof_remote_u_int(count, remote_format);
	break;
      case MPIR_LONG:
	*pack_size += globus_nexus_sizeof_long(count);
	*remote_size += nexus_dc_sizeof_remote_long(count, remote_format);
	break;
      case MPIR_ULONG:
	*pack_size += globus_nexus_sizeof_u_long(count);
	*remote_size += nexus_dc_sizeof_remote_u_long(count, remote_format);
	break;
      case MPIR_FLOAT:
	*pack_size += globus_nexus_sizeof_float(count);
	*remote_size += nexus_dc_sizeof_remote_float(count, remote_format);
	break;
      case MPIR_COMPLEX:
	*pack_size += globus_nexus_sizeof_float(2 * count);
	*remote_size += nexus_dc_sizeof_remote_float(2 * count, remote_format);
	break;
      case MPIR_DOUBLE:
	*pack_size += globus_nexus_sizeof_double(count);
	*remote_size += nexus_dc_sizeof_remote_double(count, remote_format);
	break;
      case MPIR_DOUBLE_COMPLEX:
	*pack_size += globus_nexus_sizeof_double(2 * count);
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
/* globus_nexus_printf("NICK: MPID_Pack_exact_size() case MPIR_STRUCT: i = %d datatype->blocklens[i] = %d datatype->old_types[i] = %d\n", i, datatype->blocklens[i], datatype->old_types[i]); */
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
	globus_nexus_warning("Type %d (MPIR_LONGDOUBLE) is not supported for packing\n", 
	    datatype->dte_type);
	break;
      default:
	globus_nexus_warning("Type %d is not not recoginized and not supported for packing\n", 
	    datatype->dte_type);
        break;
    }
}

/*
 *
 * before the first call to MPID_Pack_size_recursive(), *size must 
 * be initialized to 0.
 *
 * it is NOT assumed that caller to MPID_Pack_size() initialized *size = 0,
 * so we do so here.
 *
 */

void MPID_Pack_size(int count,
		    struct MPIR_DATATYPE *datatype,
		    MPID_Msg_pack_t msgact,
		    int *size)
{
    *size = 0;
    MPID_Pack_size_recursive(count, datatype, msgact, size);
} /* end MPID_Pack_size() */

/*
 *
 * before the first call to MPID_Pack_size_recursive(), *size must 
 * be initialized to 0.
 *
 */

static void MPID_Pack_size_recursive(int count,
				    struct MPIR_DATATYPE *datatype,
				    MPID_Msg_pack_t msgact,
				    int *size)
{
    int tmp;
    int i;

/* printf("NICK: enter MPID_Pack_size_recursive(): count %d type %d (MPIR_SHORT %d )*size %d\n", count, datatype->dte_type, MPIR_SHORT, *size); */
    switch(datatype->dte_type)
    {
      case MPIR_CHAR:
	*size += globus_nexus_sizeof_char(count);
	break;
      case MPIR_UCHAR:
	*size += globus_nexus_sizeof_u_char(count);
	break;
      case MPIR_PACKED:
      case MPIR_BYTE:
	*size += globus_nexus_sizeof_byte(count);
	break;
      case MPIR_SHORT:
	*size += globus_nexus_sizeof_short(count);
	break;
      case MPIR_USHORT:
	*size += globus_nexus_sizeof_u_short(count);
	break;
      case MPIR_INT:
      case MPIR_LOGICAL: /* 'logical' in FORTRAN is always same as 'int' */
	*size += globus_nexus_sizeof_int(count);
	break;
      case MPIR_UINT:
	*size += globus_nexus_sizeof_u_int(count);
	break;
      case MPIR_LONG:
	*size += globus_nexus_sizeof_long(count);
	break;
      case MPIR_ULONG:
	*size += globus_nexus_sizeof_u_long(count);
	break;
      case MPIR_FLOAT:
	*size += globus_nexus_sizeof_float(count);
	break;
      case MPIR_COMPLEX:
	*size += globus_nexus_sizeof_float(2 * count);
	break;
      case MPIR_DOUBLE:
	*size += globus_nexus_sizeof_double(count);
	break;
      case MPIR_DOUBLE_COMPLEX:
	*size += globus_nexus_sizeof_double(2 * count);
	break;
      case MPIR_CONTIG:
        MPID_Pack_size_recursive(count * datatype->count,
			       datatype->old_type,
			       msgact,
			       size);
	break;
      case MPIR_VECTOR:
      case MPIR_HVECTOR:
        tmp = 0;
        MPID_Pack_size_recursive(datatype->blocklen,
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

	    MPID_Pack_size_recursive(datatype->blocklens[i],
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

	    MPID_Pack_size_recursive(datatype->blocklens[i],
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
	globus_nexus_warning("Type %d (MPIR_LONGDOUBLE) is not supported for packing\n", 
	    datatype->dte_type);
	break;
      default:
	globus_nexus_warning("Type %d is not not recoginized and not supported for packing\n", 
	    datatype->dte_type);
        break;
    }
/* printf("NICK: exit MPID_Pack_size_recursive(): count %d type %d *size %d\n", count, datatype->dte_type, *size); */
}

void MPID_Pack_buffer_elements(int count,
		   	       struct MPIR_DATATYPE *datatype,
		   	       globus_nexus_buffer_t *send_buffer,
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
      case MPIR_LOGICAL: /* 'logical' in FORTRAN is always same as 'int' */
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
		      globus_nexus_buffer_t *send_buf)
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
	globus_nexus_put_char(send_buf, (char *)src, count);
#endif
	break;
      case MPIR_UCHAR:
#ifdef USE_DIREcT
	nexus_direct_put_u_char(send_buf, (u_char *)src, count);
#else
	globus_nexus_put_u_char(send_buf, (u_char *)src, count);
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
	globus_nexus_put_byte(send_buf, (nexus_byte_t *)src, count);
#endif
	break;
      case MPIR_SHORT:
#ifdef USE_DIRECT
	nexus_direct_put_short(send_buf, (short *)src, count);
#else
	globus_nexus_put_short(send_buf, (short *)src, count);
#endif
	break;
      case MPIR_USHORT:
#ifdef USE_DIRECT
	nexus_direct_put_u_short(send_buf, (u_short *)src, count);
#else
	globus_nexus_put_u_short(send_buf, (u_short *)src, count);
#endif
	break;
      case MPIR_INT:
      case MPIR_LOGICAL: /* 'logical' in FORTRAN is always same as 'int' */
#ifdef USE_DIRECT
	nexus_direct_put_int(send_buf, (int *)src, count);
#else
	globus_nexus_put_int(send_buf, (int *)src, count);
#endif
	break;
      case MPIR_UINT:
#ifdef USE_DIRECT
	nexus_direct_put_u_int(send_buf, (u_int *)src, count);
#else
	globus_nexus_put_u_int(send_buf, (u_int *)src, count);
#endif
	break;
      case MPIR_LONG:
#ifdef USE_DIRECT
	nexus_direct_put_long(send_buf, (long *)src, count);
#else
	globus_nexus_put_long(send_buf, (long *)src, count);
#endif
	break;
      case MPIR_ULONG:
#ifdef USE_DIRECT
	nexus_direct_put_u_long(send_buf, (u_long *)src, count);
#else
	globus_nexus_put_u_long(send_buf, (u_long *)src, count);
#endif
	break;
      case MPIR_FLOAT:
#ifdef USE_DIRECT
	nexus_direct_put_float(send_buf, (float *)src, count);
#else
	globus_nexus_put_float(send_buf, (float *)src, count);
#endif
	break;
      case MPIR_COMPLEX:
#ifdef USE_DIRECT
	nexus_direct_put_float(send_buf, (float *)src, 2 * count);
#else
	globus_nexus_put_float(send_buf, (float *)src, 2 * count);
#endif
	break;
      case MPIR_DOUBLE:
#ifdef USE_DIRECT
	nexus_direct_put_double(send_buf, (double *)src, count);
#else
	globus_nexus_put_double(send_buf, (double *)src, count);
#endif
	break;
      case MPIR_DOUBLE_COMPLEX:
#ifdef USE_DIRECT
	nexus_direct_put_double(send_buf, (double *)src, 2 * count);
#else
	globus_nexus_put_double(send_buf, (double *)src, 2 * count);
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
      /* case MPIR_LOGICAL: */
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

/*
 * It is assumed that there are 'count' 'datatypes' in the 'recv_buf'
 * and that ALL of them are complete.  In other words, that there is
 * no partial data in the 'recv_buf' like missing data from the last part
 * of the last user-defined data structure in a vector of user-define data 
 * structures.
 *
 * To retreive data from a 'recv_buf' where there is incomplete data, you
 * should call MPID_Unpack_partial_buffer() (below).  MPID_Unpack_buffer() may
 * be used to extract the first N-1 elements of a vector of 'datatype', because
 * those must be complete according to the MPI standard, and then use
 * MPID_Unpack_partial_buffer() from the last element if it is known to be
 * incomplete.
 * 
 */

void MPID_Unpack_buffer(void *dest,
			int count,
			struct MPIR_DATATYPE *datatype,
			globus_nexus_buffer_t *recv_buf)
{
    void *tmp_buf;
    int i, j;

/* if (datatype->dte_type == MPIR_SHORT) globus_nexus_printf("NICK: enter MPID_Unpack_buffer() into dest %x datatype->dte_type %d (MPIR_SHORT = %d) count %d\n", dest, datatype->dte_type, MPIR_SHORT, count); */
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
	globus_nexus_get_char(recv_buf, (char *)dest, count);
#endif
	break;
      case MPIR_UCHAR:
#ifdef USE_DIRECT
	nexus_direct_get_u_char(recv_buf, (u_char *)dest, count);
#else
	globus_nexus_get_u_char(recv_buf, (u_char *)dest, count);
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
	globus_nexus_get_byte(recv_buf, (nexus_byte_t *)dest, count);
#endif
	break;
      case MPIR_SHORT:
#ifdef USE_DIRECT
	nexus_direct_get_short(recv_buf, (short *)dest, count);
#else
	globus_nexus_get_short(recv_buf, (short *)dest, count);
#endif
	break;
      case MPIR_USHORT:
#ifdef USE_DIRECT
	nexus_direct_get_u_short(recv_buf, (u_short *)dest, count);
#else
	globus_nexus_get_u_short(recv_buf, (u_short *)dest, count);
#endif
	break;
      case MPIR_INT:
      case MPIR_LOGICAL: /* 'logical' in FORTRAN is always same as 'int' */
#ifdef USE_DIRECT
	nexus_direct_get_int(recv_buf, (int *)dest, count);
#else
	globus_nexus_get_int(recv_buf, (int *)dest, count);
#endif
	break;
      case MPIR_UINT:
#ifdef USE_DIRECT
	nexus_direct_get_u_int(recv_buf, (u_int *)dest, count);
#else
	globus_nexus_get_u_int(recv_buf, (u_int *)dest, count);
#endif
	break;
      case MPIR_LONG:
#ifdef USE_DIRECT
	nexus_direct_get_long(recv_buf, (long *)dest, count);
#else
	globus_nexus_get_long(recv_buf, (long *)dest, count);
#endif
	break;
      case MPIR_ULONG:
#ifdef USE_DIRECT
	nexus_direct_get_u_long(recv_buf, (u_long *)dest, count);
#else
	globus_nexus_get_u_long(recv_buf, (u_long *)dest, count);
#endif
	break;
      case MPIR_FLOAT:
#ifdef USE_DIRECT
	nexus_direct_get_float(recv_buf, (float *)dest, count);
#else
	globus_nexus_get_float(recv_buf, (float *)dest, count);
#endif
	break;
      case MPIR_COMPLEX:
#ifdef USE_DIRECT
	nexus_direct_get_float(recv_buf, (float *)dest, 2 * count);
#else
	globus_nexus_get_float(recv_buf, (float *)dest, 2 * count);
#endif
	break;
      case MPIR_DOUBLE:
#ifdef USE_DIRECT
	nexus_direct_get_double(recv_buf, (double *)dest, count);
#else
	globus_nexus_get_double(recv_buf, (double *)dest, count);
#endif
	break;
      case MPIR_DOUBLE_COMPLEX:
#ifdef USE_DIRECT
	nexus_direct_get_double(recv_buf, (double *)dest, 2 * count);
#else
	globus_nexus_get_double(recv_buf, (double *)dest, 2 * count);
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
      /* case MPIR_LOGICAL: */
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

/* if (datatype->dte_type == MPIR_SHORT) {int i; char *cp; printf("NICK: exit MPID_Unpack_buffer() first 10 bytes of dest %x buffer >", dest); for (cp=(char *) dest,i = 0; i < 10; i ++, cp++) printf("%0x", *cp); printf("<\n"); } */
} /* end MPID_Unpack_buffer() */

/*
 * This function is called when we know that data is missing from the 
 * end of 'datatype'.  This most commonly occurs when the 'datatype' is
 * a user-defined structure.  According to the MPI standard, it is acceptable
 * to receive into an array of N of any datatype (including user-defined data
 * structures) and only send some M <= N elements, and furthermore, the 
 * Mth element sent may have data omitted from the end.  This function is
 * called to handle the special case of extracting the partial data from that 
 * last element.
 *
 * The reason a new function was written is that care must be taken to only
 * extract as much data is there.  So before each globus_nexus_get_DATATYPE, we
 * must calculate the amount of data to retrieve.  This takes time, and
 * in practice is rarely called for.  So, for the majority of the cases,
 * MPID_Unpack_buffer() (above) can be used when full data is sent.  There
 * the calculation is *not* made before each globus_nexus_get_DATATYPE.  
 * For cases where data is missing from the Mth element, MPID_Unpack_buffer()
 * is called to rapidly extract the first M-1 elements from the 'recv_buf'
 * and then this function is called to carefully extract the remaining
 * data from the Mth element.
 *
 * It is assumed that *done == GLOBUS_FALSE upon entering this function.
 *
 */

void MPID_Unpack_partial_buffer(void *dest,
				int *recvd_nbytes,
				int req_nelem,
				int dataorigin_format,
				int *dataorigin_remaining_nbytes,
				struct MPIR_DATATYPE *req_datatype,
				globus_nexus_buffer_t *recv_buf,
				globus_bool_t *done)
{
    void *tmp_buf;
    int i, j;
    int dataorigin_unitsize;
    int extract_nelem;
    int inbuf_nelem;
    int local_unitsize;

/* globus_nexus_printf("NICK: enter MPID_Unpack_partial_buffer() into dest %x req_datatype->dte_type %d (MPIR_DOUBLE = %d) done (%x) %s\n", dest, req_datatype->dte_type, MPIR_DOUBLE, done, (done ? (*done ? "T" :"F") : "ERROR-NULL")); */
    switch (req_datatype->dte_type)
    {
      /* 
       * These are basic datatypes that Nexus directly knows how to
       * deal with.  Just put them into the send buffer and return.
       */
      case MPIR_CHAR:
	dataorigin_unitsize = 0;
	local_unitsize = 0;
	MPID_Pack_exact_size(1,
			    req_datatype,
			    dataorigin_format,
			    &local_unitsize,
			    &dataorigin_unitsize);
	inbuf_nelem = (*dataorigin_remaining_nbytes) / dataorigin_unitsize;
	if (extract_nelem = (req_nelem < inbuf_nelem ? req_nelem : inbuf_nelem))
	{
#ifdef USE_DIRECT
	    nexus_direct_get_char(recv_buf, (char *)dest, extract_nelem);
#else
	    globus_nexus_get_char(recv_buf, (char *)dest, extract_nelem);
#endif
	    *dataorigin_remaining_nbytes = *dataorigin_remaining_nbytes 
		    - (extract_nelem * dataorigin_unitsize);
	    *recvd_nbytes = *recvd_nbytes + (extract_nelem * local_unitsize);
	} /* endif */
	if (extract_nelem < req_nelem)
	{
	    *done = GLOBUS_TRUE;
	    if (*dataorigin_remaining_nbytes > 0)
	    {
		globus_nexus_printf("WARNING: after extracting %d of type %d (%d bytes), incoming buffer has %d bytes of residual data at its end that was NOT extracted (all byte counts in dataorigin format)\n", extract_nelem, req_datatype->dte_type, extract_nelem * dataorigin_unitsize, *dataorigin_remaining_nbytes);
	    } /* endif */
	} /* endif */
	break;
      case MPIR_UCHAR:
	dataorigin_unitsize = 0;
	local_unitsize = 0;
	MPID_Pack_exact_size(1,
			    req_datatype,
			    dataorigin_format,
			    &local_unitsize,
			    &dataorigin_unitsize);
	inbuf_nelem = (*dataorigin_remaining_nbytes) / dataorigin_unitsize;
	if (extract_nelem = (req_nelem < inbuf_nelem ? req_nelem : inbuf_nelem))
	{
#ifdef USE_DIRECT
	    nexus_direct_get_u_char(recv_buf, (u_char *)dest, extract_nelem);
#else
	    globus_nexus_get_u_char(recv_buf, (u_char *)dest, extract_nelem);
#endif
	    *dataorigin_remaining_nbytes = *dataorigin_remaining_nbytes 
		    - (extract_nelem * dataorigin_unitsize);
	    *recvd_nbytes = *recvd_nbytes + (extract_nelem * local_unitsize);
	} /* endif */
	if (extract_nelem < req_nelem)
	{
	    *done = GLOBUS_TRUE;
	    if (*dataorigin_remaining_nbytes > 0)
	    {
		globus_nexus_printf("WARNING: after extracting %d of type %d (%d bytes), incoming buffer has %d bytes of residual data at its end that was NOT extracted (all byte counts in dataorigin format)\n", extract_nelem, req_datatype->dte_type, extract_nelem * dataorigin_unitsize, *dataorigin_remaining_nbytes);
	    } /* endif */
	} /* endif */
	break;
      case MPIR_PACKED:
	dataorigin_unitsize = 0;
	local_unitsize = 0;
	MPID_Pack_exact_size(1,
			    req_datatype,
			    dataorigin_format,
			    &local_unitsize,
			    &dataorigin_unitsize);
	inbuf_nelem = (*dataorigin_remaining_nbytes) / dataorigin_unitsize;
	if (extract_nelem = (req_nelem < inbuf_nelem ? req_nelem : inbuf_nelem))
	{
#ifdef USE_DIRECT
	    nexus_direct_get_user(recv_buf, (nexus_byte_t *)dest, extract_nelem);
#else
	    nexus_get_user(recv_buf, (nexus_byte_t *)dest, extract_nelem);
#endif
	    *dataorigin_remaining_nbytes = *dataorigin_remaining_nbytes 
		    - (extract_nelem * dataorigin_unitsize);
	    *recvd_nbytes = *recvd_nbytes + (extract_nelem * local_unitsize);
	} /* endif */
	if (extract_nelem < req_nelem)
	{
	    *done = GLOBUS_TRUE;
	    if (*dataorigin_remaining_nbytes > 0)
	    {
		globus_nexus_printf("WARNING: after extracting %d of type %d (%d bytes), incoming buffer has %d bytes of residual data at its end that was NOT extracted (all byte counts in dataorigin format)\n", extract_nelem, req_datatype->dte_type, extract_nelem * dataorigin_unitsize, *dataorigin_remaining_nbytes);
	    } /* endif */
	} /* endif */
	break;
      case MPIR_BYTE:
	dataorigin_unitsize = 0;
	local_unitsize = 0;
	MPID_Pack_exact_size(1,
			    req_datatype,
			    dataorigin_format,
			    &local_unitsize,
			    &dataorigin_unitsize);
	inbuf_nelem = (*dataorigin_remaining_nbytes) / dataorigin_unitsize;
	if (extract_nelem = (req_nelem < inbuf_nelem ? req_nelem : inbuf_nelem))
	{
#ifdef USE_DIRECT
	    nexus_direct_get_byte(recv_buf, (nexus_byte_t *)dest, extract_nelem);
#else
	    globus_nexus_get_byte(recv_buf, (nexus_byte_t *)dest, extract_nelem);
#endif
	    *dataorigin_remaining_nbytes = *dataorigin_remaining_nbytes 
		    - (extract_nelem * dataorigin_unitsize);
	    *recvd_nbytes = *recvd_nbytes + (extract_nelem * local_unitsize);
	} /* endif */
	if (extract_nelem < req_nelem)
	{
	    *done = GLOBUS_TRUE;
	    if (*dataorigin_remaining_nbytes > 0)
	    {
		globus_nexus_printf("WARNING: after extracting %d of type %d (%d bytes), incoming buffer has %d bytes of residual data at its end that was NOT extracted (all byte counts in dataorigin format)\n", extract_nelem, req_datatype->dte_type, extract_nelem * dataorigin_unitsize, *dataorigin_remaining_nbytes);
	    } /* endif */
	} /* endif */
	break;
      case MPIR_SHORT:
	dataorigin_unitsize = 0;
	local_unitsize = 0;
	MPID_Pack_exact_size(1,
			    req_datatype,
			    dataorigin_format,
			    &local_unitsize,
			    &dataorigin_unitsize);
	inbuf_nelem = (*dataorigin_remaining_nbytes) / dataorigin_unitsize;
	if (extract_nelem = (req_nelem < inbuf_nelem ? req_nelem : inbuf_nelem))
	{
#ifdef USE_DIRECT
	    nexus_direct_get_short(recv_buf, (short *)dest, extract_nelem);
#else
	    globus_nexus_get_short(recv_buf, (short *)dest, extract_nelem);
#endif
	    *dataorigin_remaining_nbytes = *dataorigin_remaining_nbytes 
		    - (extract_nelem * dataorigin_unitsize);
	    *recvd_nbytes = *recvd_nbytes + (extract_nelem * local_unitsize);
	} /* endif */
	if (extract_nelem < req_nelem)
	{
	    *done = GLOBUS_TRUE;
	    if (*dataorigin_remaining_nbytes > 0)
	    {
		globus_nexus_printf("WARNING: after extracting %d of type %d (%d bytes), incoming buffer has %d bytes of residual data at its end that was NOT extracted (all byte counts in dataorigin format)\n", extract_nelem, req_datatype->dte_type, extract_nelem * dataorigin_unitsize, *dataorigin_remaining_nbytes);
	    } /* endif */
	} /* endif */
	break;
      case MPIR_USHORT:
	dataorigin_unitsize = 0;
	local_unitsize = 0;
	MPID_Pack_exact_size(1,
			    req_datatype,
			    dataorigin_format,
			    &local_unitsize,
			    &dataorigin_unitsize);
	inbuf_nelem = (*dataorigin_remaining_nbytes) / dataorigin_unitsize;
	if (extract_nelem = (req_nelem < inbuf_nelem ? req_nelem : inbuf_nelem))
	{
#ifdef USE_DIRECT
	    nexus_direct_get_u_short(recv_buf, (u_short *)dest, extract_nelem);
#else
	    globus_nexus_get_u_short(recv_buf, (u_short *)dest, extract_nelem);
#endif
	    *dataorigin_remaining_nbytes = *dataorigin_remaining_nbytes 
		    - (extract_nelem * dataorigin_unitsize);
	    *recvd_nbytes = *recvd_nbytes + (extract_nelem * local_unitsize);
	} /* endif */
	if (extract_nelem < req_nelem)
	{
	    *done = GLOBUS_TRUE;
	    if (*dataorigin_remaining_nbytes > 0)
	    {
		globus_nexus_printf("WARNING: after extracting %d of type %d (%d bytes), incoming buffer has %d bytes of residual data at its end that was NOT extracted (all byte counts in dataorigin format)\n", extract_nelem, req_datatype->dte_type, extract_nelem * dataorigin_unitsize, *dataorigin_remaining_nbytes);
	    } /* endif */
	} /* endif */
	break;
      case MPIR_INT:
      case MPIR_LOGICAL: /* 'logical' in FORTRAN is always same as 'int' */
	dataorigin_unitsize = 0;
	local_unitsize = 0;
	MPID_Pack_exact_size(1,
			    req_datatype,
			    dataorigin_format,
			    &local_unitsize,
			    &dataorigin_unitsize);
	inbuf_nelem = (*dataorigin_remaining_nbytes) / dataorigin_unitsize;
	if (extract_nelem = (req_nelem < inbuf_nelem ? req_nelem : inbuf_nelem))
	{
#ifdef USE_DIRECT
	    nexus_direct_get_int(recv_buf, (int *)dest, extract_nelem);
#else
	    globus_nexus_get_int(recv_buf, (int *)dest, extract_nelem);
#endif
	    *dataorigin_remaining_nbytes = *dataorigin_remaining_nbytes 
		    - (extract_nelem * dataorigin_unitsize);
	    *recvd_nbytes = *recvd_nbytes + (extract_nelem * local_unitsize);
	} /* endif */
	if (extract_nelem < req_nelem)
	{
	    *done = GLOBUS_TRUE;
	    if (*dataorigin_remaining_nbytes > 0)
	    {
		globus_nexus_printf("WARNING: after extracting %d of type %d (%d bytes), incoming buffer has %d bytes of residual data at its end that was NOT extracted (all byte counts in dataorigin format)\n", extract_nelem, req_datatype->dte_type, extract_nelem * dataorigin_unitsize, *dataorigin_remaining_nbytes);
	    } /* endif */
	} /* endif */
	break;
      case MPIR_UINT:
	dataorigin_unitsize = 0;
	local_unitsize = 0;
	MPID_Pack_exact_size(1,
			    req_datatype,
			    dataorigin_format,
			    &local_unitsize,
			    &dataorigin_unitsize);
	inbuf_nelem = (*dataorigin_remaining_nbytes) / dataorigin_unitsize;
	if (extract_nelem = (req_nelem < inbuf_nelem ? req_nelem : inbuf_nelem))
	{
#ifdef USE_DIRECT
	    nexus_direct_get_u_int(recv_buf, (u_int *)dest, extract_nelem);
#else
	    globus_nexus_get_u_int(recv_buf, (u_int *)dest, extract_nelem);
#endif
	    *dataorigin_remaining_nbytes = *dataorigin_remaining_nbytes 
		    - (extract_nelem * dataorigin_unitsize);
	    *recvd_nbytes = *recvd_nbytes + (extract_nelem * local_unitsize);
	} /* endif */
	if (extract_nelem < req_nelem)
	{
	    *done = GLOBUS_TRUE;
	    if (*dataorigin_remaining_nbytes > 0)
	    {
		globus_nexus_printf("WARNING: after extracting %d of type %d (%d bytes), incoming buffer has %d bytes of residual data at its end that was NOT extracted (all byte counts in dataorigin format)\n", extract_nelem, req_datatype->dte_type, extract_nelem * dataorigin_unitsize, *dataorigin_remaining_nbytes);
	    } /* endif */
	} /* endif */
	break;
      case MPIR_LONG:
	dataorigin_unitsize = 0;
	local_unitsize = 0;
	MPID_Pack_exact_size(1,
			    req_datatype,
			    dataorigin_format,
			    &local_unitsize,
			    &dataorigin_unitsize);
	inbuf_nelem = (*dataorigin_remaining_nbytes) / dataorigin_unitsize;
	if (extract_nelem = (req_nelem < inbuf_nelem ? req_nelem : inbuf_nelem))
	{
#ifdef USE_DIRECT
	    nexus_direct_get_long(recv_buf, (long *)dest, extract_nelem);
#else
	    globus_nexus_get_long(recv_buf, (long *)dest, extract_nelem);
#endif
	    *dataorigin_remaining_nbytes = *dataorigin_remaining_nbytes 
		    - (extract_nelem * dataorigin_unitsize);
	    *recvd_nbytes = *recvd_nbytes + (extract_nelem * local_unitsize);
	} /* endif */
	if (extract_nelem < req_nelem)
	{
	    *done = GLOBUS_TRUE;
	    if (*dataorigin_remaining_nbytes > 0)
	    {
		globus_nexus_printf("WARNING: after extracting %d of type %d (%d bytes), incoming buffer has %d bytes of residual data at its end that was NOT extracted (all byte counts in dataorigin format)\n", extract_nelem, req_datatype->dte_type, extract_nelem * dataorigin_unitsize, *dataorigin_remaining_nbytes);
	    } /* endif */
	} /* endif */
	break;
      case MPIR_ULONG:
	dataorigin_unitsize = 0;
	local_unitsize = 0;
	MPID_Pack_exact_size(1,
			    req_datatype,
			    dataorigin_format,
			    &local_unitsize,
			    &dataorigin_unitsize);
	inbuf_nelem = (*dataorigin_remaining_nbytes) / dataorigin_unitsize;
	if (extract_nelem = (req_nelem < inbuf_nelem ? req_nelem : inbuf_nelem))
	{
#ifdef USE_DIRECT
	    nexus_direct_get_u_long(recv_buf, (u_long *)dest, extract_nelem);
#else
	    globus_nexus_get_u_long(recv_buf, (u_long *)dest, extract_nelem);
#endif
	    *dataorigin_remaining_nbytes = *dataorigin_remaining_nbytes 
		    - (extract_nelem * dataorigin_unitsize);
	    *recvd_nbytes = *recvd_nbytes + (extract_nelem * local_unitsize);
	} /* endif */
	if (extract_nelem < req_nelem)
	{
	    *done = GLOBUS_TRUE;
	    if (*dataorigin_remaining_nbytes > 0)
	    {
		globus_nexus_printf("WARNING: after extracting %d of type %d (%d bytes), incoming buffer has %d bytes of residual data at its end that was NOT extracted (all byte counts in dataorigin format)\n", extract_nelem, req_datatype->dte_type, extract_nelem * dataorigin_unitsize, *dataorigin_remaining_nbytes);
	    } /* endif */
	} /* endif */
	break;
      case MPIR_FLOAT:
	dataorigin_unitsize = 0;
	local_unitsize = 0;
	MPID_Pack_exact_size(1,
			    req_datatype,
			    dataorigin_format,
			    &local_unitsize,
			    &dataorigin_unitsize);
	inbuf_nelem = (*dataorigin_remaining_nbytes) / dataorigin_unitsize;
	if (extract_nelem = (req_nelem < inbuf_nelem ? req_nelem : inbuf_nelem))
	{
#ifdef USE_DIRECT
	    nexus_direct_get_float(recv_buf, (float *)dest, extract_nelem);
#else
	    globus_nexus_get_float(recv_buf, (float *)dest, extract_nelem);
#endif
	    *dataorigin_remaining_nbytes = *dataorigin_remaining_nbytes 
		    - (extract_nelem * dataorigin_unitsize);
	    *recvd_nbytes = *recvd_nbytes + (extract_nelem * local_unitsize);
	} /* endif */
	if (extract_nelem < req_nelem)
	{
	    *done = GLOBUS_TRUE;
	    if (*dataorigin_remaining_nbytes > 0)
	    {
		globus_nexus_printf("WARNING: after extracting %d of type %d (%d bytes), incoming buffer has %d bytes of residual data at its end that was NOT extracted (all byte counts in dataorigin format)\n", extract_nelem, req_datatype->dte_type, extract_nelem * dataorigin_unitsize, *dataorigin_remaining_nbytes);
	    } /* endif */
	} /* endif */
	break;
      case MPIR_COMPLEX:
	dataorigin_unitsize = 0;
	local_unitsize = 0;
	MPID_Pack_exact_size(1,
			    req_datatype,
			    dataorigin_format,
			    &local_unitsize,
			    &dataorigin_unitsize);
	inbuf_nelem = (*dataorigin_remaining_nbytes) / dataorigin_unitsize;
	if (extract_nelem = (req_nelem < inbuf_nelem ? req_nelem : inbuf_nelem))
	{
#ifdef USE_DIRECT
	    nexus_direct_get_float(recv_buf, (float *)dest, 2 * extract_nelem);
#else
	    globus_nexus_get_float(recv_buf, (float *)dest, 2 * extract_nelem);
#endif
	    *dataorigin_remaining_nbytes = *dataorigin_remaining_nbytes 
		    - (extract_nelem * dataorigin_unitsize);
	    *recvd_nbytes = *recvd_nbytes + (extract_nelem * local_unitsize);
	} /* endif */
	if (extract_nelem < req_nelem)
	{
	    *done = GLOBUS_TRUE;
	    if (*dataorigin_remaining_nbytes > 0)
	    {
		globus_nexus_printf("WARNING: after extracting %d of type %d (%d bytes), incoming buffer has %d bytes of residual data at its end that was NOT extracted (all byte counts in dataorigin format)\n", extract_nelem, req_datatype->dte_type, extract_nelem * dataorigin_unitsize, *dataorigin_remaining_nbytes);
	    } /* endif */
	} /* endif */
	break;
      case MPIR_DOUBLE:
	dataorigin_unitsize = 0;
	local_unitsize = 0;
	MPID_Pack_exact_size(1,
			    req_datatype,
			    dataorigin_format,
			    &local_unitsize,
			    &dataorigin_unitsize);
	inbuf_nelem = (*dataorigin_remaining_nbytes) / dataorigin_unitsize;
	if (extract_nelem = (req_nelem < inbuf_nelem ? req_nelem : inbuf_nelem))
	{
#ifdef USE_DIRECT
	    nexus_direct_get_double(recv_buf, (double *)dest, extract_nelem);
#else
	    globus_nexus_get_double(recv_buf, (double *)dest, extract_nelem);
#endif
	    *dataorigin_remaining_nbytes = *dataorigin_remaining_nbytes 
		    - (extract_nelem * dataorigin_unitsize);
	    *recvd_nbytes = *recvd_nbytes + (extract_nelem * local_unitsize);
	} /* endif */
	if (extract_nelem < req_nelem)
	{
	    *done = GLOBUS_TRUE;
	    if (*dataorigin_remaining_nbytes > 0)
	    {
		globus_nexus_printf("WARNING: after extracting %d of type %d (%d bytes), incoming buffer has %d bytes of residual data at its end that was NOT extracted (all byte counts in dataorigin format)\n", extract_nelem, req_datatype->dte_type, extract_nelem * dataorigin_unitsize, *dataorigin_remaining_nbytes);
	    } /* endif */
	} /* endif */
	break;
      case MPIR_DOUBLE_COMPLEX:
	dataorigin_unitsize = 0;
	local_unitsize = 0;
	MPID_Pack_exact_size(1,
			    req_datatype,
			    dataorigin_format,
			    &local_unitsize,
			    &dataorigin_unitsize);
	inbuf_nelem = (*dataorigin_remaining_nbytes) / dataorigin_unitsize;
	if (extract_nelem = (req_nelem < inbuf_nelem ? req_nelem : inbuf_nelem))
	{
#ifdef USE_DIRECT
	    nexus_direct_get_double(recv_buf, (double *)dest, 2 * extract_nelem);
#else
	    globus_nexus_get_double(recv_buf, (double *)dest, 2 * extract_nelem);
#endif
	    *dataorigin_remaining_nbytes = *dataorigin_remaining_nbytes 
		    - (extract_nelem * dataorigin_unitsize);
	    *recvd_nbytes = *recvd_nbytes + (extract_nelem * local_unitsize);
	} /* endif */
	if (extract_nelem < req_nelem)
	{
	    *done = GLOBUS_TRUE;
	    if (*dataorigin_remaining_nbytes > 0)
	    {
		globus_nexus_printf("WARNING: after extracting %d of type %d (%d bytes), incoming buffer has %d bytes of residual data at its end that was NOT extracted (all byte counts in dataorigin format)\n", extract_nelem, req_datatype->dte_type, extract_nelem * dataorigin_unitsize, *dataorigin_remaining_nbytes);
	    } /* endif */
	} /* endif */
	break;
      /* 
       * These are complex datatypes that Nexus does not know how to
       * deal with directly.  Break them down into their component parts
       * and put the more basic elements by recursively calling
       * MPI_Pack_buffer().  Note that is allows complex structures like
       * an MPIR_STRUCT inside an MPIR_VECTOR, etc.
       */
      case MPIR_CONTIG:
/* globus_nexus_printf("NICK: MPID_Unpack_partial_buffer(): case MPIR_CONTIG:\n"); */
/* globus_nexus_printf("NICK: MPID_Unpack_partial_buffer(): case MPIR_CONTIG: before call to MPID_Unpack_partial_buffer()\n"); */
	MPID_Unpack_partial_buffer(dest,
			       recvd_nbytes,
			       req_nelem * req_datatype->count,
			       dataorigin_format,
			       dataorigin_remaining_nbytes,
			       req_datatype->old_type,
			       recv_buf,
			       done);
/* globus_nexus_printf("NICK: MPID_Unpack_partial_buffer(): case MPIR_CONTIG: after call to MPID_Unpack_partial_buffer()\n"); */
	break;
      case MPIR_VECTOR:
      case MPIR_HVECTOR:
	tmp_buf = dest;
	for (i = 0; !(*done) && i < req_nelem; i++)
	{
	    dest = tmp_buf;
	    for (j = 0; !(*done) && j < req_datatype->count; j++)
	    {
		MPID_Unpack_partial_buffer(dest,
				       recvd_nbytes,
				       req_datatype->blocklen,
				       dataorigin_format,
				       dataorigin_remaining_nbytes,
				       req_datatype->old_type,
				       recv_buf,
				       done);
		void_pointer_displace(dest, req_datatype->stride);
	    } /* endfor */
	    void_pointer_displace(tmp_buf, req_datatype->extent);
	} /* endfor */
	break;
      case MPIR_INDEXED:
      case MPIR_HINDEXED:
	for (i = 0; !(*done) && i < req_nelem; i ++)
	{
	    /* tmp_buf = (char *)dest + req_datatype->indices[j]; */
	    for (j = 0; !(*done) && j < req_datatype->count; j ++)
	    {
		tmp_buf = (char *)dest + req_datatype->indices[j];
		MPID_Unpack_partial_buffer(tmp_buf,
					recvd_nbytes,
					req_datatype->blocklens[j],
					dataorigin_format,
					dataorigin_remaining_nbytes,
					req_datatype->old_types[j],
					recv_buf,
					done);
	    } /* endfor */
	    void_pointer_displace(dest, req_datatype->extent);
	} /* endfor */
	break;
      case MPIR_STRUCT:
/* globus_nexus_printf("NICK: MPID_Unpack_partial_buffer(): case MPIR_STRUCT:\n"); */
	for (i = 0; !(*done) && i < req_nelem; i ++)
	{
/* globus_nexus_printf("NICK: MPID_Unpack_partial_buffer(): case MPIR_STRUCT: top of outer loop i %d to req_nelem %d\n", i, req_nelem); */
	    /* tmp_buf = (char *)dest + req_datatype->indices[j]; */
	    for (j = 0; !(*done) && j < req_datatype->count; j ++)
	    {
		tmp_buf = (char *)dest + req_datatype->indices[j];
/* globus_nexus_printf("NICK: MPID_Unpack_partial_buffer(): case MPIR_STRUCT: before call to MPID_Unpack_partial_buffer() j %d req_datatype->count %d\n", j, req_datatype->count); */
		MPID_Unpack_partial_buffer(tmp_buf,
					recvd_nbytes,
					req_datatype->blocklens[j],
					dataorigin_format,
					dataorigin_remaining_nbytes,
					req_datatype->old_types[j],
					recv_buf,
					done);
/* globus_nexus_printf("NICK: MPID_Unpack_partial_buffer(): case MPIR_STRUCT: after call to MPID_Unpack_partial_buffer() j %d req_datatype->count %d\n", j, req_datatype->count); */
	    } /* endfor */
	    void_pointer_displace(dest, req_datatype->extent);
	} /* endfor */
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
      /* case MPIR_LOGICAL: */
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
/* globus_nexus_printf("NICK: exit MPID_Unpack_partial_buffer() into dest %x req_datatype->dte_type %d (MPIR_DOUBLE = %d)\n", dest, req_datatype->dte_type, MPIR_DOUBLE); */
} /* end MPID_Unpack_partial_buffer() */

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

/* globus_nexus_printf("NICK: enter MPID_Pack(): src %x count %d datatype->dte_type %d dest %x maxcount %d position %d\n", src, count, datatype->dte_type, dest, maxcount, *position); */
    switch (datatype->dte_type)
    {
      /* 
       * These are basic datatypes that Nexus directly knows how to
       * deal with.  Just put them into the send buffer and return.
       */
      case MPIR_CHAR:
	globus_nexus_user_put_char(&dest_end, (char *)src, count);
	*position += dest_end - dest_start;
	break;
      case MPIR_UCHAR:
	globus_nexus_user_put_u_char(&dest_end, (u_char *)src, count);
	*position += dest_end - dest_start;
	break;
      case MPIR_PACKED:
	globus_nexus_warning("You shouldn't be packing a packed datatype\n");
      case MPIR_BYTE:
	globus_nexus_user_put_byte(&dest_end, (nexus_byte_t *)src, count);
	*position += dest_end - dest_start;
	break;
      case MPIR_SHORT:
	globus_nexus_user_put_short(&dest_end, (short *)src, count);
	*position += dest_end - dest_start;
	break;
      case MPIR_USHORT:
	globus_nexus_user_put_u_short(&dest_end, (u_short *)src, count);
	*position += dest_end - dest_start;
	break;
      case MPIR_INT:
      case MPIR_LOGICAL: /* 'logical' in FORTRAN is always same as 'int' */
	globus_nexus_user_put_int(&dest_end, (int *)src, count);
	*position += dest_end - dest_start;
	break;
      case MPIR_UINT:
	globus_nexus_user_put_u_int(&dest_end, (u_int *)src, count);
	*position += dest_end - dest_start;
	break;
      case MPIR_LONG:
	globus_nexus_user_put_long(&dest_end, (long *)src, count);
	*position += dest_end - dest_start;
	break;
      case MPIR_ULONG:
	globus_nexus_user_put_u_long(&dest_end, (u_long *)src, count);
	*position += dest_end - dest_start;
	break;
      case MPIR_FLOAT:
	globus_nexus_user_put_float(&dest_end, (float *)src, count);
	*position += dest_end - dest_start;
	break;
      case MPIR_COMPLEX:
	globus_nexus_user_put_float(&dest_end, (float *)src, 2 * count);
	*position += dest_end - dest_start;
	break;
      case MPIR_DOUBLE:
	globus_nexus_user_put_double(&dest_end, (double *)src, count);
	*position += dest_end - dest_start;
	break;
      case MPIR_DOUBLE_COMPLEX:
	globus_nexus_user_put_double(&dest_end, (double *)src, 2 * count);
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
      /* case MPIR_LOGICAL: */
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
/* globus_nexus_printf("NICK: exit MPID_Pack(): src %x count %d datatype->dte_type %d dest %x maxcount %d position %d\n", src, count, datatype->dte_type, dest, maxcount, *position); */
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

#if 0
{
int i;
char pbuff[100];

for (i = 0; i < 10 && i < maxcount; i ++) sprintf(pbuff+(2*i), "%2x", *(((char *) src)+i));
pbuff[2*i] = '\0';

globus_nexus_printf("NICK: enter MPID_Unpack(): src %x up to first 10 bytes >%s< maxcount %d in_pos %d dest %x out_pos %d datatype->dte_type %d msgrep %d (should be MPID_Msgrep_from_comm(comm) = %d which should be = NEXUS_DC_FORMAT_LOCAL %d\n", src, pbuff, maxcount, *in_position, dest, *out_position, datatype->dte_type, msgrep, MPID_Msgrep_from_comm(comm), NEXUS_DC_FORMAT_LOCAL);
}
#endif

    dest_start = dest_end = (nexus_byte_t *)dest + *out_position;
    src_start = src_end = (nexus_byte_t *)src + *in_position;

/* globus_nexus_printf("NICK: MPID_Unpack(): dest_start = dest_end = %x src_start = src_end = %x\n", dest_start, src_start); */

    switch (datatype->dte_type)
    {
      /* 
       * These are basic datatypes that Nexus directly knows how to
       * deal with.  Just put them into the send buffer and return.
       */
      case MPIR_CHAR:
	globus_nexus_user_get_char(&src_end,
			    (char *)dest_start,
			    (unsigned long) count,
			    msgrep);
	dest_end = (nexus_byte_t *)((char *)dest_start + count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_UCHAR:
	globus_nexus_user_get_u_char(&src_end,
			      (u_char *)dest_start,
			      (unsigned long) count,
			      msgrep);
	dest_end = (nexus_byte_t *)((u_char *)dest_start + count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_PACKED:
	/* globus_nexus_warning("You shouldn't be packing a packed datatype\n"); */
      case MPIR_BYTE:
	globus_nexus_user_get_byte(&src_end, dest_start, (unsigned long) count, msgrep);
	dest_end = dest_start + count;
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_SHORT:
#if 0
{
int i;
char pbuff[20];

for (i = 0; i < 4; i ++) sprintf(pbuff+(2*i), "%2x", *(((char *) src_end)+i));
pbuff[2*i] = '\0';

globus_nexus_printf("NICK: MPID_Unpack(): case MPIR_SHORT: before globus_nexus_user_get_short() dest_start %x (%d) src_end %x first four bytes >%s< count %d\n", dest_start, *((short *) dest_start), src_end, pbuff, count);
}
#endif
	globus_nexus_user_get_short(&src_end,
			     (short *)dest_start,
			     (unsigned long) count,
			     msgrep);
#if 0
{
int i;
char pbuff[20];

for (i = 0; i < 4; i ++) sprintf(pbuff+(2*i), "%2x", *(((char *) dest_start)+i));
pbuff[2*i] = '\0';

globus_nexus_printf("NICK: MPID_Unpack(): case MPIR_SHORT: after globus_nexus_user_get_short() dest_start %x (%d) first four bytes of dest >%s< src_end %x count %d\n", dest_start, *((short *) dest_start), pbuff, src_end, count);
}
#endif
	dest_end = (nexus_byte_t *)((short *)dest_start + count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_USHORT:
	globus_nexus_user_get_u_short(&src_end,
			       (u_short *)dest_start,
			       (unsigned long) count,
			       msgrep);
	dest_end = (nexus_byte_t *)((u_short *)dest_start + count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_INT:
      case MPIR_LOGICAL: /* 'logical' in FORTRAN is always same as 'int' */
	globus_nexus_user_get_int(&src_end,
			   (int *)dest_start,
			   (unsigned long) count,
			   msgrep);
	dest_end = (nexus_byte_t *)((int *)dest_start + count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_UINT:
	globus_nexus_user_get_u_int(&src_end,
			     (u_int *)dest_start,
			     (unsigned long) count,
			     msgrep);
	dest_end = (nexus_byte_t *)((u_int *)dest_start + count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_LONG:
	globus_nexus_user_get_long(&src_end,
			    (long *)dest_start,
			    (unsigned long) count,
			    msgrep);
	dest_end = (nexus_byte_t *)((long *)dest_start + count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_ULONG:
	globus_nexus_user_get_u_long(&src_end,
			      (u_long *)dest_start,
			      (unsigned long) count,
			      msgrep);
	dest_end = (nexus_byte_t *)((u_long *)dest_start + count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_FLOAT:
	globus_nexus_user_get_float(&src_end,
			     (float *)dest_start,
			     (unsigned long) count,
			     msgrep);
	dest_end = (nexus_byte_t *)((float *)dest_start + count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_COMPLEX:
	globus_nexus_user_get_float(&src_end,
			     (float *)dest_start,
			     (unsigned long) (2 * count),
			     msgrep);
	dest_end = (nexus_byte_t *)((float *)dest_start + 2 * count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_DOUBLE:
#if 0
{
int i;
char pbuff[20];

for (i = 0; i < 8; i ++) sprintf(pbuff+(2*i), "%2x", *(((char *) src_end)+i));
pbuff[2*i] = '\0';

/* globus_nexus_printf("NICK: MPID_Unpack(): case MPIR_DOUBLE: before globus_nexus_user_get_double() dest_start %x (%f) src_end %x >%s< count %d\n", dest_start, *((double *) dest_start), src_end, pbuff, count); */
}
#endif
	globus_nexus_user_get_double(&src_end,
			      (double *)dest_start,
			      (unsigned long) count,
			      msgrep);
/* globus_nexus_printf("NICK: MPID_Unpack(): case MPIR_DOUBLE: after globus_nexus_user_get_double() dest_start %x (%f) src_end %x count %d\n", dest_start, *((double *) dest_start), src_end, count); */
	dest_end = (nexus_byte_t *)((double *)dest_start + count);
	*out_position += dest_end - dest_start;
	*in_position += src_end - src_start;
	break;
      case MPIR_DOUBLE_COMPLEX:
	globus_nexus_user_get_double(&src_end,
			      (double *)dest_start,
			      (unsigned long) (2 * count),
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
      /* case MPIR_LOGICAL: */
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
#if 0
{
int i;
char pbuff[100];
char pbuff2[100];

for (i = 0; i< 10 && i < maxcount; i ++) sprintf(pbuff+(2*i), "%2x", *(((char *) src)+i));
pbuff[2*i] = '\0';
for (i = 0; i< 10 && i < maxcount; i ++) sprintf(pbuff2+(2*i), "%2x", *(((char *) dest)+i));
pbuff2[2*i] = '\0';

globus_nexus_printf("NICK: exit MPID_Unpack(): src %x up to first 10 bytes >%s< dest %x up to first 10 bytes >%s< maxcount %d in_pos %d dest %x datatype->dte_type %d msgrep %d\n", src, pbuff, dest, pbuff2, maxcount, *in_position, dest, datatype->dte_type, msgrep);
}
#endif
}

void MPID_extract_data(MPIR_RHANDLE *rhandle,
			globus_nexus_buffer_t *source_buffer,
			int req_nelem,
			struct MPIR_DATATYPE *req_datatype,
			void *dest_buffer,
    	    	    	int *error_code, /* optional */
			int *recvd_nbytes)
{

int dataorigin_maxnbytes;
int dataorigin_unitnbytes;
int count_of_completed_elements;
int dataorigin_partial_data_nbytes;
int dummy;
globus_bool_t done;

/* globus_nexus_printf("NICK: enter MPID_extract_data() rhandle->dataorigin_nonpacksize %d req_nelem %d type %d\n", rhandle->dataorigin_nonpacksize, req_nelem, req_datatype->dte_type); */
    /* determining if recv buffer is big enough for incoming message */
    dataorigin_maxnbytes = 0;
    dummy = 0;
    MPID_Pack_exact_size(req_nelem,
			req_datatype,
			rhandle->dataorigin_format,
			&dummy, /* local nbytes ... ignored */
			&dataorigin_maxnbytes);

/* globus_nexus_printf("NICK: MPID_extract_data(): calcuated dataorigin_maxnbytes %d\n", dataorigin_maxnbytes); */
    if (rhandle->dataorigin_nonpacksize > dataorigin_maxnbytes)
    {
/* globus_nexus_printf("NICK: MPID_extract_data(): recv buffer is NOT big enough\n"); */
	/* recv buffer is NOT big enough */

	*recvd_nbytes = 0;

/* #ifdef DEBUG */
/* globus_nexus_printf("MPID_extract_data(): TRUNCATION DETECTED: incoming msg %d bytes > recv buffer %d bytes (all byte counts in dataorigin format)\n", rhandle->dataorigin_nonpacksize, dataorigin_maxnbytes); */
/* #endif */
	rhandle->s.MPI_ERROR = MPI_ERR_TRUNCATE;
    	if (error_code)
    	    *error_code = MPI_ERR_TRUNCATE;
    }
    else
    {
	/* recv buffer is big enough */
/* globus_nexus_printf("NICK: MPID_extract_data(): recv buffer is big enough\n"); */

	if (req_nelem > 0)
	{
	    /* RECV asked for nelem > 0 */
/* #ifdef DEBUG */
	    if (dataorigin_maxnbytes % req_nelem)
	    {
		globus_fatal("MPID_extract_data(): dataorigin_maxnbytes %d is not a multiple of req_nelem %d\n", dataorigin_maxnbytes, req_nelem);
	    } /* endif */
/* #endif */

	    dataorigin_unitnbytes = dataorigin_maxnbytes/req_nelem;
/* globus_nexus_printf("NICK: MPID_extract_data(): just calculated dataorigin_unitnbytes %d\n", dataorigin_unitnbytes); */

	    /* extracting all complete data elements first */
	    *recvd_nbytes = 0;
	    if (count_of_completed_elements =
		    rhandle->dataorigin_nonpacksize / dataorigin_unitnbytes)
	    {
		/* there are some complete data elements */
/* globus_nexus_printf("NICK: MPID_extract_data(): there are some complete data elements\n"); */

		/* extracting complete data elements only */
		MPID_Unpack_buffer(dest_buffer,
				count_of_completed_elements,
				req_datatype,
				source_buffer);

		/* accumulating nbytes we just received into 'recvd_nbytes' */
		dummy = 0;
		MPID_Pack_exact_size(count_of_completed_elements,
				    req_datatype,
				    NEXUS_DC_FORMAT_LOCAL,
				    &dummy,                        /* ignored */
				    recvd_nbytes);

	    } /* endif */

	    if (dataorigin_partial_data_nbytes = 
		    rhandle->dataorigin_nonpacksize % dataorigin_unitnbytes)
	    {
/* globus_nexus_printf("NICK: MPID_extract_data(): there are some partial data elements\n"); */
		/* incoming buffer is NOT a multiple of requested datatype */
		/* ... this means that the last data element (typically a  */
		/* user-defined data structure) has data missing from the  */
		/* end.  This is OK according to the MPI standard.         */

		void_pointer_displace(dest_buffer, *recvd_nbytes);

/* globus_nexus_printf("NICK: MPID_extract_data(): before call to MPID_Unpack_partial_buffer()\n"); */
		done = GLOBUS_FALSE;
		MPID_Unpack_partial_buffer(dest_buffer,
					recvd_nbytes,
					1,
					rhandle->dataorigin_format,
					&dataorigin_partial_data_nbytes,
					req_datatype,
					source_buffer,
					&done);
/* globus_nexus_printf("NICK: MPID_extract_data(): after call to MPID_Unpack_partial_buffer()\n"); */
	    } /* endif */
	}
	else
	{
	    /* RECV asked for nelem = 0 */
	    *recvd_nbytes = 0;
	} /* endif */
    } /* endif */
/* globus_nexus_printf("NICK: exit MPID_extract_data() *recvd_nbytes %d\n", *recvd_nbytes); */

} /* end MPID_extract_data() */

