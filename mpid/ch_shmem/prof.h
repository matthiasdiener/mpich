/*	$CHeader: prof.h 1.1 1995/11/08 13:59:59 $
 *	Copyright 1995  Convex Computer Corp.
 */
/* $Header: /epm/mpi/MCS/src/mpich1.0.11/mpid/ch_shmem/prof.h,v 1.1 1995/11/08 13:59:59 raja Exp $ */

/*
 * Note:  this file is identical to ../sys/DEFS.h with some definitions
 *        deleted (_ASPREFIX, ASENTRY, setjmp stuff).
 */

/*
 * _PREFIX -- prefix macro that generates procedure header with
 *            proper labels.
 *
 * There are three cases of how secondary definitions (name space
 * pollution) are to be handled.
 *
 * Do nothing if you want:
 *    Primary definition   -- "_name"
 *    Secondary definition -- "name"
 *
 * #define DSECDEF if you want:
 *    Primary definition   -- "__name"
 *    Secondary definition -- "name"
 *
 * #define NOSECDEF if you want:
 *    Primary definition   -- "name"
 *    Secondary definition -- NONE
 *
 * NOSECDEF or DSECDEF must be defined before the call to ENTRY(name)
 * or SYSCALL(name)
 */

/*
 * If _NAMESPACE_CLEAN is undefined, make sure that NOSECDEF
 * is defined and DSECDEF is not defined
 */
#ifndef _NAMESPACE_CLEAN
#   ifdef DSECDEF
#      undef DSECDEF
#   endif
#   ifndef NOSECDEF
#      define NOSECDEF
#   endif
#endif /* _NAMESPACE_CLEAN */

/*
 * _PREFIX for use by ENTRY
 */
#ifdef DSECDEF
#   define _PREFIX(x,y)\
        .space  $TEXT$;\
        .subspa $CODE$;\
        .export __/**/x,entry;\
        .export x,SEC_DEF;\
        .proc;\
        .callinfo y;\
        .entry;__/**/x;\
_/**/x;
#endif
#ifdef NOSECDEF
#   define _PREFIX(x,y)\
        .space  $TEXT$;\
        .subspa $CODE$;\
        .export x,entry;\
        .proc;\
        .callinfo y;\
        .entry;\
_/**/x;
#endif
#if !defined(DSECDEF) && !defined(NOSECDEF)
#   define _PREFIX(x,y)\
        .space  $TEXT$;\
        .subspa $CODE$;\
        .export _/**/x,entry;\
        .export x,SEC_DEF;\
        .proc;\
        .callinfo y;\
        .entry;_/**/x;\
_/**/x;
#endif

#ifndef PROF
#   define ENTRY(x,y)\
	_PREFIX(x,y)
#else

#ifdef NOSECDEF
/*
 * Profiled ENTRY definition
 */
/* _mcount(frompcindex, selfpc, cntp) */
#   define PROFCALL(x) \
        stw     rp,-20(sp);\
        stw     arg0,-36(sp);\
        stw     arg1,-40(sp);\
        stw     arg2,-44(sp);\
        stw     arg3,-48(sp);\
        ldo     48(sp),sp;\
        copy    rp,arg0;\
        ldil    L%x,arg1;\
        ldo     R%x(arg1),arg1;\
        ldil    L%x/**/cnt,arg2;\
        ldo     R%x/**/cnt(arg2),arg2;\
        .import _mcount;\
        ldil    L%_mcount,r31;\
        .call;\
        ble     R%_mcount,(4,r31);\
        copy    r31,rp;\
        ldo     -48(sp),sp;\
        ldw     -20(sp),rp;\
        ldw     -36(sp),arg0;\
        ldw     -40(sp),arg1;\
        ldw     -44(sp),arg2;\
        ldw     -48(sp),arg3

#   define ENTRY(x,y)\
        .space  $PRIVATE$;\
        .subspa $DATA$;x/**/cnt;\
        .word   0;\
        _PREFIX(x,no_unwind)\
        PROFCALL(x)
#endif /* NOSECDEF */

#ifdef DSECDEF
/*
 * Profiled ENTRY definition
 */
/* _mcount(frompcindex, selfpc, cntp) */
#   define PROFCALL(x) \
        stw     rp,-20(sp);\
        stw     arg0,-36(sp);\
        stw     arg1,-40(sp);\
        stw     arg2,-44(sp);\
        stw     arg3,-48(sp);\
        ldo     48(sp),sp;\
        copy    rp,arg0;\
        ldil    L%__/**/x,arg1;\
        ldo     R%__/**/x(arg1),arg1;\
        ldil    L%__/**/x/**/cnt,arg2;\
        ldo     R%__/**/x/**/cnt(arg2),arg2;\
        .import _mcount;\
        ldil    L%_mcount,r31;\
        .call;\
        ble     R%_mcount,(4,r31);\
        copy    r31,rp;\
        ldo     -48(sp),sp;\
        ldw     -20(sp),rp;\
        ldw     -36(sp),arg0;\
        ldw     -40(sp),arg1;\
        ldw     -44(sp),arg2;\
        ldw     -48(sp),arg3

#   define ENTRY(x,y)\
        .space  $PRIVATE$;\
        .subspa $DATA$;__/**/x/**/cnt;\
        .word   0;\
        _PREFIX(x,no_unwind)\
        PROFCALL(x)
#endif /* DSECDEF */

#if !defined(NOSECDEF) && !defined(DSECDEF)
/*
 * Profiled ENTRY definition
 */
/* _mcount(frompcindex, selfpc, cntp) */
#   define PROFCALL(x) \
        stw     rp,-20(sp);\
        stw     arg0,-36(sp);\
        stw     arg1,-40(sp);\
        stw     arg2,-44(sp);\
        stw     arg3,-48(sp);\
        ldo     48(sp),sp;\
        copy    rp,arg0;\
        ldil    L%_/**/x,arg1;\
        ldo     R%_/**/x(arg1),arg1;\
        ldil    L%_/**/x/**/cnt,arg2;\
        ldo     R%_/**/x/**/cnt(arg2),arg2;\
        .import _mcount;\
        ldil    L%_mcount,r31;\
        .call;\
        ble     R%_mcount,(4,r31);\
        copy    r31,rp;\
        ldo     -48(sp),sp;\
        ldw     -20(sp),rp;\
        ldw     -36(sp),arg0;\
        ldw     -40(sp),arg1;\
        ldw     -44(sp),arg2;\
        ldw     -48(sp),arg3

#   define ENTRY(x,y)\
        .space  $PRIVATE$;\
        .subspa $DATA$;_/**/x/**/cnt;\
        .word   0;\
        _PREFIX(x,no_unwind)\
        PROFCALL(x)
#endif /* ! NOSECDEF && ! DSECDEF*/

#endif /* PROF */
