/*
 * p0_funcs.h
 *
 * rcsid = "$Header: /nfs/globus1/src/master/ports0/p0_funcs.h,v 1.7 1996/01/29 22:47:23 patton Exp $"
 */

#ifndef _PORTS0_INCLUDE_FUNCS_H
#define _PORTS0_INCLUDE_FUNCS_H

/*
 * args.c
 */
extern void	_p0_args_init(int *argc, char **argv[]);


/*
 * th_*.c
 */
#ifndef BUILD_LITE
extern void	_p0_thread_usage_message(void);
extern int	_p0_thread_new_process_params(char *buf, int size);
extern void	_p0_thread_preinit(void);
extern void	_p0_thread_init(int *argc, char **argv[]);
extern int	_p0_thread_shutdown(void);
extern void	_p0_thread_abort(int return_code);
extern void	_p0_thread_create_idle_thread(void);
extern void	_p0_thread_shutdown_idle_thread(void);
#endif /* BUILD_LITE */
    

/*
 * trace.c
 */
extern void	_p0_traceback(void);


/*
 * util.c
 */
extern char *	_p0_copy_string(char *s);

#endif /* _PORTS0_INCLUDE_FUNCS_H */

