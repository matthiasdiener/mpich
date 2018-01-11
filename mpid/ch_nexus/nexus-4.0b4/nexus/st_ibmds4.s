.set r0,0; .set SP,1; .set RTOC,2; .set r3,3; .set r4,4
.set r5,5; .set r6,6; .set r7,7; .set r8,8; .set r9,9
.set r10,10; .set r11,11; .set r12,12; .set r13,13; .set r14,14
.set r15,15; .set r16,16; .set r17,17; .set r18,18; .set r19,19
.set r20,20; .set r21,21; .set r22,22; .set r23,23; .set r24,24
.set r25,25; .set r26,26; .set r27,27; .set r28,28; .set r29,29
.set r30,30; .set r31,31
.set fp0,0; .set fp1,1; .set fp2,2; .set fp3,3; .set fp4,4
.set fp5,5; .set fp6,6; .set fp7,7; .set fp8,8; .set fp9,9
.set fp10,10; .set fp11,11; .set fp12,12; .set fp13,13; .set fp14,14
.set fp15,15; .set fp16,16; .set fp17,17; .set fp18,18; .set fp19,19
.set fp20,20; .set fp21,21; .set fp22,22; .set fp23,23; .set fp24,24
.set fp25,25; .set fp26,26; .set fp27,27; .set fp28,28; .set fp29,29
.set fp30,30; .set fp31,31
.set MQ,0; .set XER,1; .set FROM_RTCU,4; .set FROM_RTCL,5; .set FROM_DEC,6
.set LR,8; .set CTR,9; .set TID,17; .set DSISR,18; .set DAR,19; .set TO_RTCU,20
.set TO_RTCL,21; .set TO_DEC,22; .set SDR_0,24; .set SDR_1,25; .set SRR_0,26
.set SRR_1,27
.set BO_dCTR_NZERO_AND_NOT,0; .set BO_dCTR_NZERO_AND_NOT_1,1
.set BO_dCTR_ZERO_AND_NOT,2; .set BO_dCTR_ZERO_AND_NOT_1,3
.set BO_IF_NOT,4; .set BO_IF_NOT_1,5; .set BO_IF_NOT_2,6
.set BO_IF_NOT_3,7; .set BO_dCTR_NZERO_AND,8; .set BO_dCTR_NZERO_AND_1,9
.set BO_dCTR_ZERO_AND,10; .set BO_dCTR_ZERO_AND_1,11; .set BO_IF,12
.set BO_IF_1,13; .set BO_IF_2,14; .set BO_IF_3,15; .set BO_dCTR_NZERO,16
.set BO_dCTR_NZERO_1,17; .set BO_dCTR_ZERO,18; .set BO_dCTR_ZERO_1,19
.set BO_ALWAYS,20; .set BO_ALWAYS_1,21; .set BO_ALWAYS_2,22
.set BO_ALWAYS_3,23; .set BO_dCTR_NZERO_8,24; .set BO_dCTR_NZERO_9,25
.set BO_dCTR_ZERO_8,26; .set BO_dCTR_ZERO_9,27; .set BO_ALWAYS_8,28
.set BO_ALWAYS_9,29; .set BO_ALWAYS_10,30; .set BO_ALWAYS_11,31
.set CR0_LT,0; .set CR0_GT,1; .set CR0_EQ,2; .set CR0_SO,3
.set CR1_FX,4; .set CR1_FEX,5; .set CR1_VX,6; .set CR1_OX,7
.set CR2_LT,8; .set CR2_GT,9; .set CR2_EQ,10; .set CR2_SO,11
.set CR3_LT,12; .set CR3_GT,13; .set CR3_EQ,14; .set CR3_SO,15
.set CR4_LT,16; .set CR4_GT,17; .set CR4_EQ,18; .set CR4_SO,19
.set CR5_LT,20; .set CR5_GT,21; .set CR5_EQ,22; .set CR5_SO,23
.set CR6_LT,24; .set CR6_GT,25; .set CR6_EQ,26; .set CR6_SO,27
.set CR7_LT,28; .set CR7_GT,29; .set CR7_EQ,30; .set CR7_SO,31
.set TO_LT,16; .set TO_GT,8; .set TO_EQ,4; .set TO_LLT,2; .set TO_LGT,1

	.rename	H.10.NO_SYMBOL{PR},""
	.rename	H.20.gettocbase{TC},"gettocbase"
	.rename	H.24.settocbase_{TC},"settocbase_"

	.lglobl	H.10.NO_SYMBOL{PR}      
	.globl	.gettocbase             
	.globl	.settocbase_             
	.globl	gettocbase{DS}          
	.globl	settocbase_{DS}          


# .text section


	.csect	H.10.NO_SYMBOL{PR}      
.gettocbase:                            # 0x00000000 (H.10.NO_SYMBOL)
	.file	"gettocbase.c"          
	xor	r3, r3, r3
	or	r3, r3, RTOC
	bcr	BO_ALWAYS,CR0_LT
# traceback table
	.long	0x00000000
	.byte	0x00			# VERSION=0
	.byte	0x00			# LANG=TB_C
	.byte	0x20			# IS_GL=0,IS_EPROL=0,HAS_TBOFF=1
					# INT_PROC=0,HAS_CTL=0,TOCLESS=0
					# FP_PRESENT=0,LOG_ABORT=0
	.byte	0x40			# INT_HNDL=0,NAME_PRESENT=1
					# USES_ALLOCA=0,CL_DIS_INV=WALK_ONCOND
					# SAVES_CR=0,SAVES_LR=0
	.byte	0x80			# STORES_BC=1,FPR_SAVED=0
	.byte	0x00			# GPR_SAVED=0
	.byte	0x00			# FIXEDPARMS=0
	.byte	0x01			# FLOATPARMS=0,PARMSONSTK=1
	.long	0x00000010		# TB_OFFSET
	.short	10			# NAME_LEN
	.byte	"gettocbase"
# End of traceback table
.settocbase_:                            # 0x0000002c (H.10.NO_SYMBOL+0x2c)
	xor	r5, r5, r5
	or	r5, r5, RTOC
	xor	RTOC, RTOC, RTOC
	or	RTOC, r3, r3
	xor	r3, r3, r3
	or	r3, r3, r5
	bcr	BO_ALWAYS,CR0_LT
# traceback table
	.long	0x00000000
	.byte	0x00			# VERSION=0
	.byte	0x00			# LANG=TB_C
	.byte	0x20			# IS_GL=0,IS_EPROL=0,HAS_TBOFF=1
					# INT_PROC=0,HAS_CTL=0,TOCLESS=0
					# FP_PRESENT=0,LOG_ABORT=0
	.byte	0x40			# INT_HNDL=0,NAME_PRESENT=1
					# USES_ALLOCA=0,CL_DIS_INV=WALK_ONCOND
					# SAVES_CR=0,SAVES_LR=0
	.byte	0x80			# STORES_BC=1,FPR_SAVED=0
	.byte	0x00			# GPR_SAVED=0
	.byte	0x01			# FIXEDPARMS=1
	.byte	0x01			# FLOATPARMS=0,PARMSONSTK=1
	.long	0x00000000		# 
	.long	0x00000024		# TB_OFFSET
	.short	11			# NAME_LEN
	.byte	"settocbase_"
# End of traceback table
# End	csect	H.10.NO_SYMBOL{PR}

# .data section


	.toc	                        # 0x00000070 
T.20.gettocbase:
	.tc	H.20.gettocbase{TC},gettocbase{DS}
T.24.settocbase_:
	.tc	H.24.settocbase_{TC},settocbase_{DS}


	.csect	gettocbase{DS}          
	.long	.gettocbase             # "\0\0\0\0"
	.long	TOC{TC0}                # "\0\0\0p"
	.long	0x00000000              # "\0\0\0\0"
# End	csect	gettocbase{DS}


	.csect	settocbase_{DS}          
	.long	.settocbase_             # "\0\0\0,"
	.long	TOC{TC0}                # "\0\0\0p"
	.long	0x00000000              # "\0\0\0\0"
# End	csect	settocbase_{DS}



# .bss section
