/* 
 * power.h - machine dependent header file
 * 
 * Author:	Chi-Chao Chang
 * 		Cornell University, Ithaca, NY
 * Date:	Tue Aug  8 02:07:51 1995
 *
 * Copyright (c) 1995 Chi-Chao Chang
 */

/*
 * QuickThreads -- Threads-building toolkit.
 * Copyright (c) 1993 by David Keppel
 *
 * Permission to use, copy, modify and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice and this notice
 * appear in all copies.  This software is provided as a
 * proof-of-concept and for demonstration purposes; there is no
 * representation about the suitability of this software for any
 * purpose.
 */

#ifndef QT_POWER_H
#define QT_POWER_H

#define QT_GROW_DOWN

typedef unsigned long qt_word_t;

/* Stack layout on the Power:
   Stack grows down: high addresses at top, low addresses at bottom.
   
   non-varargs:

   +---
   | align
   | lr <- qt_start
   | ccr
   | old_sp
   +---
   | fpr31
   | ...
   | fpr2
   | fpr1
   | gpr31
   | ...
   | gpr17 -> only
   | gpr16 -> userf
   | gpr15
   | gpr14 -> pt
   | gpr13 -> pu
   | align
   +---
   | arg area (8 words)
   +---
   | link area (6 words) <- qt_sp
   +---

   varargs:

   +---
   | remaining args at offset 56 from sp
   +---
   | need to copy first 8 args to gprs 3-10: done in QT_VARGS_MD1
   +---
   | don't care
   | don't care
   | don't care
   | lr <- qt_start
   | ccr
   | old_sp
   +---
   | fpr31
   | ...
   | fpr2
   | fpr1
   | gpr31
   | ...
   | gpr18 -> varg0
   | gpr17 -> startup
   | gpr16 -> userf
   | gpr15 -> cleanup
   | gpr14 -> pt
   | gpr13
   | align
   +---
   | arg area (8 words)            \
   +---                             > need this space in case thread calls function
   | link area (6 words) <- qt_sp  /  
   +---  
   */

/* What to do to start a thread running. */
extern void qt_start (void);
extern void qt_vstart (void);

#define QT_STKBASE     (4 + 4 + 4 + 4 + 19 * 4 + 18 * 8 + 4 + 14 * 4 + 32)
#define QT_VSTKBASE     (QT_STKBASE + 4 + 4)

/* Stack must be double-word aligned. */
#define QT_STKALIGN	(8)

#define QT_ONLY_INDEX	(QT_GPR17)
#define QT_USER_INDEX	(QT_GPR16)
#define QT_ARGT_INDEX	(QT_GPR14)
#define QT_ARGU_INDEX	(QT_GPR13)

#define QT_VSTARTUP_INDEX (QT_GPR17)
#define QT_VUSERF_INDEX   (QT_GPR16)
#define QT_VCLEANUP_INDEX (QT_GPR15)
#define QT_VARGT_INDEX    (QT_GPR14)

#define QT_START_ADDR	(19 + 18 * 2 + 1 + 1 + 1 + 14)
#define QT_GPR18        (6 + 14)
#define QT_GPR17        (5 + 14)
#define QT_GPR16       	(4 + 14)
#define QT_GPR15        (3 + 14)
#define QT_GPR14	(2 + 14)
#define QT_GPR13	(1 + 14)

#define QT_ARGS_MD(top) \
   (QT_SPUT((top), QT_START_ADDR, (*(int *)((int)qt_start))))

#define QT_VARGS_MD0(sp, vabytes) \
   ((qt_t *)(((char *)(sp)) - 8*4 - QT_STKROUNDUP(vabytes)))

/*
 Varargs implementation:
 The argument list is copied into the output's argument area by
 qt_vargs.  But the first 8 words are not used by the caller since
 their corresponding values should be place directly into gprs 3 to
 10.  This is indirectly done by QT_VARGS_MD1, which copies them to
 gprs 18-25 from where qt_vstart will move to 3-10.  QT_VARGS_MD1
 cannot move them to 3-10, obviously, because they'd be clobbered.
 So the args are left in the arg area except for the very first one
 which is clobbered during the call to startup (takes 1 arg).
 Therefore, the first arg is saved in gpr 18.  One could modify
 qt_vargs to move the arg list directly to registers if vabytes <= 8.
 For simplicity, it wasn't done.
 */

#define QT_VARGS_MD1(sp) \
   (QT_SPUT(sp, QT_START_ADDR, (*(int *)((int)qt_vstart))), \
    QT_SPUT(sp, QT_GPR18, ((qt_word_t *)vargs)[0]))

#define QT_VARGS_ADJUST(sp) ((char *)sp)

#define QT_VARGS_DEFAULT

#endif /* ndef QT_POWER_H */






