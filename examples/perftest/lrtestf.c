/* lrtest.c */
/* Fortran interface file */
#include "tools.h"

#ifdef POINTER_64_BITS
extern void *__ToPointer();
extern int __FromPointer();
extern void __RmPointer();
#else
#define __ToPointer(a) (a)
#define __FromPointer(a) (int)(a)
#define __RmPointer(a)
#endif

#include "testing/lrctx.h"
#ifdef FORTRANCAPS
#define lrrunsingletest_ LRRUNSINGLETEST
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define lrrunsingletest_ lrrunsingletest
#endif
 double  lrrunsingletest_( lrctx, f, fctx, x)
LRctx  *lrctx;
double (*f)();
void   *fctx;
double*x;
{
return LRRunSingleTest(
	(LRctx* )__ToPointer( *(int*)(lrctx) ),f,fctx,*x);
}
