// Copyright 1997-1999, University of Notre Dame.
// Authors:  Jeremy G. Siek, Michael P. McNally, Jeffery M. Squyres, 
//           Andrew Lumsdaine
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
/****************************************************************************

 MESSAGE PASSING INTERFACE TEST CASE SUITE

 Copyright IBM Corp. 1995

 IBM Corp. hereby grants a non-exclusive license to use, copy, modify, and
 distribute this software for any purpose and without fee provided that the
 above copyright notice and the following paragraphs appear in all copies.

 IBM Corp. makes no representation that the test cases comprising this
 suite are correct or are an accurate representation of any standard.

 In no event shall IBM be liable to any party for direct, indirect, special
 incidental, or consequential damage arising out of the use of this software
 even if IBM Corp. has been advised of the possibility of such damage.

 IBM CORP. SPECIFICALLY DISCLAIMS ANY WARRANTIES INCLUDING, BUT NOT LIMITED
 TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS AND IBM
 CORP. HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 ENHANCEMENTS, OR MODIFICATIONS.

****************************************************************************

 These test cases reflect an interpretation of the MPI Standard.  They are
 are, in most cases, unit tests of specific MPI behaviors.  If a user of any
 test case from this set believes that the MPI Standard requires behavior
 different than that implied by the test case we would appreciate feedback.

 Comments may be sent to:
    Richard Treumann
    treumann@kgn.ibm.com

****************************************************************************
*/
#include "mpi2c++_test.h"


void
bcast_struct()
{
  char msg[150];

  int ii;
  double a[2];

  int len[2];

  MPI::Aint disp[2];
  MPI::Datatype newtype;
  MPI::Datatype type[2];
  MPI::Status status;
  
  Testing( (char *)"Create_struct");

  len[0] = 1; 
  len[1] = 2;
  
  disp[0] = (MPI::Aint) &ii; 
  disp[1] = (MPI::Aint) a;
  
  type[0] = MPI::INT; 
  type[1] = MPI::DOUBLE;

  newtype = MPI::Datatype::Create_struct(2, len, disp, type);
  newtype.Commit();

  if(my_rank == 0) { 
    ii = 2;
    a[0] = 123.456;  
    a[1] = 456.123;
  }

  MPI::COMM_WORLD.Bcast((void*) MPI::BOTTOM, 1, newtype, 0);
   
  if(ii != 2 || a[0] != 123.456 || a[1] != 456.123) {
     sprintf(msg, "NODE %d - ERROR! %d %f %f", my_rank, ii, a[0], a[1]);
     Fail(msg);
  }

  Pass(); // Create_struct

  if(newtype != MPI::DATATYPE_NULL)
    newtype.Free();
}

