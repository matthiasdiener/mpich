/*
   Standard, simple Tcl argument conversion

   Ek Darrels
   Argonne National Laboratory
*/

#include "tcl.h"
#include <ctype.h>

#define DEBUG 0


#if defined(sparc) && defined(__STDC__)
int sscanf( char *, const char *, ... );
#endif


#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif


/* ugly, but it should work */
#ifdef __STDC__
int ConvertArgs( Tcl_Interp *interp, char *command, char *format,
		 int argc, char **argv, ... )
#else
int ConvertArgs( va_alist )
va_dcl
#endif
{
  va_list args;
  int argNum, *d, nskip;
  double *f;
  char **str, *readPt;

#ifdef __STDC__
  va_start( args,  argv );
#else
  Tcl_Interp *interp;
  char *command;
  char *format;
  int argc;
  char **argv;
  va_start( args );
  interp = va_arg( args, Tcl_Interp* );
  command = va_arg( args, char* );
  format = va_arg( args, char* );
  argc = va_arg( args, int );
  argv = va_arg( args, char** );
#endif

  /* get the number of arguments to skip in argv[] */
  sscanf( format, "%d", &nskip );
  argNum = nskip;

  readPt = format;
  /* skip over integer with possible leading and trailing whitespace */
  while (isspace( *readPt )) readPt++;
  while (!isspace( *readPt )) readPt++;
  while (isspace( *readPt )) readPt++;


  while (*readPt && argNum<argc) {
    switch (*readPt) {
    case 'd':
      d = va_arg( args, int* );
      if (!sscanf( argv[argNum], "%d", d )) {
	goto failed_to_convert;
      }
      break;

    case 'f':
      f = va_arg( args, double* );
      if (!sscanf( argv[argNum], "%lf", f )) {
	goto failed_to_convert;
      }
      break;

    case 's':
      str = va_arg( args, char** );
      *str = argv[argNum];
      break;
    }
    argNum++;
    readPt++;
  }

    /* if there were more arguments left to convert, complain */
  if (*readPt) {
fprintf( stderr, "Not enough arguments for procedure.  Syntax: %s.\n",
	 command );
    Tcl_AppendResult( interp, "Not enough arguments for procedure.  ",
		      "Syntax: ", command, (char*)0 );
    va_end( args );
    return TCL_ERROR;
  }

  va_end( args );
  return TCL_OK;

 failed_to_convert:
  {
    char numStr[20];
    va_end( args );
    sprintf( numStr, "%d", argNum+1 );
    Tcl_AppendResult( interp, "Failed to convert arg ", numStr, " (",
		      argv[argNum], ") to a",
		      (*readPt=='d') ? "n integer" :
		      (*readPt=='f') ? " double" : " string",
		      ".  Syntax: ", command, (char *)0 );
    return TCL_ERROR;
  }
}

