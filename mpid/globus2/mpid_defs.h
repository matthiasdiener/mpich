#if !defined(MPID_DEFS_H)
#define MPID_DEFS_H 1


/* SEBASTIEN L: experiments */
#include "global_c_symb.h"
/* END EXPERIMENTS */


/*
 * This header file converts all MPI_ names into MPQ_ names, so that we avoid
 * name clashing when using the vendor's MPI library.
 *
 * Based on a C hack by Warren Smith, extended to Fortran by Olle Larsson,
 * updated and integrated in the MPICH distribution by Nick Karonis and
 * Brian Toonen.
 *
 */

#define MPICH_RENAMING_MPI_FUNCS
#define MPICH_SR_PACKED_INTRINSIC_UNSUPPORTED

#define MPI_Abort MPQ_Abort
#define PMPI_Abort PMPQ_Abort

#define MPI_Add_error_class MPQ_Add_error_class
#define PMPI_Add_error_class PMPQ_Add_error_class

#define MPI_Address MPQ_Address
#define PMPI_Address PMPQ_Address

#define MPI_Allgather MPQ_Allgather
#define PMPI_Allgather PMPQ_Allgather

#define MPI_Allgatherv MPQ_Allgatherv
#define PMPI_Allgatherv PMPQ_Allgatherv

#define MPI_Alloc_mem MPQ_Alloc_mem
#define PMPI_Alloc_mem PMPQ_Alloc_mem

#define MPI_Allreduce MPQ_Allreduce
#define PMPI_Allreduce PMPQ_Allreduce

#define MPI_Alltoall MPQ_Alltoall
#define PMPI_Alltoall PMPQ_Alltoall

#define MPI_Alltoallv MPQ_Alltoallv
#define PMPI_Alltoallv PMPQ_Alltoallv

#define MPI_Attr_delete MPQ_Attr_delete
#define PMPI_Attr_delete PMPQ_Attr_delete

#define MPI_Attr_get MPQ_Attr_get
#define PMPI_Attr_get PMPQ_Attr_get

#define MPI_Attr_put MPQ_Attr_put
#define PMPI_Attr_put PMPQ_Attr_put

#define MPI_Barrier MPQ_Barrier
#define PMPI_Barrier PMPQ_Barrier

#define MPI_Bcast MPQ_Bcast
#define PMPI_Bcast PMPQ_Bcast

#define MPI_Bsend MPQ_Bsend
#define PMPI_Bsend PMPQ_Bsend

#define MPI_Bsend_init MPQ_Bsend_init
#define PMPI_Bsend_init PMPQ_Bsend_init

#define MPI_Buffer_attach MPQ_Buffer_attach
#define PMPI_Buffer_attach PMPQ_Buffer_attach

#define MPI_Buffer_detach MPQ_Buffer_detach
#define PMPI_Buffer_detach PMPQ_Buffer_detach

#define MPI_Cancel MPQ_Cancel
#define PMPI_Cancel PMPQ_Cancel

#define MPI_Cart_coords MPQ_Cart_coords
#define PMPI_Cart_coords PMPQ_Cart_coords

#define MPI_Cart_create MPQ_Cart_create
#define PMPI_Cart_create PMPQ_Cart_create

#define MPI_Cart_get MPQ_Cart_get
#define PMPI_Cart_get PMPQ_Cart_get

#define MPI_Cart_map MPQ_Cart_map
#define PMPI_Cart_map PMPQ_Cart_map

#define MPI_Cart_rank MPQ_Cart_rank
#define PMPI_Cart_rank PMPQ_Cart_rank

#define MPI_Cart_shift MPQ_Cart_shift
#define PMPI_Cart_shift PMPQ_Cart_shift

#define MPI_Cart_sub MPQ_Cart_sub
#define PMPI_Cart_sub PMPQ_Cart_sub

#define MPI_Cartdim_get MPQ_Cartdim_get
#define PMPI_Cartdim_get PMPQ_Cartdim_get

#define MPI_Close_port MPQ_Close_port
#define PMPI_Close_port PMPQ_Close_port

#define MPI_Comm_call_errhandler MPQ_Comm_call_errhandler
#define PMPI_Comm_call_errhandler PMPQ_Comm_call_errhandler

#define MPI_Comm_clone MPQ_Comm_clone
#define PMPI_Comm_clone PMPQ_Comm_clone

#define MPI_Comm_compare MPQ_Comm_compare
#define PMPI_Comm_compare PMPQ_Comm_compare

#define MPI_Comm_connect MPQ_Comm_connect
#define PMPI_Comm_connect PMPQ_Comm_connect

#define MPI_Comm_create MPQ_Comm_create
#define PMPI_Comm_create PMPQ_Comm_create

#define MPI_Comm_create_errhandler MPQ_Comm_create_errhandler
#define PMPI_Comm_create_errhandler PMPQ_Comm_create_errhandler

#define MPI_Comm_create_keyval MPQ_Comm_create_keyval
#define PMPI_Comm_create_keyval PMPQ_Comm_create_keyval

#define MPI_Comm_disconnect MPQ_Comm_disconnect
#define PMPI_Comm_disconnect PMPQ_Comm_disconnect

#define MPI_Comm_dup MPQ_Comm_dup
#define PMPI_Comm_dup PMPQ_Comm_dup

#define MPI_Comm_free MPQ_Comm_free
#define PMPI_Comm_free PMPQ_Comm_free

#define MPI_Comm_free_keyval MPQ_Comm_free_keyval
#define PMPI_Comm_free_keyval PMPQ_Comm_free_keyval

#define MPI_Comm_get_errhandler MPQ_Comm_get_errhandler
#define PMPI_Comm_get_errhandler PMPQ_Comm_get_errhandler

#define MPI_Comm_get_name MPQ_Comm_get_name
#define PMPI_Comm_get_name PMPQ_Comm_get_name

#define MPI_Comm_get_parent MPQ_Comm_get_parent
#define PMPI_Comm_get_parent PMPQ_Comm_get_parent

#define MPI_Comm_group MPQ_Comm_group
#define PMPI_Comm_group PMPQ_Comm_group

#define MPI_Comm_join MPQ_Comm_join
#define PMPI_Comm_join PMPQ_Comm_join

#define MPI_Comm_rank MPQ_Comm_rank
#define PMPI_Comm_rank PMPQ_Comm_rank

#define MPI_Comm_remote_group MPQ_Comm_remote_group
#define PMPI_Comm_remote_group PMPQ_Comm_remote_group

#define MPI_Comm_remote_size MPQ_Comm_remote_size
#define PMPI_Comm_remote_size PMPQ_Comm_remote_size

#define MPI_Comm_set_errhandler MPQ_Comm_set_errhandler
#define PMPI_Comm_set_errhandler PMPQ_Comm_set_errhandler

#define MPI_Comm_set_name MPQ_Comm_set_name
#define PMPI_Comm_set_name PMPQ_Comm_set_name

#define MPI_Comm_size MPQ_Comm_size
#define PMPI_Comm_size PMPQ_Comm_size

#define MPI_Comm_spawn MPQ_Comm_spawn
#define PMPI_Comm_spawn PMPQ_Comm_spawn

#define MPI_Comm_spawn_multiple MPQ_Comm_spawn_multiple
#define PMPI_Comm_spawn_multiple PMPQ_Comm_spawn_multiple

#define MPI_Comm_split MPQ_Comm_split
#define PMPI_Comm_split PMPQ_Comm_split

#define MPI_Comm_test_inter MPQ_Comm_test_inter
#define PMPI_Comm_test_inter PMPQ_Comm_test_inter

#define MPI_DOUBLE_INT_var MPQ_DOUBLE_INT_var
#define PMPI_DOUBLE_INT_var PMPQ_DOUBLE_INT_var

#define MPI_Dims_create MPQ_Dims_create
#define PMPI_Dims_create PMPQ_Dims_create

#define MPI_Errhandler_create MPQ_Errhandler_create
#define PMPI_Errhandler_create PMPQ_Errhandler_create

#define MPI_Errhandler_free MPQ_Errhandler_free
#define PMPI_Errhandler_free PMPQ_Errhandler_free

#define MPI_Errhandler_get MPQ_Errhandler_get
#define PMPI_Errhandler_get PMPQ_Errhandler_get

#define MPI_Errhandler_set MPQ_Errhandler_set
#define PMPI_Errhandler_set PMPQ_Errhandler_set

#define MPI_Error_class MPQ_Error_class
#define PMPI_Error_class PMPQ_Error_class

#define MPI_Error_string MPQ_Error_string
#define PMPI_Error_string PMPQ_Error_string

#define MPI_Exscan MPQ_Exscan
#define PMPI_Exscan PMPQ_Exscan

#define MPI_FLOAT_INT_var MPQ_FLOAT_INT_var
#define PMPI_FLOAT_INT_var PMPQ_FLOAT_INT_var

#define MPI_File_c2f MPQ_File_c2f
#define PMPI_File_c2f PMPQ_File_c2f

#define MPI_File_call_errhandler MPQ_File_call_errhandler
#define PMPI_File_call_errhandler PMPQ_File_call_errhandler

#define MPI_File_close MPQ_File_close
#define PMPI_File_close PMPQ_File_close

#define MPI_File_create_errhandler MPQ_File_create_errhandler
#define PMPI_File_create_errhandler PMPQ_File_create_errhandler

#define MPI_File_delete MPQ_File_delete
#define PMPI_File_delete PMPQ_File_delete

#define MPI_File_f2c MPQ_File_f2c
#define PMPI_File_f2c PMPQ_File_f2c

#define MPI_File_get_amode MPQ_File_get_amode
#define PMPI_File_get_amode PMPQ_File_get_amode

#define MPI_File_get_atomicity MPQ_File_get_atomicity
#define PMPI_File_get_atomicity PMPQ_File_get_atomicity

#define MPI_File_get_byte_offset MPQ_File_get_byte_offset
#define PMPI_File_get_byte_offset PMPQ_File_get_byte_offset

#define MPI_File_get_errhandler MPQ_File_get_errhandler
#define PMPI_File_get_errhandler PMPQ_File_get_errhandler

#define MPI_File_get_group MPQ_File_get_group
#define PMPI_File_get_group PMPQ_File_get_group

#define MPI_File_get_info MPQ_File_get_info
#define PMPI_File_get_info PMPQ_File_get_info

#define MPI_File_get_position MPQ_File_get_position
#define PMPI_File_get_position PMPQ_File_get_position

#define MPI_File_get_position_shared MPQ_File_get_position_shared
#define PMPI_File_get_position_shared PMPQ_File_get_position_shared

#define MPI_File_get_size MPQ_File_get_size
#define PMPI_File_get_size PMPQ_File_get_size

#define MPI_File_get_type_extent MPQ_File_get_type_extent
#define PMPI_File_get_type_extent PMPQ_File_get_type_extent

#define MPI_File_get_view MPQ_File_get_view
#define PMPI_File_get_view PMPQ_File_get_view

#define MPI_File_iread MPQ_File_iread
#define PMPI_File_iread PMPQ_File_iread

#define MPI_File_iread_at MPQ_File_iread_at
#define PMPI_File_iread_at PMPQ_File_iread_at

#define MPI_File_iread_shared MPQ_File_iread_shared
#define PMPI_File_iread_shared PMPQ_File_iread_shared

#define MPI_File_iwrite MPQ_File_iwrite
#define PMPI_File_iwrite PMPQ_File_iwrite

#define MPI_File_iwrite_at MPQ_File_iwrite_at
#define PMPI_File_iwrite_at PMPQ_File_iwrite_at

#define MPI_File_iwrite_shared MPQ_File_iwrite_shared
#define PMPI_File_iwrite_shared PMPQ_File_iwrite_shared

#define MPI_File_open MPQ_File_open
#define PMPI_File_open PMPQ_File_open

#define MPI_File_preallocate MPQ_File_preallocate
#define PMPI_File_preallocate PMPQ_File_preallocate

#define MPI_File_read MPQ_File_read
#define PMPI_File_read PMPQ_File_read

#define MPI_File_read_all MPQ_File_read_all 
#define PMPI_File_read_all PMPQ_File_read_all

#define MPI_File_read_all_begin MPQ_File_read_all_begin
#define PMPI_File_read_all_begin PMPQ_File_read_all_begin

#define MPI_File_read_all_end MPQ_File_read_all_end
#define PMPI_File_read_all_end PMPQ_File_read_all_end

#define MPI_File_read_at MPQ_File_read_at
#define PMPI_File_read_at PMPQ_File_read_at

#define MPI_File_read_at_all MPQ_File_read_at_all
#define PMPI_File_read_at_all PMPQ_File_read_at_all

#define MPI_File_read_at_all_begin MPQ_File_read_at_all_begin
#define PMPI_File_read_at_all_begin PMPQ_File_read_at_all_begin

#define MPI_File_read_at_all_end MPQ_File_read_at_all_end
#define PMPI_File_read_at_all_end PMPQ_File_read_at_all_end

#define MPI_File_read_ordered MPQ_File_read_ordered
#define PMPI_File_read_ordered PMPQ_File_read_ordered

#define MPI_File_read_ordered_begin MPQ_File_read_ordered_begin
#define PMPI_File_read_ordered_begin PMPQ_File_read_ordered_begin

#define MPI_File_read_ordered_end MPQ_File_read_ordered_end
#define PMPI_File_read_ordered_end PMPQ_File_read_ordered_end

#define MPI_File_read_shared MPQ_File_read_shared
#define PMPI_File_read_shared PMPQ_File_read_shared

#define MPI_File_seek MPQ_File_seek
#define PMPI_File_seek PMPQ_File_seek

#define MPI_File_seek_shared MPQ_File_seek_shared
#define PMPI_File_seek_shared PMPQ_File_seek_shared

#define MPI_File_set_atomicity MPQ_File_set_atomicity
#define PMPI_File_set_atomicity PMPQ_File_set_atomicity

#define MPI_File_set_errhandler MPQ_File_set_errhandler
#define PMPI_File_set_errhandler PMPQ_File_set_errhandler

#define MPI_File_set_info MPQ_File_set_info
#define PMPI_File_set_info PMPQ_File_set_info

#define MPI_File_set_size MPQ_File_set_size
#define PMPI_File_set_size PMPQ_File_set_size

#define MPI_File_set_view MPQ_File_set_view
#define PMPI_File_set_view PMPQ_File_set_view

#define MPI_File_sync MPQ_File_sync
#define PMPI_File_sync PMPQ_File_sync

#define MPI_File_write MPQ_File_write
#define PMPI_File_write PMPQ_File_write

#define MPI_File_write_all MPQ_File_write_all
#define PMPI_File_write_all PMPQ_File_write_all

#define MPI_File_write_all_begin MPQ_File_write_all_begin
#define PMPI_File_write_all_begin PMPQ_File_write_all_begin

#define MPI_File_write_all_end MPQ_File_write_all_end
#define PMPI_File_write_all_end PMPQ_File_write_all_end

#define MPI_File_write_at MPQ_File_write_at
#define PMPI_File_write_at PMPQ_File_write_at

#define MPI_File_write_at_all MPQ_File_write_at_all
#define PMPI_File_write_at_all PMPQ_File_write_at_all

#define MPI_File_write_at_all_begin MPQ_File_write_at_all_begin
#define PMPI_File_write_at_all_begin PMPQ_File_write_at_all_begin

#define MPI_File_write_at_all_end MPQ_File_write_at_all_end
#define PMPI_File_write_at_all_end PMPQ_File_write_at_all_end

#define MPI_File_write_ordered MPQ_File_write_ordered
#define PMPI_File_write_ordered PMPQ_File_write_ordered

#define MPI_File_write_ordered_begin MPQ_File_write_ordered_begin
#define PMPI_File_write_ordered_begin PMPQ_File_write_ordered_begin

#define MPI_File_write_ordered_end MPQ_File_write_ordered_end
#define PMPI_File_write_ordered_end PMPQ_File_write_ordered_end

#define MPI_File_write_shared MPQ_File_write_shared
#define PMPI_File_write_shared PMPQ_File_write_shared

#define MPI_Finalize MPQ_Finalize
#define PMPI_Finalize PMPQ_Finalize

#define MPI_Finalized MPQ_Finalized
#define PMPI_Finalized PMPQ_Finalized

#define MPI_Free_mem MPQ_Free_mem
#define PMPI_Free_mem PMPQ_Free_mem

#define MPI_Gather MPQ_Gather
#define PMPI_Gather PMPQ_Gather

#define MPI_Gatherv MPQ_Gatherv
#define PMPI_Gatherv PMPQ_Gatherv

#define MPI_Get MPQ_Get
#define PMPI_Get PMPQ_Get

#define MPI_Get_address MPQ_Get_address
#define PMPI_Get_address PMPQ_Get_address

#define MPI_Get_count MPQ_Get_count
#define PMPI_Get_count PMPQ_Get_count

#define MPI_Get_elements MPQ_Get_elements
#define PMPI_Get_elements PMPQ_Get_elements

#define MPI_Get_processor_name MPQ_Get_processor_name
#define PMPI_Get_processor_name PMPQ_Get_processor_name

#define MPI_Get_version MPQ_Get_version
#define PMPI_Get_version PMPQ_Get_version

#define MPI_Graph_create MPQ_Graph_create
#define PMPI_Graph_create PMPQ_Graph_create

#define MPI_Graph_get MPQ_Graph_get
#define PMPI_Graph_get PMPQ_Graph_get

#define MPI_Graph_map MPQ_Graph_map
#define PMPI_Graph_map PMPQ_Graph_map

#define MPI_Graph_neighbors MPQ_Graph_neighbors
#define PMPI_Graph_neighbors PMPQ_Graph_neighbors

#define MPI_Graph_neighbors_count MPQ_Graph_neighbors_count
#define PMPI_Graph_neighbors_count PMPQ_Graph_neighbors_count

#define MPI_Graphdims_get MPQ_Graphdims_get
#define PMPI_Graphdims_get PMPQ_Graphdims_get

#define MPI_Grequest_complete MPQ_Grequest_complete
#define PMPI_Grequest_complete PMPQ_Grequest_complete

#define MPI_Group_compare MPQ_Group_compare
#define PMPI_Group_compare PMPQ_Group_compare

#define MPI_Group_difference MPQ_Group_difference
#define PMPI_Group_difference PMPQ_Group_difference

#define MPI_Group_excl MPQ_Group_excl
#define PMPI_Group_excl PMPQ_Group_excl

#define MPI_Group_free MPQ_Group_free
#define PMPI_Group_free PMPQ_Group_free

#define MPI_Group_incl MPQ_Group_incl
#define PMPI_Group_incl PMPQ_Group_incl

#define MPI_Group_intersection MPQ_Group_intersection
#define PMPI_Group_intersection PMPQ_Group_intersection

#define MPI_Group_range_excl MPQ_Group_range_excl
#define PMPI_Group_range_excl PMPQ_Group_range_excl

#define MPI_Group_range_incl MPQ_Group_range_incl
#define PMPI_Group_range_incl PMPQ_Group_range_incl

#define MPI_Group_rank MPQ_Group_rank
#define PMPI_Group_rank PMPQ_Group_rank

#define MPI_Group_size MPQ_Group_size
#define PMPI_Group_size PMPQ_Group_size

#define MPI_Group_translate_ranks MPQ_Group_translate_ranks
#define PMPI_Group_translate_ranks PMPQ_Group_translate_ranks

#define MPI_Group_union MPQ_Group_union
#define PMPI_Group_union PMPQ_Group_union

#define MPI_Ibsend MPQ_Ibsend
#define PMPI_Ibsend PMPQ_Ibsend

#define MPI_Info_c2f MPQ_Info_c2f
#define PMPI_Info_c2f PMPQ_Info_c2f

#define MPI_Info_create MPQ_Info_create
#define PMPI_Info_create PMPQ_Info_create

#define MPI_Info_delete MPQ_Info_delete
#define PMPI_Info_delete PMPQ_Info_delete

#define MPI_Info_dup MPQ_Info_dup
#define PMPI_Info_dup PMPQ_Info_dup

#define MPI_Info_f2c MPQ_Info_f2c
#define PMPI_Info_f2c PMPQ_Info_f2c

#define MPI_Info_free MPQ_Info_free
#define PMPI_Info_free PMPQ_Info_free

#define MPI_Info_get MPQ_Info_get
#define PMPI_Info_get PMPQ_Info_get

#define MPI_Info_get_nkeys MPQ_Info_get_nkeys
#define PMPI_Info_get_nkeys PMPQ_Info_get_nkeys

#define MPI_Info_get_nthkey MPQ_Info_get_nthkey
#define PMPI_Info_get_nthkey PMPQ_Info_get_nthkey

#define MPI_Info_get_valuelen MPQ_Info_get_valuelen
#define PMPI_Info_get_valuelen PMPQ_Info_get_valuelen

#define MPI_Info_set MPQ_Info_set
#define PMPI_Info_set PMPQ_Info_set

#define MPI_Init MPQ_Init
#define PMPI_Init PMPQ_Init

#define MPI_Init_thread MPQ_Init_thread
#define PMPI_Init_thread PMPQ_Init_thread

#define MPI_Initialized MPQ_Initialized
#define PMPI_Initialized PMPQ_Initialized

#define MPI_Intercomm_create MPQ_Intercomm_create
#define PMPI_Intercomm_create PMPQ_Intercomm_create

#define MPI_Intercomm_merge MPQ_Intercomm_merge
#define PMPI_Intercomm_merge PMPQ_Intercomm_merge

#define MPI_Iprobe MPQ_Iprobe
#define PMPI_Iprobe PMPQ_Iprobe

#define MPI_Irecv MPQ_Irecv
#define PMPI_Irecv PMPQ_Irecv

#define MPI_Irsend MPQ_Irsend
#define PMPI_Irsend PMPQ_Irsend

#define MPI_Isend MPQ_Isend
#define PMPI_Isend PMPQ_Isend

#define MPI_Issend MPQ_Issend
#define PMPI_Issend PMPQ_Issend

#define MPI_Keyval_create MPQ_Keyval_create
#define PMPI_Keyval_create PMPQ_Keyval_create

#define MPI_Keyval_free MPQ_Keyval_free
#define PMPI_Keyval_free PMPQ_Keyval_free

#define MPI_LONG_INT_var MPQ_LONG_INT_var
#define PMPI_LONG_INT_var PMPQ_LONG_INT_var

#define MPI_Lookup_name MPQ_Lookup_name
#define PMPI_Lookup_name PMPQ_Lookup_name

#define MPI_Name_get MPQ_Name_get
#define PMPI_Name_get PMPQ_Name_get

#define MPI_Name_put MPQ_Name_put
#define PMPI_Name_put PMPQ_Name_put

#define MPI_Op_create MPQ_Op_create
#define PMPI_Op_create PMPQ_Op_create

#define MPI_Op_free MPQ_Op_free
#define PMPI_Op_free PMPQ_Op_free

#define MPI_Open_port MPQ_Open_port
#define PMPI_Open_port PMPQ_Open_port

#define MPI_Pack MPQ_Pack
#define PMPI_Pack PMPQ_Pack

#define MPI_Pack_external MPQ_Pack_external
#define PMPI_Pack_external PMPQ_Pack_external

#define MPI_Pack_size MPQ_Pack_size
#define PMPI_Pack_size PMPQ_Pack_size

#define MPI_Pcontrol MPQ_Pcontrol
#define PMPI_Pcontrol PMPQ_Pcontrol

#define MPI_Probe MPQ_Probe
#define PMPI_Probe PMPQ_Probe

#define MPI_Publish_name MPQ_Publish_name
#define PMPI_Publish_name PMPQ_Publish_name

#define MPI_Put MPQ_Put
#define PMPI_Put PMPQ_Put

#define MPI_Query_thread MPQ_Query_thread
#define PMPI_Query_thread PMPQ_Query_thread

#define MPI_Recv MPQ_Recv
#define PMPI_Recv PMPQ_Recv

#define MPI_Recv_init MPQ_Recv_init
#define PMPI_Recv_init PMPQ_Recv_init

#define MPI_Reduce MPQ_Reduce
#define PMPI_Reduce PMPQ_Reduce

#define MPI_Reduce_scatter MPQ_Reduce_scatter
#define PMPI_Reduce_scatter PMPQ_Reduce_scatter

#define MPI_Register_datarep MPQ_Register_datarep
#define PMPI_Register_datarep PMPQ_Register_datarep

#define MPI_Request_c2f MPQ_Request_c2f
#define PMPI_Request_c2f PMPQ_Request_c2f

#define MPI_Request_free MPQ_Request_free
#define PMPI_Request_free PMPQ_Request_free

#define MPI_Request_get_status MPQ_Request_get_status
#define PMPI_Request_get_status PMPQ_Request_get_status

#define MPI_Rsend MPQ_Rsend
#define PMPI_Rsend PMPQ_Rsend

#define MPI_Rsend_init MPQ_Rsend_init
#define PMPI_Rsend_init PMPQ_Rsend_init

#define MPI_SHORT_INT_var MPQ_SHORT_INT_var
#define PMPI_SHORT_INT_var PMPQ_SHORT_INT_var

#define MPI_Scan MPQ_Scan
#define PMPI_Scan PMPQ_Scan

#define MPI_Scatter MPQ_Scatter
#define PMPI_Scatter PMPQ_Scatter

#define MPI_Scatterv MPQ_Scatterv
#define PMPI_Scatterv PMPQ_Scatterv

#define MPI_Send MPQ_Send
#define PMPI_Send PMPQ_Send

#define MPI_Send_init MPQ_Send_init
#define PMPI_Send_init PMPQ_Send_init

#define MPI_Sendrecv MPQ_Sendrecv
#define PMPI_Sendrecv PMPQ_Sendrecv

#define MPI_Sendrecv_replace MPQ_Sendrecv_replace
#define PMPI_Sendrecv_replace PMPQ_Sendrecv_replace

#define MPI_Sizeof MPQ_Sizeof
#define PMPI_Sizeof PMPQ_Sizeof

#define MPI_Ssend MPQ_Ssend
#define PMPI_Ssend PMPQ_Ssend

#define MPI_Ssend_init MPQ_Ssend_init
#define PMPI_Ssend_init PMPQ_Ssend_init

#define MPI_Start MPQ_Start
#define PMPI_Start PMPQ_Start

#define MPI_Startall MPQ_Startall
#define PMPI_Startall PMPQ_Startall

#define MPI_Status_c2f MPQ_Status_c2f
#define PMPI_Status_c2f PMPQ_Status_c2f

#define MPI_Status_f2c MPQ_Status_f2c
#define PMPI_Status_f2c PMPQ_Status_f2c

#define MPI_Status_set_cancelled MPQ_Status_set_cancelled
#define PMPI_Status_set_cancelled PMPQ_Status_set_cancelled

#define MPI_Status_set_elements MPQ_Status_set_elements
#define PMPI_Status_set_elements PMPQ_Status_set_elements

#define MPI_Test MPQ_Test
#define PMPI_Test PMPQ_Test

#define MPI_Test_cancelled MPQ_Test_cancelled
#define PMPI_Test_cancelled PMPQ_Test_cancelled

#define MPI_Testall MPQ_Testall
#define PMPI_Testall PMPQ_Testall

#define MPI_Testany MPQ_Testany
#define PMPI_Testany PMPQ_Testany

#define MPI_Testsome MPQ_Testsome
#define PMPI_Testsome PMPQ_Testsome

#define MPI_Topo_status MPQ_Topo_status
#define PMPI_Topo_status PMPQ_Topo_status

#define MPI_Topo_test MPQ_Topo_test
#define PMPI_Topo_test PMPQ_Topo_test

#define MPI_Type_commit MPQ_Type_commit
#define PMPI_Type_commit PMPQ_Type_commit

#define MPI_Type_contiguous MPQ_Type_contiguous
#define PMPI_Type_contiguous PMPQ_Type_contiguous

#define MPI_Type_count MPQ_Type_count
#define PMPI_Type_count PMPQ_Type_count

#define MPI_Type_create_darray MPQ_Type_create_darray
#define PMPI_Type_create_darray PMPQ_Type_create_darray

#define MPI_Type_create_hindexed MPQ_Type_create_hindexed
#define PMPI_Type_create_hindexed PMPQ_Type_create_hindexed

#define MPI_Type_create_indexed_block MPQ_Type_create_indexed_block
#define PMPI_Type_create_indexed_block PMPQ_Type_create_indexed_block

#define MPI_Type_create_keyval MPQ_Type_create_keyval
#define PMPI_Type_create_keyval PMPQ_Type_create_keyval

#define MPI_Type_create_resized MPQ_Type_create_resized
#define PMPI_Type_create_resized PMPQ_Type_create_resized

#define MPI_Type_create_struct MPQ_Type_create_struct
#define PMPI_Type_create_struct PMPQ_Type_create_struct

#define MPI_Type_create_subarray MPQ_Type_create_subarray
#define PMPI_Type_create_subarray PMPQ_Type_create_subarray

#define MPI_Type_delete_attr MPQ_Type_delete_attr
#define PMPI_Type_delete_attr PMPQ_Type_delete_attr

#define MPI_Type_dup MPQ_Type_dup
#define PMPI_Type_dup PMPQ_Type_dup

#define MPI_Type_extent MPQ_Type_extent
#define PMPI_Type_extent PMPQ_Type_extent

#define MPI_Type_free MPQ_Type_free
#define PMPI_Type_free PMPQ_Type_free

#define MPI_Type_free_keyval MPQ_Type_free_keyval
#define PMPI_Type_free_keyval PMPQ_Type_free_keyval

#define MPI_Type_get_cont MPQ_Type_get_cont
#define PMPI_Type_get_cont PMPQ_Type_get_cont

#define MPI_Type_get_contents MPQ_Type_get_contents
#define PMPI_Type_get_contents PMPQ_Type_get_contents

#define MPI_Type_get_env MPQ_Type_get_env
#define PMPI_Type_get_env PMPQ_Type_get_env

#define MPI_Type_get_envelope MPQ_Type_get_envelope
#define PMPI_Type_get_envelope PMPQ_Type_get_envelope

#define MPI_Type_get_extent MPQ_Type_get_extent
#define PMPI_Type_get_extent PMPQ_Type_get_extent

#define MPI_Type_get_name MPQ_Type_get_name
#define PMPI_Type_get_name PMPQ_Type_get_name

#define MPI_Type_get_true_extent MPQ_Type_get_true_extent
#define PMPI_Type_get_true_extent PMPQ_Type_get_true_extent

#define MPI_Type_hindexed MPQ_Type_hindexed
#define PMPI_Type_hindexed PMPQ_Type_hindexed

#define MPI_Type_hvector MPQ_Type_hvector
#define PMPI_Type_hvector PMPQ_Type_hvector

#define MPI_Type_indexed MPQ_Type_indexed
#define PMPI_Type_indexed PMPQ_Type_indexed

#define MPI_Type_lb MPQ_Type_lb
#define PMPI_Type_lb PMPQ_Type_lb

#define MPI_Type_match_size MPQ_Type_match_size
#define PMPI_Type_match_size PMPQ_Type_match_size

#define MPI_Type_set_name MPQ_Type_set_name
#define PMPI_Type_set_name PMPQ_Type_set_name

#define MPI_Type_size MPQ_Type_size
#define PMPI_Type_size PMPQ_Type_size

#define MPI_Type_struct MPQ_Type_struct
#define PMPI_Type_struct PMPQ_Type_struct

#define MPI_Type_ub MPQ_Type_ub
#define PMPI_Type_ub PMPQ_Type_ub

#define MPI_Type_vector MPQ_Type_vector
#define PMPI_Type_vector PMPQ_Type_vector

#define MPI_Unpack MPQ_Unpack
#define PMPI_Unpack PMPQ_Unpack

#define MPI_Unpack_external MPQ_Unpack_external
#define PMPI_Unpack_external PMPQ_Unpack_external

#define MPI_Unpublish_name MPQ_Unpublish_name
#define PMPI_Unpublish_name PMPQ_Unpublish_name

#define MPI_Wait MPQ_Wait
#define PMPI_Wait PMPQ_Wait

#define MPI_Waitall MPQ_Waitall
#define PMPI_Waitall PMPQ_Waitall

#define MPI_Waitany MPQ_Waitany
#define PMPI_Waitany PMPQ_Waitany

#define MPI_Waitsome MPQ_Waitsome
#define PMPI_Waitsome PMPQ_Waitsome

#define MPI_Win_call_errhandler MPQ_Win_call_errhandler
#define PMPI_Win_call_errhandler PMPQ_Win_call_errhandler

#define MPI_Win_complete MPQ_Win_complete
#define PMPI_Win_complete PMPQ_Win_complete

#define MPI_Win_create MPQ_Win_create
#define PMPI_Win_create PMPQ_Win_create

#define MPI_Win_create_errhandler MPQ_Win_create_errhandler
#define PMPI_Win_create_errhandler PMPQ_Win_create_errhandler

#define MPI_Win_create_keyval MPQ_Win_create_keyval
#define PMPI_Win_create_keyval PMPQ_Win_create_keyval

#define MPI_Win_fence MPQ_Win_fence
#define PMPI_Win_fence PMPQ_Win_fence

#define MPI_Win_free MPQ_Win_free
#define PMPI_Win_free PMPQ_Win_free

#define MPI_Win_free_keyval MPQ_Win_free_keyval
#define PMPI_Win_free_keyval PMPQ_Win_free_keyval

#define MPI_Win_get_errhandler MPQ_Win_get_errhandler
#define PMPI_Win_get_errhandler PMPQ_Win_get_errhandler

#define MPI_Win_get_group MPQ_Win_get_group
#define PMPI_Win_get_group PMPQ_Win_get_group

#define MPI_Win_get_name MPQ_Win_get_name
#define PMPI_Win_get_name PMPQ_Win_get_name

#define MPI_Win_lock MPQ_Win_lock
#define PMPI_Win_lock PMPQ_Win_lock

#define MPI_Win_post MPQ_Win_post
#define PMPI_Win_post PMPQ_Win_post

#define MPI_Win_set_attr MPQ_Win_set_attr
#define PMPI_Win_set_attr PMPQ_Win_set_attr

#define MPI_Win_set_errhandler MPQ_Win_set_errhandler
#define PMPI_Win_set_errhandler PMPQ_Win_set_errhandler

#define MPI_Win_set_name MPQ_Win_set_name
#define PMPI_Win_set_name PMPQ_Win_set_name

#define MPI_Win_start MPQ_Win_start
#define PMPI_Win_start PMPQ_Win_start

#define MPI_Win_unlock MPQ_Win_unlock
#define PMPI_Win_unlock PMPQ_Win_unlock

#define MPI_Win_wait MPQ_Win_wait
#define PMPI_Win_wait PMPQ_Win_wait

#define MPI_Wtick MPQ_Wtick
#define PMPI_Wtick PMPQ_Wtick

#define MPI_Wtime MPQ_Wtime
#define PMPI_Wtime PMPQ_Wtime

/**************************/
/* Begin MPI-2 Extensions */
/**************************/

#define MPI_Open MPQ_Open
#define PMPI_Open PMPQ_Open

#define MPI_Close MPQ_Close
#define PMPI_Close PMPQ_Close

#define MPI_Comm_connect MPQ_Comm_connect
#define PMPI_Comm_connect PMPQ_Comm_connect

#define MPI_Comm_accept MPQ_Comm_accept
#define PMPI_Comm_accept PMPQ_Comm_accept

/************************/
/* End MPI-2 Extensions */
/************************/

#endif /* !defined(MPID_DEFS_H) */
