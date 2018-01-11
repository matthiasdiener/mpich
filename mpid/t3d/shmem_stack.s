        .ident  $_shmem_stack
;
;        (C) COPYRIGHT CRAY RESEARCH, INC.
;        UNPUBLISHED PROPRIETARY INFORMATION.
;        ALL RIGHTS RESERVED.
;
#include <mpp/regdef.h>
#include <mpp/vmmap.h>
#include <mpp/asdef.h>
 
        CRI_REGISTER_NAMES
        .psect  shmem_stack_code,code
;
;       SHMEM_STACK
;
;       Synopsis (Fortran)
;
;               CALL SHMEM_STACK(target)
;
;       Synopsis (C)
;
;               void shmem_stack(void * target)
;
;       Arguments
;
;               target  is a data object of any noncharacter type which must
;                       reside on the stack on a remote PE.
;       Description
;
;               SHMEM_STACK checks that the target address is a valid stack
;               address on the current PE.  If target is beyond the top
;               of stack, shmem_stack extends the stack to include target.
;
;               SHMEM_STACK may be called prior to shmem_put or shmem_get
;               when the target address is on the stack.  This ensures that
;               an operand range error does not occur during the data passing
;               step.  On T3D systems a remote target address must also
;               be a valid address on the local PE for shmem_get or
;               shmem_put to work properly.
;
;       Note
;
;               This routine will not be available (or needed) on T3E systems.

SHMEM_STACK::
shmem_stack::
        subq    sp,a0,t0
        ble     t0,ret          ; simply return if stack is high enough already
        bic     a0,^x1f,sp
        stq     zero,0(sp)
        mb
ret:
        ret     zero, (ra)

        .psect  usmid,data
        .asciz  "%Z%%M% %I%     %G% %U%"

        .end


