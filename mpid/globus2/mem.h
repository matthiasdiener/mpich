#ifndef __glodev_mem__
#define __glodev_mem__

#include <globus_common.h>

/*******************/
/* Mem mgmt macros */
/*******************/

#define g_malloc(Var, Type, Size) \
{ \
    size_t size = (Size); \
    if (size > 0) \
    { \
        if (((Var) = (Type) globus_libc_malloc (size)) == (Type) NULL) \
        { \
globus_libc_printf("FATAL ERROR: failed malloc %d bytes: file %s line %d\n", \
                   (int) size, __FILE__, __LINE__); \
            abort(); \
        } \
    } \
    else \
    { \
        (Var) = (Type) NULL; \
    } \
}

#define g_free(Ptr) \
{ \
    if ((Ptr) != NULL) {  globus_libc_free((void *)(Ptr));  } \
}

#endif /* __glodev_mem__ */
