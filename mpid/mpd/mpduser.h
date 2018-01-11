#ifndef MPD_USER
#define MPD_USER

/* These are used in p4mpd/lib/p4_sock_conn.c */
#define MAXLINE       2048
int parse_keyvals( char * );
int read_line( int, char *, int );
void send_msg( int, char *, int );
char *getval( char *, char * );

#endif
