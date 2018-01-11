/*
 * fault.c
 *
 * Fault tolerance interface code.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/fault.c,v 1.5 1996/12/11 05:40:23 tuecke Exp $";

#include "internal.h"

/*
 * nexus_fault_strings
 *
 * String to go with each of the NEXUS_FAULT_* defines in nexus.h
 */
char *nexus_fault_strings[] =
{
    "None",
    "Unknown",
    "Process died",
    "Process shutdown abnormally",
    "Process shutdown normally",
    "Accepting of attachment failed",
    "Connect to process failed",
    "Bad protocol",
    "Read failed",
    "Bad URL"
};

static int (*callback_func_save)(void *user_arg,
			    int fault_code);
static void *callback_func_user_arg_save;


/*
 * _nx_fault_tolerance_usage_message()
 */
void _nx_fault_tolerance_usage_message(void)
{
} /* _nx_fault_tolerance_usage_message() */


/*
 * _nx_fault_tolerance_new_process_params()
 */
int _nx_fault_tolerance_new_process_params(char *buf, int size)
{
    return(0);
} /* _nx_fault_tolerance_new_process_params() */


/*
 * _nx_fault_tolerance_init()
 */
void _nx_fault_tolerance_init(int *argc, char ***argv)
{
     _nx_fault_tolerant = NEXUS_FALSE;
} /* _nx_fault_tolerance_init() */


/*
 * nexus_enable_fault_tolerance()
 */
void nexus_enable_fault_tolerance(int (*callback_func)(void *user_arg,
						       int fault_code),
				  void *callback_func_user_arg)
{
    callback_func_save = callback_func;
    callback_func_user_arg_save = callback_func_user_arg;
    
     _nx_fault_tolerant = NEXUS_TRUE;
} /* nexus_enable_fault_tolerance() */


/*
 * _nx_fault_detected()
 */
int _nx_fault_detected(int fault_code)
{
    int rc;
    if (!_nx_fault_tolerant)
    {
	rc = -1;
    }
    else if (callback_func_save)
    {
	rc = (*callback_func_save)(callback_func_user_arg_save,
				   fault_code);
    }
    else
    {
	rc = 0;
    }
    return(rc);
} /* _nx_fault_detected() */
