        .ident  $_set_stack
#include <mpp/regdef.h>
#include <mpp/vmmap.h>
#include <mpp/asdef.h>
 
        CRI_REGISTER_NAMES
        .psect  set_stack_code,code

SET_STACK::
set_stack::
	bic	a0,zero,sp
	stq	zero,0(sp)
        ret     zero, (ra)

        .psect  usmid,data
        .asciz  "%Z%%M% %I%     %G% %U%"

        .end


