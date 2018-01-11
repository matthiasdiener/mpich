/*
 * macros.h
 *
 * rcsid = "$Header: /nfs/globus1/src/master/nexus_source/macros.h,v 1.25 1996/12/12 08:35:27 tuecke Exp $"
 */

#ifndef _NEXUS_INCLUDE_MACROS_H
#define _NEXUS_INCLUDE_MACROS_H

/*
 * _nx_context()
 *
 * Get the context for the current thread and save it
 * into *Context (nexus_context_t **).
 */
#ifdef BUILD_LITE
#define _nx_context(Context) \
    *(Context) = _nx_lite_context
#else
#define _nx_context(Context) \
    *(Context) = (nexus_context_t *)nexus_thread_getspecific(_nx_context_key)
#endif


/*
 * _nx_set_context()
 *
 * Set Context to be the context of the current thread.
 */
#ifdef BUILD_LITE
#define _nx_set_context(Context) \
    _nx_lite_context = (Context)
#else
#define _nx_set_context(Context) \
    nexus_thread_setspecific(_nx_context_key, (void *) (Context));
#endif


#ifndef BUILD_LITE

/*
 * _nx_thread_context()
 *
 * Set *Context (nexus_context_t **) to be the context for the passed thread.
 */
#define _nx_thread_context(Thread, Context) \
    *(Context) = ((Thread)->context)

/*
 * _nx_thread_set_context()
 *
 * Set the context (nexus_context_t *) for the passed thread.
 *
 * This may be used by a message handling thread to set itself
 * to the appropriate context before handling a rsr.
 */
#define _nx_thread_set_context(Thread, Context) \
    (Thread)->context = (Context)

#endif /* BUILD_LITE */


/*
 * _nx_context_id()
 *
 * Set *Context_ID (int *) to be the context id for the context
 * of the calling thread.
 */
#define _nx_context_id(Context_ID) \
{ \
    nexus_context_t *__context; \
    _nx_context(&__context); \
    if (__context) \
        *(Context_ID) = __context->id; \
    else \
        *(Context_ID) = -1; \
}


/*
 * _nx_node_id()
 *
 * Set *Node_ID (int *) to be the node id for the node
 * of the calling thread.
 */
#define _nx_node_id(Node_ID) \
{ \
    *(Node_ID) = _nx_my_node_id; \
}


/*
 * Liba packing and unpacking:
 *	unsigned long Endpoint
 *	unsigned long State
 */
#define LibaSizeofWithoutState() (sizeof(unsigned long))
#define LibaSizeofWithState() (2*sizeof(unsigned long))

#define LibaPackWithoutState(Liba, Endpoint) \
{ \
    unsigned long __l; \
    __l = (Endpoint); \
    memcpy((Liba), &__l, (sizeof(unsigned long))); \
}
#define LibaPackWithState(Liba, Endpoint, State) \
{ \
    unsigned long __l[2]; \
    __l[0] = (Endpoint); \
    __l[1] = (State); \
    memcpy((Liba), __l, (2*sizeof(unsigned long))); \
}

#define LibaUnpack(Liba, Endpoint, State) \
{ \
    unsigned long __l[2]; \
    memcpy(__l, (Liba), (2*sizeof(unsigned long))); \
    (Endpoint) = __l[0]; \
    (State) = __l[1]; \
}
#define LibaUnpackEndpoint(Liba, Endpoint) \
{ \
    unsigned long __l; \
    memcpy(&__l, (Liba), (sizeof(unsigned long))); \
    (Endpoint) = __l; \
}
#define LibaUnpackState(Liba, State) \
{ \
    unsigned long __l; \
    memcpy(&__l, ((Liba)+sizeof(unsigned long)), (sizeof(unsigned long))); \
    (State) = __l; \
}


/*
 * PackInt1(nexus_byte_t *Array, int Index, int Integer)
 */
#define PackInt1(Array, Index, Integer) \
{ \
    (Array)[(Index)++] = (nexus_byte_t)  ((Integer) & 0xFF); \
}

/*
 * UnpackInt1(nexus_byte_t *Array, int Index, int Integer)
 */
#define UnpackInt1(Array, Index, Integer) \
{ \
    (Integer) = (int) (Array)[(Index)]; \
    (Index) += 1; \
}


/*
 * PackInt2(nexus_byte_t *Array, int Index, int Integer)
 */
#define PackInt2(Array, Index, Integer) \
{ \
    (Array)[(Index)++] = (nexus_byte_t) (((Integer) & 0xFF00) >> 8); \
    (Array)[(Index)++] = (nexus_byte_t)  ((Integer) & 0xFF); \
}

/*
 * UnpackInt2(nexus_byte_t *Array, int Index, int Integer)
 */
#define UnpackInt2(Array, Index, Integer) \
{ \
    (Integer) = (  ( ((int) (Array)[(Index)]) << 8) \
		 |   ((int) (Array)[(Index)+1]) ); \
    (Index) += 2; \
}


/*
 * PackInt4(nexus_byte_t *Array, int Index, int Integer)
 */
#define PackInt4(Array, Index, Integer) \
{ \
    (Array)[(Index)++] = (nexus_byte_t) (((Integer) & 0xFF000000) >> 24); \
    (Array)[(Index)++] = (nexus_byte_t) (((Integer) & 0xFF0000) >> 16); \
    (Array)[(Index)++] = (nexus_byte_t) (((Integer) & 0xFF00) >> 8); \
    (Array)[(Index)++] = (nexus_byte_t)  ((Integer) & 0xFF); \
}

/*
 * UnpackInt4(nexus_byte_t *Array, int Index, int Integer)
 */
#if NEXUS_DC_FORMAT_LOCAL == NEXUS_DC_FORMAT_32BIT_BE || NEXUS_DC_FORMAT_LOCAL == NEXUS_DC_FORMAT_32BIT_LE
#define UnpackInt4(Array, Index, Integer) \
{ \
    (Integer) = (  ( ((int) (Array)[(Index)])   << 24) \
		 | ( ((int) (Array)[(Index)+1]) << 16) \
		 | ( ((int) (Array)[(Index)+2]) << 8) \
		 |   ((int) (Array)[(Index)+3]) ); \
    (Index) += 4; \
}
#else
#define UnpackInt4(Array, Index, Integer) \
{ \
    (Integer) = (  ( ((int) (Array)[(Index)])   << 24) \
		 | ( ((int) (Array)[(Index)+1]) << 16) \
		 | ( ((int) (Array)[(Index)+2]) << 8) \
		 |   ((int) (Array)[(Index)+3]) ); \
    if ((Integer) & 0x80000000) /* Sign extend */ \
    { \
	(Integer) |= 0xFFFFFFFF00000000; \
    } \
    (Index) += 4; \
}
#endif


/*
 * PackMIProtoEntry(nexus_byte_t *Array, int Index,
 *                  nexus_proto_type_t Type, int Size, nexus_byte_t *SubArray)
 */
#define PackMIProtoEntry(Array, Index, Type, Size, SubArray) \
{ \
    PackInt2(Array, Index, (int) (Type)); \
    PackInt2(Array, Index, Size); \
    memcpy(&((Array)[(Index)]), SubArray, Size) ; \
    (Index) += (Size); \
}


/*
 * UnpackMIProtoEntry(nexus_byte_t *Array, int Index,
 *               nexus_proto_type_t Type, int Size, nexus_byte_t *SubArray)
 */
#define UnpackMIProtoEntry(Array, Index, Type, Size, SubArray) \
{ \
    int __i; \
    UnpackInt2(Array, Index, __i); \
    (Type) = (nexus_proto_type_t) __i; \
    UnpackInt2(Array, Index, Size); \
    (SubArray) = &((Array)[(Index)]); \
    (Index) += (Size); \
}


/*
 * UnpackMIProtoHeader(nexus_byte_t *Array, int Index,
 *                     int Node_number, int Context_differentiator,
 *                     char *Node_name)
 */
#define UnpackMIProtoHeader(Array, Index, Node_number, Context_differentiator, Node_name) \
{ \
    UnpackInt4(Array, Index, Node_number); \
    UnpackInt4(Array, Index, Context_differentiator); \
    (Node_name) = (char *) &((Array)[(Index)]); \
    (Index) += (strlen(Node_name) + 1); \
}


#ifdef BUILD_DEBUG
nexus_bool_t NexusDbgState( unsigned long, unsigned long,
			      unsigned long, unsigned long );
#define NexusDebug(Level) (_nx_debug_level >= (Level) || NexusDbgState( 0xffffffff, 0xffffffff, 0xffffffff, Level ))
#endif

#ifdef BUILD_DEBUG
#define nexus_debug_printf(level, message) \
do { \
    if (NexusDebug(level)) \
    { \
	nexus_printf message; \
    } \
} while (0)
#define nexus_new_debug_printf(catagory, module, operation, level, message) \
do { \
    if (NexusDbgState(catagory, module, operation, level)) \
    { \
	nexus_printf message; \
    } \
} while (0)
#else
#define nexus_debug_printf(level, message)
#define nexus_new_debug_printf(level, message)
#endif

#ifdef BUILD_DEBUG
#define NexusBufferMagicCheck(func, buf) \
{ \
    if (*(buf) == (nexus_buffer_t) NULL) \
	nexus_fatal(#func ": NULL nexus_buffer_t\n"); \
    if ((*(buf))->magic != NEXUS_BUFFER_MAGIC) \
	nexus_fatal(#func ": Bad magic number %x on buffer %x\n", \
		    (*(buf))->magic, buf); \
}
#else
#define NexusBufferMagicCheck(func, buf)
#endif


#ifdef BUILD_LITE
/*
 * We need to spoof a few functions in the NexusLite version.
 */
#define _nx_thread_id(Thread_ID) *(Thread_ID) = 0;
#endif /* BUILD_LITE */

#endif /* _NEXUS_INCLUDE_MACROS_H */
