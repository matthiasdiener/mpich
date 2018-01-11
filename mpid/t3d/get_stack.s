        .ident  $_get_stack
;
;
#include <mpp/regdef.h>
#include <mpp/vmmap.h>
#include <mpp/asdef.h>
 
        CRI_REGISTER_NAMES
        .psect  get_stack_code,code

GET_STACK::
get_stack::
	addq    r31, r30, r0
	ret     zero, (ra)

        .psect  usmid,data
        .asciz  "%Z%%M% %I%     %G% %U%"

        .end


