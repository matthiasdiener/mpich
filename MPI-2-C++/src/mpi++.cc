// -*- c++ -*-
//
// Copyright 1997, University of Notre Dame.
// Authors: Andrew Lumsdaine, Michael P. McNally, Jeremy G. Siek,
//          Jeffery M. Squyres.
//
// This file is part of the Notre Dame C++ bindings for MPI
//
// You should have received a copy of the License Agreement for the
// Notre Dame C++ bindings for MPI along with the software;  see the
// file LICENSE.  If not, contact Office of Research, University of Notre
// Dame, Notre Dame, IN  46556.
//
// Permission to modify the code and to distribute modified code is
// granted, provided the text of this NOTICE is retained, a notice that
// the code was modified is included with the above COPYRIGHT NOTICE and
// with the COPYRIGHT NOTICE in the LICENSE file, and that the LICENSE
// file is distributed with the modified code.
//
// LICENSOR MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.
// By way of example, but not limitation, Licensor MAKES NO
// REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY
// PARTICULAR PURPOSE OR THAT THE USE OF THE LICENSED SOFTWARE COMPONENTS
// OR DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS, TRADEMARKS
// OR OTHER RIGHTS.
//


#include "mpi++.h"
#if HPUX_OS & LAM61
  // #%$^#$%^#$%^$ LAM on HP'S!!!!
#include <mpisys.h>
#undef MIN
#undef MAX
#endif

#if IBM_SP
  void throw_excptn_fctn(MPI_Comm* , int* errcode, char*, int*, int*)
#else
  void throw_excptn_fctn(MPI_Comm* , int* errcode, ...)
#endif
{
#if _MPIPP_USEEXCEPTIONS_
  throw(MPI::Exception(*errcode));
#else
  cerr << "exception throwing is disabled, MPI::errno has the error code" << endl;
  MPI::errno = *errcode;
#endif  
}

map _REAL_MPI_::Comm::mpi_comm_map;

#if IBM_SP
void
errhandler_intercept(MPI_Comm * mpi_comm, int * err, char*, int*, int*)
#else
void
errhandler_intercept(MPI_Comm * mpi_comm, int* err, ...)
#endif

{
  _REAL_MPI_::Comm* comm =
    (_REAL_MPI_::Comm*)_REAL_MPI_::Comm::mpi_comm_map[(void*)*mpi_comm];
  if (comm && comm->my_errhandler) {
    va_list ap;
    va_start(ap, err);
    comm->my_errhandler->handler_fn(*comm, err, ap);
    va_end(ap);
  }
}

_REAL_MPI_::Op* _REAL_MPI_::Intracomm::current_op;

void
op_intercept(void *invec, void *outvec, int *len, MPI_Datatype *datatype)
{
  _REAL_MPI_::Op* op = _REAL_MPI_::Intracomm::current_op;
  MPI::Datatype thedata = *datatype;
  ((MPI::User_function*)op->op_user_function)(invec, outvec, *len, thedata);
  //JGS the above cast is a bit of a hack, I'll explain:
  //  the type for the PMPI::Op::op_user_function is PMPI::User_function
  //  but what it really stores is the user's MPI::User_function supplied when
  //  the user did an Op::Init. We need to cast the function pointer back to
  //  the MPI::User_function. The reason the PMPI::Op::op_user_function was
  //  not declared a MPI::User_function instead of a PMPI::User_function is
  //  that without namespaces we cannot do forward declarations.
  //  Anyway, without the cast the code breaks on HP LAM with the aCC compiler.
}

map _REAL_MPI_::Comm::key_fn_map;

int
copy_attr_intercept(MPI_Comm oldcomm, int keyval, 
		    void *extra_state, void *attribute_val_in, 
		    void *attribute_val_out, int *flag)
{
  int ret = 0;
  map::Pair* copy_and_delete = (map::Pair*)_REAL_MPI_::Comm::key_fn_map[(map::address)keyval];
  MPI::Comm::Copy_attr_function* copy_fn;
  copy_fn = (MPI::Comm::Copy_attr_function*)copy_and_delete->first;

  map::Pair* comm_type = (map::Pair*)_REAL_MPI_::Comm::mpi_comm_map[(map::address)oldcomm];
  
  MPI::Intracomm intracomm;
  MPI::Intercomm intercomm;
  MPI::Graphcomm graphcomm;
  MPI::Cartcomm cartcomm;
  
  int thetype = (ATTR)comm_type->second;
  MPI2CPP_BOOL_T bflag = (MPI2CPP_BOOL_T)*flag; 

  switch (thetype) {
  case eIntracomm:
    intracomm = MPI::Intracomm(*(_REAL_MPI_::Intracomm*)comm_type->first);
    ret = copy_fn(intracomm, keyval, extra_state,
		  attribute_val_in, attribute_val_out, bflag);
  case eIntercomm:
    intercomm = MPI::Intercomm(*(_REAL_MPI_::Intercomm*)comm_type->first);
    ret = copy_fn(intercomm, keyval, extra_state,
		  attribute_val_in, attribute_val_out, bflag);
  case eGraphcomm:
    graphcomm = MPI::Graphcomm(*(_REAL_MPI_::Graphcomm*)comm_type->first);
    ret = copy_fn(graphcomm, keyval, extra_state,
		  attribute_val_in, attribute_val_out, bflag);
  case eCartcomm:
    cartcomm = MPI::Cartcomm(*(_REAL_MPI_::Cartcomm*)comm_type->first);
    ret = copy_fn(cartcomm, keyval, extra_state,
		  attribute_val_in, attribute_val_out, bflag);
  }

  *flag = (int)bflag;
  return ret;
}

int
delete_attr_intercept(MPI_Comm comm, int keyval, 
		      void *attribute_val, void *extra_state)
{
  int ret = 0;

  map::Pair* copy_and_delete = (map::Pair*)_REAL_MPI_::Comm::key_fn_map[(map::address)keyval];

  MPI::Comm::Delete_attr_function* delete_fn;  
  delete_fn = (MPI::Comm::Delete_attr_function*)copy_and_delete->second;

  map::Pair* comm_type = (map::Pair*)_REAL_MPI_::Comm::mpi_comm_map[(map::address)comm];

  MPI::Intracomm intracomm;
  MPI::Intercomm intercomm;
  MPI::Graphcomm graphcomm;
  MPI::Cartcomm cartcomm;
  
  int thetype = (long)(comm_type->second);

  switch (thetype) {
  case eIntracomm:
    intracomm = MPI::Intracomm(*(_REAL_MPI_::Intracomm*)comm_type->first);
    ret = delete_fn(intracomm, keyval, attribute_val, extra_state);
  case eIntercomm:
    intercomm = MPI::Intercomm(*(_REAL_MPI_::Intercomm*)comm_type->first);
    ret = delete_fn(intercomm, keyval, attribute_val, extra_state);
  case eGraphcomm:
    graphcomm = MPI::Graphcomm(*(_REAL_MPI_::Graphcomm*)comm_type->first);
    ret = delete_fn(graphcomm, keyval, attribute_val, extra_state);
  case eCartcomm:
    cartcomm = MPI::Cartcomm(*(_REAL_MPI_::Cartcomm*)comm_type->first);
    ret = delete_fn(cartcomm, keyval, attribute_val, extra_state);
  }
  return ret;
}

MPI::Status MPI::Comm::ignored_status;
MPI::Status MPI::Request::ignored_status;

#if ! _MPIPP_USEEXCEPTIONS_
int MPI::errno = MPI_SUCCESS;
#endif


const void* MPI::BOTTOM = (void*) MPI_BOTTOM;

// return  codes
const int MPI::SUCCESS = MPI_SUCCESS;
const int MPI::ERR_BUFFER = MPI_ERR_COUNT;
const int MPI::ERR_TYPE = MPI_ERR_TYPE;
const int MPI::ERR_TAG = MPI_ERR_TAG;
const int MPI::ERR_COMM = MPI_ERR_COMM;
const int MPI::ERR_RANK = MPI_ERR_RANK;
const int MPI::ERR_REQUEST = MPI_ERR_REQUEST;
const int MPI::ERR_ROOT = MPI_ERR_ROOT;
const int MPI::ERR_GROUP = MPI_ERR_GROUP;
const int MPI::ERR_OP = MPI_ERR_OP;
const int MPI::ERR_TOPOLOGY = MPI_ERR_TOPOLOGY;
const int MPI::ERR_DIMS = MPI_ERR_DIMS;
const int MPI::ERR_ARG = MPI_ERR_ARG;
const int MPI::ERR_UNKNOWN = MPI_ERR_UNKNOWN;
const int MPI::ERR_TRUNCATE = MPI_ERR_TRUNCATE;
const int MPI::ERR_OTHER = MPI_ERR_OTHER;
const int MPI::ERR_INTERN = MPI_ERR_INTERN;
#if HAVE_PENDING
const int MPI::ERR_PENDING = MPI_PENDING;
#else
const int MPI::ERR_PENDING = MPI_ERR_PENDING;
#endif
const int MPI::ERR_IN_STATUS = MPI_ERR_IN_STATUS;
const int MPI::ERR_LASTCODE = MPI_ERR_LASTCODE;

// assorted constants
const int MPI::PROC_NULL = MPI_PROC_NULL;
const int MPI::ANY_SOURCE = MPI_ANY_SOURCE;
const int MPI::ANY_TAG = MPI_ANY_TAG;
const int MPI::UNDEFINED = MPI_UNDEFINED;
const int MPI::BSEND_OVERHEAD = MPI_BSEND_OVERHEAD;
const int MPI::KEYVAL_INVALID = MPI_KEYVAL_INVALID;

// error-handling specifiers
const MPI::Errhandler  MPI::ERRORS_ARE_FATAL(MPI_ERRORS_ARE_FATAL);
const MPI::Errhandler  MPI::ERRORS_RETURN(MPI_ERRORS_RETURN);
const MPI::Errhandler  MPI::ERRORS_THROW_EXCEPTIONS(MPI_ERRORS_RETURN);
//JGS: the MPI_ERRORS_RETURN function in ERRORS_THROW_EXCEPTIONS gets replaced
//by the throw_exptn_fctn in MPI::Init

// maximum sizes for strings
const int MPI::MAX_PROCESSOR_NAME = MPI_MAX_PROCESSOR_NAME;
const int MPI::MAX_ERROR_STRING = MPI_MAX_ERROR_STRING;

// elementary datatypes
const MPI::Datatype MPI::CHAR(MPI_CHAR);
const MPI::Datatype MPI::SHORT(MPI_SHORT);
const MPI::Datatype MPI::INT(MPI_INT);
const MPI::Datatype MPI::LONG(MPI_LONG);
const MPI::Datatype MPI::UNSIGNED_CHAR(MPI_UNSIGNED_CHAR);
const MPI::Datatype MPI::UNSIGNED_SHORT(MPI_UNSIGNED_SHORT);
const MPI::Datatype MPI::UNSIGNED(MPI_UNSIGNED);
const MPI::Datatype MPI::UNSIGNED_LONG(MPI_UNSIGNED_LONG);
const MPI::Datatype MPI::FLOAT(MPI_FLOAT);
const MPI::Datatype MPI::DOUBLE(MPI_DOUBLE);
const MPI::Datatype MPI::LONG_DOUBLE(MPI_LONG_DOUBLE);
const MPI::Datatype MPI::BYTE(MPI_BYTE);
const MPI::Datatype MPI::PACKED(MPI_PACKED);

// datatypes for reductions functions (C / C++)
const MPI::Datatype MPI::FLOAT_INT(MPI_FLOAT_INT);
const MPI::Datatype MPI::DOUBLE_INT(MPI_FLOAT_INT);
const MPI::Datatype MPI::LONG_INT(MPI_LONG_INT);
const MPI::Datatype MPI::TWOINT(MPI_2INT);
const MPI::Datatype MPI::SHORT_INT(MPI_SHORT_INT);
const MPI::Datatype MPI::LONG_DOUBLE_INT(MPI_LONG_DOUBLE);

#if FORTRAN
// elementary datatype (Fortran)
const MPI::Datatype INTEGER(MPI_INTEGER);
const MPI::Datatype REAL(MPI_REAL);
const MPI::Datatype DOUBLE_PRECISION(MPI_DOUBLE_PRECISION);
const MPI::Datatype F_COMPLEX(MPI_COMPLEX);
const MPI::Datatype LOGICAL(MPI_LOGICAL);
const MPI::Datatype CHARACTER(MPI_CHARACTER);

// datatype for reduction functions (Fortran)
const MPI::Datatype TWOREAL(MPI_2REAL);
const MPI::Datatype TWODOUBLE_PRECISION(MPI_2DOUBLE_PRECISION);
const MPI::Datatype TWOINTEGER(MPI_2INTEGER);

// optional datatypes (Fortran)
#if ALL_OPTIONAL_FORTRAN
const MPI::Datatype INTEGER1(MPI_1INTEGER);
const MPI::Datatype INTEGER2(MPI_2INTEGER);
const MPI::Datatype INTEGER4(MPI_4INTEGER);
const MPI::Datatype REAL2(MPI_2REAL);
const MPI::Datatype REAL4(MPI_4REAL);
const MPI::Datatype REAL8(MPI_8REAL);
#elif SOME_OPTIONAL_FORTRAN
const MPI::Datatype INTEGER2(MPI_2INTEGER);
const MPI::Datatype REAL2(MPI_2REAL);
#endif //optional datatypes (Fortran)

#endif //FORTRAN

#if OPTIONAL_C
// optional datatype (C / C++)
const MPI::Datatype LONG_LONG(MPI_LONG_LONG);
const MPI::Datatype UNSIGNED_LONG_LONG(MPI_UNSIGNED_LONG_LONG);
#endif //OPTIONAL_C

#if 0
// c++ types
const MPI::Datatype BOOL;
const MPI::Datatype COMPLEX;
const MPI::Datatype DOUBLE_COMPLEX;
const MPI::Datatype LONG_DOUBLE_COMPLEX;
#endif

// reserved communicators
MPI::Intracomm MPI::COMM_WORLD(MPI_COMM_WORLD);
MPI::Intracomm MPI::COMM_SELF(MPI_COMM_SELF);

// results of communicator and group comparisons
const int MPI::IDENT = MPI_IDENT;
const int MPI::CONGRUENT = MPI_CONGRUENT;
const int MPI::SIMILAR = MPI_SIMILAR;
const int MPI::UNEQUAL = MPI_UNEQUAL;

// environmental inquiry keys
const int MPI::TAG_UB = MPI_TAG_UB;
const int MPI::IO = MPI_IO;
const int MPI::HOST= MPI_HOST;
const int MPI::WTIME_IS_GLOBAL = MPI_WTIME_IS_GLOBAL;

// collective operations
const MPI::Op MPI::MAX(MPI_MAX);
const MPI::Op MPI::MIN(MPI_MIN);
const MPI::Op MPI::SUM(MPI_SUM);
const MPI::Op MPI::PROD(MPI_PROD);
const MPI::Op MPI::MAXLOC(MPI_MAXLOC);
const MPI::Op MPI::MINLOC(MPI_MINLOC);
const MPI::Op MPI::BAND(MPI_BAND);
const MPI::Op MPI::BOR(MPI_BOR);
const MPI::Op MPI::BXOR(MPI_BXOR);
const MPI::Op MPI::LAND(MPI_LAND);
const MPI::Op MPI::LOR(MPI_LOR);
const MPI::Op MPI::LXOR(MPI_LXOR);

// null handles
const MPI::Group        MPI::GROUP_NULL = MPI_GROUP_NULL;
//const MPI::Comm         MPI::COMM_NULL = MPI_COMM_NULL;
//const MPI_Comm          MPI::COMM_NULL = MPI_COMM_NULL;
MPI::Comm_Null    MPI::COMM_NULL;
const MPI::Datatype     MPI::DATATYPE_NULL = MPI_DATATYPE_NULL;
MPI::Request      MPI::REQUEST_NULL = MPI_REQUEST_NULL;
const MPI::Op           MPI::OP_NULL = MPI_OP_NULL;
const MPI::Errhandler   MPI::ERRHANDLER_NULL;  

// empty group
const MPI::Group MPI::GROUP_EMPTY(MPI_GROUP_EMPTY);

// topologies
const int MPI::GRAPH = MPI_GRAPH;
const int MPI::CART = MPI_CART;

// special datatypes for contstruction of derived datatypes
const MPI::Datatype MPI::UB(MPI_UB);
const MPI::Datatype MPI::LB(MPI_LB);


