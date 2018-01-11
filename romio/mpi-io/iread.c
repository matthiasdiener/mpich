/* 
 *   $Id: iread.c,v 1.2 1998/06/02 19:02:14 thakur Exp $    
 *
 *   Copyright (C) 1997 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "mpioimpl.h"

/*@
    MPI_File_iread - Nonblocking read using individual file pointer

Input Parameters:
. fh - file handle (handle)
. count - number of elements in buffer (nonnegative integer)
. datatype - datatype of each buffer element (handle)

Output Parameters:
. buf - initial address of buffer (choice)
. request - request object (handle)

.N fortran
@*/
int MPI_File_iread(MPI_File fh, void *buf, int count, 
                   MPI_Datatype datatype, MPIO_Request *request)
{
    int error_code, bufsize, buftype_is_contig, filetype_is_contig;
    int datatype_size;
    ADIO_Offset off;
    ADIO_Status status;
#ifdef MPI_hpux
    int fl_xmpi;

    HPMP_IO_START(fl_xmpi, BLKMPIFILEIREAD, TRDTSYSTEM, fh, datatype, count);
#endif /* MPI_hpux */

    if ((fh <= (MPI_File) 0) || (fh->cookie != ADIOI_FILE_COOKIE)) {
	printf("MPI_File_iread: Invalid file handle\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (buf <= (void *) 0) {
	printf("MPI_File_iread: buf is not a valid address\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (count < 0) {
	printf("MPI_File_iread: Invalid count argument\n");
	MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (datatype == MPI_DATATYPE_NULL) {
        printf("MPI_File_iread: Invalid datatype\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Type_size(datatype, &datatype_size);

    if ((count*datatype_size) % fh->etype_size != 0) {
        printf("MPI_File_iread: Only an integral number of etypes can be accessed\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    ADIOI_Datatype_iscontig(datatype, &buftype_is_contig);
    ADIOI_Datatype_iscontig(fh->filetype, &filetype_is_contig);

    /* contiguous or strided? */

    if (buftype_is_contig && filetype_is_contig) {
    /* convert count and offset to bytes */
	bufsize = datatype_size * count;
	if (!(fh->atomicity))
	    ADIO_IreadContig(fh, buf, bufsize, ADIO_INDIVIDUAL, 0,
			request, &error_code);
	else {
	    /* to maintain strict atomicity semantics with other concurrent
              operations, lock (exclusive) and call blocking routine */

	    *request = ADIOI_Malloc_request();
	    (*request)->optype = ADIOI_READ;
	    (*request)->fd = fh;
	    (*request)->next = ADIO_REQUEST_NULL;
	    (*request)->queued = 0;
	    (*request)->handle = 0;
	    
	    off = fh->fp_ind;
	    if ((fh->file_system != ADIO_PIOFS) && 
		(fh->file_system != ADIO_NFS))
		ADIOI_WRITE_LOCK(fh, off, SEEK_SET, bufsize);
		
	    ADIO_ReadContig(fh, buf, bufsize, ADIO_INDIVIDUAL, 0, &status,
                    &error_code);  

	    if ((fh->file_system != ADIO_PIOFS) && 
		(fh->file_system != ADIO_NFS))
		ADIOI_UNLOCK(fh, off, SEEK_SET, bufsize);

	    fh->async_count++;
	    /* status info. must be linked to the request structure, so that it
	       can be accessed later from a wait */
	}
    }
    else ADIO_IreadStrided(fh, buf, count, datatype, ADIO_INDIVIDUAL,
			  0, request, &error_code); 
    
#ifdef MPI_hpux
    HPMP_IO_END(fl_xmpi, fh, datatype, count);
#endif /* MPI_hpux */
    return error_code;
}
