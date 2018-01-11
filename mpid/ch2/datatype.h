#ifndef MPIR_DATATYPE_COOKIE
/*****************************************************************************/
/* Datatypes.  The contiguous, predefined datatypes are handled separately   */
/* to demonstrate that the added functionality has low cost                  */
/*****************************************************************************/

/* Note that MPIR_VECTOR and MPIR_INDEXED are not used */
typedef enum {
    MPIR_INT, MPIR_FLOAT, MPIR_DOUBLE, MPIR_COMPLEX, MPIR_LONG, MPIR_SHORT,
    MPIR_CHAR, MPIR_BYTE, MPIR_UCHAR, MPIR_USHORT, MPIR_ULONG, MPIR_UINT,
    MPIR_CONTIG, MPIR_VECTOR, MPIR_HVECTOR, 
    MPIR_INDEXED,
    MPIR_HINDEXED, MPIR_STRUCT, MPIR_DOUBLE_COMPLEX, MPIR_PACKED, 
	MPIR_UB, MPIR_LB, MPIR_LONGDOUBLE, MPIR_LONGLONGINT, 
    MPIR_LOGICAL, MPIR_FORT_INT 
} MPIR_NODETYPE;

#define MPIR_DATATYPE_COOKIE 0xea31beaf
struct MPIR_DATATYPE {
    MPIR_NODETYPE dte_type; /* type of datatype element this is */
    MPIR_COOKIE             /* Cookie to help detect valid item */
    int          committed; /* whether committed or not */
    int          is_contig; /* whether entirely contiguous */
    int              basic; /* Is this a basic type */
    int          permanent; /* Is this a permanent type */
    MPI_Aint        ub, lb; /* upper/lower bound of type */
    MPI_Aint real_ub, real_lb; /* values WITHOUT TYPE_UB/TYPE_LB */
    int             has_ub; /* Indicates that the datatype has a TYPE_UB */
    int             has_lb; /* Indicates that the datatype has a TYPE_LB */
    MPI_Aint        extent; /* extent of this datatype */
    int               size; /* size of type */
    int           elements; /* number of basic elements */
    int          ref_count; /* nodes depending on this node */
    int              align; /* alignment needed for start of datatype */
    int              count; /* replication count */
    MPI_Aint        stride; /* stride, for VECTOR and HVECTOR types */
    MPI_Aint      *indices; /* array of indices, for (H)INDEXED, STRUCT */
    int           blocklen; /* blocklen, for VECTOR and HVECTOR types */
    int         *blocklens; /* array of blocklens for (H)INDEXED, STRUCT */
    MPI_Datatype old_type;  /* type this type is built of, if 1 */
    MPI_Datatype *old_types;/* array of types, for STRUCT */
    MPI_Datatype flattened; /* Flattened version, if available */
};

/* Holds translation from integer to MPI_Datatype */
#define MPIR_MAX_DATATYPE_ARRAY 256
/* defined in src/env/initutil.c */
/* MPIR_datatypes[0] is always 0, so null->null requires no special test */
extern MPI_Datatype MPIR_datatypes[MPIR_MAX_DATATYPE_ARRAY];

/* Used to allocate elements */
extern void *MPIR_dtes;   /* sbcnst datatype elements */

/* Translate between index and datatype pointer */
#define MPIR_GET_REAL_DATATYPE(a) \
  {if(MPIR_TEST_PREDEF_DATATYPE(a)) a = MPIR_datatypes[(MPI_Aint)(a)];}
/* Need to cast int to MPI_Aint to suppress silly compiler warnings */
#define MPIR_TEST_PREDEF_DATATYPE(a) \
    ((MPI_Aint)(a)<(MPI_Aint)MPIR_MAX_DATATYPE_ARRAY && (MPI_Aint)(a) >0)
#define MPIR_DATATYPE_CONTIG(a) \
    (MPIR_TEST_PREDEF_DATATYPE(a) || (a)->is_contig)
/* For ONLY the predefined datatypes, the size MAY be encoded in the 
   value of the datatype */
#define MPIR_DATATYPE_SIZE(a) (1 + ( (MPI_Aint)(a)&0xf ) )

/* Eventually, this will use MPIR_DATATYPE_SIZE */
#define MPIR_DATATYPE_GET_SIZE(a,contig_size) \
   MPIR_GET_REAL_DATATYPE(a);\
   if ((a)->is_contig) contig_size = (a)->size; else contig_size = 0;

#ifdef FOO
#define MPIR_DATATYPE_GET_SIZE(a,contig_size) \
   if (MPIR_TEST_PREDEF_DATATYPE(a)) contig_size=MPIR_DATATYPE_SIZE(a); \
   else {MPIR_GET_REAL_DATATYPE(a);\
   if ((a)->is_contig) contig_size = (a)->size; else contig_size = 0;}
#endif
#endif
