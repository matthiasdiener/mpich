
static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/sanity.c,v 1.17 1996/10/09 23:45:27 tuecke Exp $";

#include "internal.h"

#ifdef NEXUS_SANITY_CHECK

void _nx_interrogate_suspect( void *evidence,
			      _nx_structure_types suspect_type, char *caller )
{
    switch( suspect_type ) {
    case _NX_NODE_T:
	break;
    case _NX_CONTEXT_T:
	break;
    case _NX_SEGMENT_T:
	break;
    case _NX_STARTPOINT_T:
	break;
    case _NX_ENDPOINT_T:
	break;
    case _NX_ENDPOINTATTR_T:
	break;
    case _NX_BUFFER_T:
	break;
    case _NX_BASE_SEGMENT_T:
	break;
    case _NX_DIRECT_SEGMENT_T:
	break;
#if 0
    case _NX_GLOBAL_POINTER_T:
    {
	/*
	 * struct _nexus_global_pointer_t = nexus_global_pointer_t
	 *
	 * nexus_proto_t *  proto
	 * nexus_mi_proto_t *  all_protos
	 * unsigned long       context
	 * unsigned long       address
	 * nexus_class_type_t  class_type
	 * int                 node_id
	 * int                 context_id
	 *
	 * struct _nexus_proto_t = nexus_proto_t
	 *
	 * nexus_proto_type_t   type
	 * nexus_proto_funcs_t *funcs
	 *
	 * struct _nexus_mi_proto_t = nexus_mi_proto_t
	 *
	 * int                        proto_type
	 * int                        n_ints
	 * int                        n_strings
	 * int                       *ints
	 * int                       *string_lengths
	 * char                     **strings
	 * struct _nexus_mi_proto_t * next
	 *
	 */
	nexus_global_pointer_t *gp;
	
	gp = (nexus_global_pointer_t *) evidence;

	nexus_debug_printf(1, ("Interrogating gp:%x in %s\n",gp,caller));

	if( gp==NULL ) {
	    char buf[100];
	    sprintf(buf, "%s: global pointer was at NULL address", caller);
	    _nx_imprison_thread( buf );
	    return;
	}

	/* check mi_proto */
	if ( gp->mi_proto == NULL )
	{
	    nexus_printf("Insanity! %s: warning..mi_proto was NULL\n", caller);
	    return;
	}
#ifdef DONT_INCLUDE
	/* we have a possible proto */
	{
	    nexus_proto_type_t max_pr_type;
	    unsigned long pr_type, ulmax_pr_type;

	    /* use ints to ensure valid comparisons regardless of
	     * compiler quirks with enums
	     */
	    pr_type = (unsigned long)(gp->proto->type);
	    max_pr_type = NEXUS_PROTO_TYPE_MAXVAL;
	    ulmax_pr_type = (unsigned long)max_pr_type;

	    if( pr_type >= ulmax_pr_type ) {
		char buf[300];
		sprintf(buf, "%s: nexus_proto_type_t (%lu) for nexus_proto_t* (%x) referenced by nexus_global_pointer_t* (%x) is invalid\n",caller,pr_type,gp->proto, gp);
		_nx_imprison_thread( buf );
		return;
	    }
	}
	/* check class type */
	{
	    unsigned long gp_class_type, max_class_type;
	    nexus_class_type_t max_class_type_val = NEXUS_CLASS_TYPE_MAXVAL;

	    max_class_type = (unsigned long)max_class_type_val;
	    gp_class_type = (unsigned long)(gp->class_type);
	    if (gp_class_type >= max_class_type) {
		char buf[200];
		sprintf(buf, "%s: gp(%x) class type %lu is invalid", gp, gp_class_type);
		_nx_imprison_thread( buf );
		return;
	    }
	}

	/* start checking mi_protos */
	{
	    nexus_mi_proto_t * curr_mi_proto;

	    if (gp->all_protos == NULL) {
/*		nexus_printf("%s: proto was non-null==(%x), but all_protos was null for gp(%x)\n", caller, gp->proto, gp);
		_nx_traceback();
		*/
		return;
	    }
	    /* start checking each mi_proto in the linked list */
	    curr_mi_proto = gp->all_protos;
	    do {
		nexus_proto_type_t max_proto_type;
		unsigned long pr_type, max_pr_type;

		max_proto_type = NEXUS_PROTO_TYPE_MAXVAL;
		max_pr_type = (unsigned long)max_proto_type;
		pr_type = (unsigned long)(curr_mi_proto->proto_type);

		/* unknown proto_type or mi_proto for local protocol */
		if( pr_type >= max_pr_type || pr_type == 0 ) {
		    char buf[300];
		    sprintf(buf, "%s: nexus_proto_type_t (%lu) for nexus_mi_proto_t* (%x) referenced by nexus_global_pointer_t* (%x) is invalid\n", caller, pr_type, curr_mi_proto, gp);
		    _nx_imprison_thread( buf );
		    return;
		}

		{
		    unsigned long  n_ints, n_strings;
		    unsigned long  pr_ints, pr_strings;
		    unsigned long  max_reasonable_int;
		    unsigned long  max_reasonable_string_length;
		    int test_pr_data=0;
		    int known_proto=1;

		    n_ints = (unsigned long)(curr_mi_proto->n_ints);
		    n_strings = (unsigned long)(curr_mi_proto->n_strings);

		    switch( curr_mi_proto->proto_type ) {
		    case NEXUS_PROTO_TYPE_TCP:
			pr_ints = 1; pr_strings = 1;
			max_reasonable_int = 65536;
			max_reasonable_string_length = 50;
			test_pr_data = 1;
			break;
		    case NEXUS_PROTO_TYPE_INX:
			pr_ints = 2; pr_strings = 1;
			break;
		    case NEXUS_PROTO_TYPE_MPL:
			pr_ints = 1; pr_strings = 0;
			break;
		    case NEXUS_PROTO_TYPE_MPINX:
			pr_ints = 1; pr_strings = 0;
			break;
		    default:
			known_proto = 0;
			break;
		    }
		    if( known_proto ) {
			if ( n_ints != pr_ints || n_strings != pr_strings ) {
			    char buf[300];
			    sprintf(buf, "%s: in gp(%x) mi_proto(%x) of type (%lu) n_ints/n_strings was (%lu/%lu) but should be (%lu/%lu)\n", 
				    caller, 
				    gp, 
				    curr_mi_proto, 
				    curr_mi_proto->proto_type, 
				    n_ints, 
				    n_strings, 
				    pr_ints, 
				    pr_strings);
			    _nx_imprison_thread( buf );
			    return;
			}
			if ( pr_ints > 0 && curr_mi_proto->ints==NULL ) {
			    char buf[300];
			    sprintf(buf, "%s: in gp(%x) mi_proto(%x) of type (%lu) n_ints was %lu but the ints field was NULL\n",
				    caller,
				    gp, 
				    curr_mi_proto,
				    curr_mi_proto->proto_type,
				    n_ints);
			    _nx_imprison_thread( buf );
			    return;
			}
			if ( pr_strings > 0 && 
			     curr_mi_proto->string_lengths==NULL ) {
			    char buf[300];
			    sprintf(buf, "%s: in gp(%x) mi_proto(%x) of type (%lu) n_strings was %lu but the string_lengths field was NULL\n",
				    caller,
				    gp, 
				    curr_mi_proto, 
				    curr_mi_proto->proto_type,
				    n_strings);
			    _nx_imprison_thread( buf );
			    return;
			}
			if ( pr_strings > 0 &&
			     curr_mi_proto->strings == NULL ) {
			    char buf[300];
			    sprintf(buf, "%s: in gp(%x) mi_proto(%x) of type (%lu) n_strings was %lu but the strings field was NULL\n",
				    caller,
				    gp, 
				    curr_mi_proto, 
				    curr_mi_proto->proto_type,
				    n_strings);
			    _nx_imprison_thread( buf );
			    return;
			}
			if (test_pr_data) {
			    unsigned long curr_indx;
			    for(curr_indx=0;curr_indx<pr_ints;curr_indx++){
				unsigned long curr_int;
				curr_int = (unsigned long)(curr_mi_proto->ints[curr_indx]);
				if( curr_int > max_reasonable_int ) {
				    nexus_printf("Insanity! %s: in gp(%x) mi_proto(%x) of type(%lu) integer number %lu of %lu was %lu which seems unreasonable\n",
						 caller, gp,
						 curr_mi_proto, curr_mi_proto->proto_type,
						 curr_indx+1, pr_ints,
						 curr_int);
				}
			    }
			    for(curr_indx=0;curr_indx<pr_strings;curr_indx++){
				unsigned long curr_string_length;
				curr_string_length = (unsigned long)
				   (curr_mi_proto->string_lengths[curr_indx]);
				/* check that the string lengths are ok */
				if( curr_string_length >
				    max_reasonable_string_length ) {
				    nexus_printf("Insanity! %s: in gp(%x) mi_proto(%x) of type(%lu) string length number %lu of %lu was %lu which seems unreasonable\n",
						 caller,
						 gp,
						 curr_mi_proto,
						 curr_mi_proto->proto_type,
						 curr_indx+1,
						 pr_strings,
						 curr_string_length);
				}
				/* check that the actual strings are ok */
				if( curr_string_length > 0 ) {
				    if( curr_mi_proto->strings[curr_indx] ==
					NULL ) {
					nexus_printf("Insanity! %s: in gp(%x) mi_proto(%x) of type(%lu) string number %lu of %lu should be %lu characters long but was NULL\n",
						     caller,
						     gp,
						     curr_mi_proto,
						     curr_mi_proto->proto_type,
						     curr_indx+1,
						     pr_strings,
						     curr_string_length);
				    } else {
					unsigned int char_pos;
					for(char_pos=0;
					    char_pos < curr_string_length;
					    char_pos++) {
					    if( (curr_mi_proto->strings[curr_indx])[char_pos] == '\0' ) {
						nexus_printf("Insanity! %s: in gp(%x) mi_proto(%x) of type(%lu) string number %lu of %lu should be %lu characters long but found NULL char at position %lu\n",
							     caller,
							     gp,
							     curr_mi_proto,
							     curr_mi_proto->proto_type,
							     curr_indx+1,
							     pr_strings,
							     curr_string_length,
							     char_pos);
						char_pos = curr_string_length;
					    }
					} /* check for short string */
				    } /* check for NULL or short string */
				} /* check for non-zero length strings */
			    } /* check string lengths and strings */
			}  /* test_pr_data */
		    } /* known proto */
		} /* local block */
		curr_mi_proto = curr_mi_proto->next;
	    } while( curr_mi_proto != NULL );
	}
#endif /* DONT_INCLUDE */
    }
    break;
#endif /* 0 */
#ifndef BUILD_LITE
    case _NX_THREAD_T:
	break;
    case _NX_MUTEX_T:
    {
	nexus_mutex_t *m;

	m = (nexus_mutex_t *) evidence;
	if(!_NX_CHECK_START_MAGIC_COOKIE(m))
	{
	    char buf[100];
	    sprintf(buf, "%s: Corrupted mutex (starting cookie = %x)",
		    caller, (unsigned int) _NX_EXTRACT_START_MAGIC_COOKIE(m));
	    _nx_imprison_thread( buf );
	}
	else if(!_NX_CHECK_END_MAGIC_COOKIE(m))
	{
	    char buf[100];
	    sprintf(buf, "%s: Corrupted mutex (ending cookie = %x)",
		    caller, (unsigned int) _NX_EXTRACT_END_MAGIC_COOKIE(m));
	    _nx_imprison_thread( buf );
	}
    }
	break;
    case _NX_COND_T:
    {
	nexus_cond_t *c;

	c = (nexus_cond_t *) evidence;
	if(!_NX_CHECK_START_MAGIC_COOKIE(c))
	{
	    char buf[100];
	    sprintf(buf, "%s: Corrupted condition (starting cookie = %x)",
		    caller, (unsigned int) _NX_EXTRACT_START_MAGIC_COOKIE(c));
	    _nx_imprison_thread( buf );
	}
	else if(!_NX_CHECK_END_MAGIC_COOKIE(c))
	{
	    char buf[100];
	    sprintf(buf, "%s: Corrupted condition (ending cookie = %x)",
		    caller, (unsigned int) _NX_EXTRACT_END_MAGIC_COOKIE(c));
	    _nx_imprison_thread( buf );
	}
	break;
    }
    case _NX_THREAD_FREELIST_T:
	break;
#endif /* BUILD_LITE */
    case _NX_HANDLER_LIST_T:
	break;
    case _NX_HANDLER_RECORD_T:
	break;
    default:
	nexus_printf("Unknown object type:%d\n",suspect_type);
	break;
    }

}

void _nx_imprison_thread( char *psz_charges )
{
    int dummy_flag;
    int parole=NEXUS_FALSE;

    nexus_printf("Insanity! Thread Imprisoned:\nCharged with: %s\n", psz_charges );
#ifdef TARGET_ARCH_AIX
    _nx_traceback();
#endif

    while( !parole )
    {
	dummy_flag=0;
    }

    nexus_printf("Thread released on parole\n" );

}

#endif /* NEXUS_SANITY_CHECK */
