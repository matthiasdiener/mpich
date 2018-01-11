#include <stdio.h>

#ifdef ANSI_ARGS
#undef ANSI_ARGS
#endif

#if defined(__STDC__) || defined(__cplusplus)
#ifndef USE_STDARG
#define USE_STDARG
#endif
#define ANSI_ARGS(x) x
#else
#define ANSI_ARGS(x) ()
#endif

void handle_listen_input ANSI_ARGS(( int ));
void handle_client_listen_input ANSI_ARGS(( int ));
void handle_mpirun_input ANSI_ARGS(( int ));
void handle_client_msgs_input ANSI_ARGS(( int ));
void handle_lhs_msgs_input ANSI_ARGS(( int ));
void handle_rhs_msgs_input ANSI_ARGS(( int ));
void handle_con_stdin_input ANSI_ARGS(( int ));
void handle_con_cntl_input ANSI_ARGS(( int ));
void handle_client_stdout_input ANSI_ARGS(( int ));
void handle_client_stderr_input ANSI_ARGS(( int ));
void man_cli_alive ANSI_ARGS(( int ));
void man_cli_findclient  ANSI_ARGS(( int ));
void man_cli_request_peer_connection  ANSI_ARGS(( int ));
void man_cleanup  ANSI_ARGS(( void ));
void man_compute_nodes_in_print_tree  ANSI_ARGS(( int, int, int *, int *, int * ));
void sig_all ANSI_ARGS(( char * ));

/* mananger defines for handlers are in mpd.h */
