#ifndef MPIR_HBT_COOKIE

#include "comm.h"
typedef struct _MPIR_HBT_node {
  MPIR_COOKIE                    /* Cookie to help detect valid items */
  void                  *value;
  int                   keyval;
  short                 balance;
  struct _MPIR_HBT_node *left;
  struct _MPIR_HBT_node *right;
} MPIR_HBT_node;
#define MPIR_HBT_NODE_COOKIE 0x03b740de

struct _MPIR_HBT {
  MPIR_COOKIE                    /* Cookie to help detect valid items */
  unsigned int   height;
  int            ref_count;
  MPIR_HBT_node *root;
};
#define MPIR_HBT_COOKIE 0x03b7c007

typedef union {
    int (*c_copy_fn) ANSI_ARGS(( MPI_Comm, int, void *, void *, void *, 
				 int * ));
    int (*f77_copy_fn) ANSI_ARGS(( MPI_Comm, int *, int *, int *, int *, 
				   int * ));
} MPIR_Copy_fn;
typedef union {
    int (*c_delete_fn) ANSI_ARGS(( MPI_Comm, int, void *, void * ));
    int (*f77_delete_fn) ANSI_ARGS(( MPI_Comm, int *, int *, void * ));
} MPIR_Delete_fn;

typedef struct  {
    MPIR_COOKIE                    /* Cookie to help detect valid items */
    MPIR_Copy_fn copy_fn;
    MPIR_Delete_fn delete_fn;
    void  *extra_state;
    int    FortranCalling;        /* Used to indicate whether Fortran or
				     C calling conventions are used for
				     copy_fn (attribute_in is passed by 
				     value in C, must be passed by reference
				     in Fortran); the underlying code
				     must also understand what a 
				     Fortran logical looks like */
    int    ref_count;
    int    permanent;             /* Used to mark the permanent attributes of
				     MPI_COMM_WORLD */
} MPIR_Attr_key;
#define MPIR_ATTR_COOKIE 0xa774c003

int MPIR_Attr_copy_node ANSI_ARGS(( MPI_Comm, MPI_Comm, MPIR_HBT_node * ));
int MPIR_Attr_copy_subtree ANSI_ARGS(( MPI_Comm, MPI_Comm, MPIR_HBT, 
				       MPIR_HBT_node * ));
int MPIR_Attr_free_node ANSI_ARGS(( MPI_Comm, MPIR_HBT_node * ));
int MPIR_Attr_free_subtree ANSI_ARGS(( MPI_Comm, MPIR_HBT_node * ));

int MPIR_HBT_new_tree ANSI_ARGS(( MPIR_HBT * ));
int MPIR_HBT_new_node ANSI_ARGS(( int, void *, MPIR_HBT_node ** ));
int MPIR_HBT_free_node ANSI_ARGS(( MPIR_HBT_node * ));
int MPIR_HBT_free_subtree ANSI_ARGS(( MPIR_HBT_node * ));
int MPIR_HBT_free_tree ANSI_ARGS(( MPIR_HBT ));
int MPIR_HBT_lookup ANSI_ARGS(( MPIR_HBT, int, MPIR_HBT_node ** ));
int MPIR_HBT_insert ANSI_ARGS(( MPIR_HBT, MPIR_HBT_node * ));
int MPIR_HBT_delete ANSI_ARGS(( MPIR_HBT, int, MPIR_HBT_node ** ));
void MPIR_HBT_Init ANSI_ARGS((void));
void MPIR_HBT_Free ANSI_ARGS((void));
#endif
