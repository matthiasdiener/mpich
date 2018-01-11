# QuickThreads -- Threads-building toolkit.
# Copyright (c) 1993 by David Keppel
#
# Permission to use, copy, modify and distribute this software and
# its documentation for any purpose and without fee is hereby
# granted, provided that the above copyright notice and this notice
# appear in all copies.  This software is provided as a
# proof-of-concept and for demonstration purposes; there is no
# representation about the suitability of this software for any
# purpose.
#

#
# power.s -- machine dependent asm
#
# Author:	 Chi-Chao Chang
#		 Cornell University, Ithaca, NY
#
# Copyright (c) 1995 Chi-Chao Chang
#

		.toc
T.qt_blocki:	.tc	.qt_blocki[TC],.qt_blocki[PR]
		.globl	.qt_blocki[PR]
		.csect	.qt_blocki[PR]
		l	12, T.qt_blocki(2)
		.using	.qt_blocki[PR],2

		.set	nfprs,18
		.set	ngprs,19
		.set	linkarea,32
		.set	argarea,24
		.set	savearea,8*nfprs+4*ngprs+4
		.set	roffset,4*ngprs+4+linkarea+argarea
		.set	sa,savearea+linkarea+argarea

		mfcr	12			# saving ccr and lr in caller's
		st	12,4(1)			# linker area
		mflr	0			
		st	0,8(1)
		stu	1,-sa(1)		# making a new stack area

		stm	13,4+linkarea+argarea(1)

		# register assignment
		# r3 = helper function
		# r4 = arg0
		# r5 = arg1
		# r6 = new sp

		oril	8,1,0
		oril	1,6,0			# switch stacks

		# move helper function in r3 to LR for a pointer call
		# branch always to address in LR;  LR updated to next inst.
		# move old stack pointer to r3 in delay slot
		# so, parameters to helper function in r3, r4, and r5
		l	3,0(3)
		mtlr	3
		oril	3,8,0
		brl
		nop
		
		lm	13,4+linkarea+argarea(1)	# restore gprs

		ai	1,1,sa
		l	12,4(1)			# restore volatile ccr bits
		mtcrf	0x38,12			# 2,3, and 4, so mask is 0x38
		l	12,8(1)			# restore lr
		mtlr	12
		brl

		.toc
T.qt_block:	.tc	.qt_block[TC],.qt_block[PR]
		.globl	.qt_block[PR]
		.csect	.qt_block[PR]
		l	12, T.qt_block(2)
		.using	.qt_block[PR],2

		mfcr	12			# saving ccr and lr in caller's
		st	12,4(1)			# linker area
		mflr	0			
		st	0,8(1)
		stu	1,-sa(1)		# making a new stack area

		stm	13,4+linkarea+argarea(1)
		ai	7,1,roffset
	
		stfd	14,0(7)			# saving volatile fprs
		stfd	15,8(7)			# alignment?
		stfd	16,0x10(7)
		stfd	17,0x18(7)
		stfd	18,0x20(7)
		stfd	19,0x28(7)
		stfd	20,0x30(7)
		stfd	21,0x38(7)
		stfd	22,0x40(7)
		stfd	23,0x48(7)
		stfd	24,0x50(7)
		stfd	25,0x58(7)
		stfd	26,0x60(7)
		stfd	27,0x68(7)
		stfd	28,0x70(7)
		stfd	29,0x78(7)
		stfd	30,0x80(7)
		stfd	31,0x88(7)

		# register assignment
		# r3 = helper function
		# r4 = arg0
		# r5 = arg1
		# r6 = new sp
		# r7 = &savearea

		oril	8,1,0
		oril	1,6,0			# switch stacks

		# move helper function in r3 to LR for a pointer call
		# branch always to address in LR;  LR updated to next inst.
		# move old stack pointer to r3 in delay slot
		# so, parameters to helper function in r3, r4, and r5
		l	3,0(3)
		mtlr	3
		oril	3,8,0
		brl
		nop
		
		lm	13,4+linkarea+argarea(1)	# restore gprs

		ai	7,1,roffset		
		lfd	14,0(7)
		lfd	15,8(7)
		lfd	16,0x10(7)
		lfd	17,0x18(7)
		lfd	18,0x20(7)
		lfd	19,0x28(7)
		lfd	20,0x30(7)
		lfd	21,0x38(7)
		lfd	22,0x40(7)
		lfd	23,0x48(7)
		lfd	24,0x50(7)
		lfd	25,0x58(7)
		lfd	26,0x60(7)
		lfd	27,0x68(7)
		lfd	28,0x70(7)
		lfd	29,0x78(7)
		lfd	30,0x80(7)
		lfd	31,0x88(7)

		ai	1,1,sa
		l	12,4(1)			# restore volatile ccr bits
		mtcrf	0x38,12			# 2,3, and 4, so mask is 0x38
		l	12,8(1)			# restore lr
		mtlr	12
		brl
	
		.toc
T.qt_abort:	.tc	.qt_abort[TC],.qt_abort[PR]
		.globl	.qt_abort[PR]
		.csect	.qt_abort[PR]
		l	12, T.qt_abort(2)
		.using	.qt_abort[PR],2
	
		oril	8,1,0			# save old sp
		oril	1,6,0			# switch stacks

		# move helper function in r3 to LR for a pointer call
		# branch always to address in LR;  LR updated to next inst.
		# move old stack pointer to r3 in delay slot
		# so, parameters to helper function in r3, r4, and r5
		l	3,0(3)
		mtlr	3
		oril	3,8,0	
		brl
		nop

		lm	13,4+linkarea+argarea(1)	# restore gprs
	
		ai	7,1,roffset		# restore fprs
		lfd	14,0(7)
		lfd	15,8(7)
		lfd	16,0x10(7)
		lfd	17,0x18(7)
		lfd	18,0x20(7)
		lfd	19,0x28(7)
		lfd	20,0x30(7)
		lfd	21,0x38(7)
		lfd	22,0x40(7)
		lfd	23,0x48(7)
		lfd	24,0x50(7)
		lfd	25,0x58(7)
		lfd	26,0x60(7)
		lfd	27,0x68(7)
		lfd	28,0x70(7)
		lfd	29,0x78(7)
		lfd	30,0x80(7)
		lfd	31,0x88(7)

		ai	1,1,sa
		l	12,4(1)			# restore volatile ccr bits
		mtcrf	0x38,12			# 2,3, and 4, so mask is 0x38
		l	0,8(1)			# restore lr
		mtlr	0
		brl

		.globl	qt_start
		.csect	qt_start[DS]
qt_start:	.long	.qt_start[PR],TOC[tc0],0

		.toc
T.qt_start:	.tc	.qt_start[TC],.qt_start[PR]
		.globl	.qt_start[PR]
		.csect	.qt_start[PR]
		l	12,T.qt_start(2)
		.using	.qt_start[PR],2

		oril	3,13,0
		oril	4,14,0
		oril	5,16,0
		l	17,0(17)
		mtlr	17
		brl
		nop

		.globl	qt_vstart
		.csect	qt_vstart[DS]
qt_vstart:	.long	.qt_vstart[PR],TOC[tc0],0

		.toc
T.qt_vstart:	.tc	.qt_vstart[TC],.qt_vstart[PR]
		.globl	.qt_vstart[PR]
		.csect	.qt_vstart[PR]
		l	12,T.qt_vstart(2)
		.using	.qt_vstart[PR],2
	
		oril	3,14,0			# calling startup with
		l	17,0(17)		# arg pt
		mtlr	17
		brl
		nop
		oril	3,18,0			# copying the first 8 args
		l	4,28(1)			# of varg list to gprs 3-10
		l	5,32(1)			# first arg is retrieved from
		l	6,36(1)			# gpr 18.
		l	7,40(1)
		l	8,44(1)
		l	9,48(1)
		l	10,52(1)
		l	16,0(16)		# ... before calling userf
		mtlr	16
		brl
		nop
		oril	4,3,0			# calling cleanup with return
		oril	3,14,0			# value of userf
		l	15,0(15)
		mtlr	15
		brl
		nop
	


