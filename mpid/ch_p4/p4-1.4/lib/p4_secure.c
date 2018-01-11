#include "p4.h"
#include "p4_sys.h"

#if defined(SYMMETRY) || defined(SUN)  || \
    defined(DEC5000)  || defined(SGI)  || \
    defined(RS6000)   || defined(HP)   || \
    defined(NEXT)     || defined(CRAY) || \
    defined(CONVEX)   || defined(KSR)  || \
    defined(FX2800)   || defined(FX2800_SWITCH)  || \
    defined(SP1)

/**********************************
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pwd.h>
#include <stdio.h>
**********************************/

/* #define DEBUG */

char *start_prog_error;

extern int errno;
extern char *sys_errlist[];

static int connect_to_server ANSI_ARGS((char *));
static void send_string ANSI_ARGS((int,char *));
static void recv_string ANSI_ARGS((int,char *,int));

int start_slave(host, username, prog, port, am_slave, pw_hook)
char *host, *username, *prog, *am_slave;
int port;
char *(*pw_hook) ();
{
    int n, conn;
    struct passwd *pw;
    char port_string[250];
    char pgm_args_string[250];
    char *pw_string;
    static char buf[250];
    char *local_username;
    char myhost[256];
    int new_port, new_fd, stdout_fd;
    char msg[500];
    struct sockaddr_in temp;
    int rc, templen;
    int pid;
    struct timeval tv;
    fd_set rcv_fds;

    myhost[0] = '\0';
    get_qualified_hostname(myhost);
    /* gethostname(myhost, sizeof(myhost)); */

    conn = connect_to_server(host);
    if (conn < 0)
	return -1;

#ifdef DEBUG
    printf("Connected\n");
#endif

    pw = getpwuid(geteuid());
    if (pw == NULL)
    {
	extern char *getlogin();

	local_username = getlogin();
	if (local_username == NULL)
	{
	    start_prog_error = "Cannot get pw entry";
	    return -3;
	}
    }
    else
    {
	local_username = pw->pw_name;
    }

    send_string(conn, local_username);
    send_string(conn, username);
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    FD_ZERO(&rcv_fds);
    FD_SET(conn, &rcv_fds);
    n = select(conn+1,&rcv_fds,0,0,&tv);
    if (n == 1)
    {
	recv_string(conn, buf, sizeof(buf));
    }
    else
    {
	start_prog_error = "Handshake with server failed";
	return -3;
    }
#ifdef DEBUG
    printf("Got reply1 '%s'\n", buf);
#endif

    if (strncmp(buf, "Password", 8) == 0)
    {
	if (pw_hook == NULL)
	    pw_string = "";
	else
	    pw_string = (*pw_hook) (host, username);
	send_string(conn, pw_string);
	recv_string(conn, buf, sizeof(buf));
#ifdef DEBUG
	printf("Got reply '%s'\n", buf);
#endif
	if (strncmp(buf, "Proceed", 7) != 0)
	{
	    start_prog_error = buf;
	    return -4;
	}
    }
    else if (strncmp(buf, "Proceed", 7) != 0)
    {
	start_prog_error = buf;
	return -4;
    }

    send_string(conn, prog);

    sprintf(pgm_args_string, "%s %d %s", myhost, port, am_slave);
    send_string(conn, pgm_args_string);

    if ((pid = fork_p4()) == 0)
    {
	net_setup_anon_listener(10, &new_port, &new_fd);
	fflush(stdout);
	sprintf(port_string, "%d", new_port);
	send_string(conn, port_string);
	fflush(stdout);
	/* stdout_fd = net_accept(new_fd); */
	templen = sizeof(temp);
	SYSCALL_P4(stdout_fd, accept(new_fd, (struct sockaddr *) &temp, &templen));
	close(new_fd);

	n = 1;
	while (n > 0)
	{
	    SYSCALL_P4(n, read(stdout_fd, msg, 499));
	    if (n > 0)
	    {
		SYSCALL_P4(rc, write(1,msg,n));
		fflush(stdout);
	    }
	}
	exit(0);
    }

    recv_string(conn, buf, sizeof(buf));
#ifdef DEBUG
    printf("Got reply2 '%s'\n", buf);
#endif
    if (strncmp(buf, "Success", 7) != 0)
    {
	/* kill i/o handling process and decrement num forked */
	kill(pid,SIGKILL);
	p4_global->n_forked_pids--;
	start_prog_error = buf;
	return -4;
    }

    start_prog_error = buf;
    close(conn);

/***** Peter Krauss uses these lines
    if (kill(pid, 0) == 0)
        kill(pid, SIGKILL);
    p4_dprintfl(00, "waiting for termination of anon_listener %d\n", pid);
#if defined(DEC5000) || defined(HP) || defined(SUN)
    waitpid(pid, (int *) 0, 0);
#else
    wait((int *) 0);
#endif
*****/

    return 0;
}

static int connect_to_server(host)
char *host;
{
    int conn;
    int rc;
    struct hostent *hostent;
    struct sockaddr_in addr;

#ifdef SGI_TEST
    extern P4VOID net_set_sockbuf_size(int size, int skt);	/* 7/12/95, bri@sgi.com */
#endif

    SYSCALL_P4(conn, socket(AF_INET, SOCK_STREAM, 0));
    if (conn < 0)
    {
	start_prog_error = sys_errlist[errno];
	return -1;
    }

#ifdef SGI_TEST
    net_set_sockbuf_size(-1,conn);	/* 7/12/95, bri@sgi.com */
#endif

    hostent = gethostbyname_p4(host);

    addr.sin_family = hostent->h_addrtype;
    addr.sin_port = htons(sserver_port);
    bcopy(hostent->h_addr, &addr.sin_addr, hostent->h_length);

    SYSCALL_P4(rc, connect(conn, (struct sockaddr *) &addr, sizeof(addr)));
    if (rc < 0)
    {
	start_prog_error = sys_errlist[errno];
	return -1;
    }

    return conn;
}

static void send_string(sock, str)
int sock;
char *str;
{
    int rc, len = strlen(str);
    char nl = 10;

    SYSCALL_P4(rc, write(sock, str, len));
    if (rc < 0)
    {
	perror("write");
	p4_error("send_string write 1 ", -1);
    }
    SYSCALL_P4(rc, write(sock, &nl, 1));
    if (rc < 0)
    {
	perror("write");
	p4_error("send_string write 2 ", -1);
    }

}

static void recv_string(sock, buf, len)
int sock, len;
char *buf;
{
    char *bptr;
    int n;

    bptr = buf;
    while (1)
    {
	SYSCALL_P4(n, read(sock, bptr, 1));
	if (n < 0)
	{
	    perror("read");
	    p4_error("recv_string read ", -1);
	    exit(1);
	}
	if (*bptr == '\n')
	    break;
	bptr++;
	if (bptr - buf >= len)
	    break;
    }
    *bptr = 0;
}

#if defined(P4BSD) && !defined(NO_ECHO)

#ifdef FREEBSD
#include <sys/ioctl_compat.h>
#else
#include <sys/ioctl.h>
#endif

static struct sgttyb orig_tty;

static int echo_off ANSI_ARGS((void))
{
    struct sgttyb tty_new;

    if (ioctl(0, TIOCGETP, &orig_tty) < 0)
    {
	fprintf(stderr, "iotcl TIOCGETP failed: %s\n", sys_errlist[errno]);
	return -1;
    }

    tty_new = orig_tty;
    tty_new.sg_flags &= ~(ECHO);

    if (ioctl(0, TIOCSETP, &tty_new) < 0)
    {
	fprintf(stderr, "iotcl TIOCSETP failed: %s\n", sys_errlist[errno]);
	return -1;
    }
    return 0;
}

static int echo_on ANSI_ARGS((void))
{
    if (ioctl(0, TIOCSETP, &orig_tty) < 0)
    {
	fprintf(stderr, "iotcl TIOCSETP failed: %s\n", sys_errlist[errno]);
	return -1;
    }
    return 0;
}

#else

#include <termio.h>

struct termio tty_orig;

static int echo_off ANSI_ARGS((void))
{
    struct termio tty_new;

    if (ioctl(0, TCGETA, &tty_orig) < 0)
    {
	fprintf(stderr, "tcgetattr failed: %s\n", sys_errlist[errno]);
	return -1;
    }

    tty_new = tty_orig;

    tty_new.c_lflag &= ~(ECHO);

    if (ioctl(0, TCSETA, &tty_new) < 0)
    {
	fprintf(stderr, "tcsetattr failed: %s\n", sys_errlist[errno]);
	return -1;
    }
    return (0);
}

static int echo_on ANSI_ARGS((void))
{
    if (ioctl(0, TCSETA, &tty_orig) < 0)
    {
	fprintf(stderr, "tcsetattr failed: %s\n", sys_errlist[errno]);
	return -1;
    }
    return (0);
}

#endif

char *getpw_ss(host, name)
char *host, *name;
{
    static char buf[1024];
    char *s;

    echo_off();
    printf("Password for %s@%s: ", name, host);
    fflush(stdout);
    fgets(buf, sizeof(buf), stdin);
    echo_on();
    printf("\n");

    for (s = buf; *s; s++)
	if (*s == '\n')
	{
	    *s = 0;
	    break;
	}

    return buf;
}

#endif
/* #ifdef SYMMETRY */
