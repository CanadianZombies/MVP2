#ifdef NEW_WORLD
#define VERSION_STR "Aturion Dynasty (c) Owen Emlen, 1996 v2.5  Revision 11/22/97"
#else
#define VERSION_STR "EmlenMud(c)1995 & revision: EKKCode 2003 v3.0 - 02/01/2003"
#endif
/*#define SETEVAL(real,array) 

int rstr=11, rint=7, rwis=7, rdex=13, con=6;
*/


#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif


#if defined(unix)
#include <signal.h>
#endif

#ifndef WINDOWS
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/telnet.h>

#endif


/*
int select  ( int width, fd_set *readfds, fd_set *writefds,
fd_set *exceptfds, struct timeval *timeout  );
int write  ( int fd, char *buf, int nbyte  );
int setsockopt  ( int s, int level, int optname, caddr_t optval,
int optlen  );
int socket  ( int domain, int type, int protocol  );
*/
int read (int fd, char *buf, int nbyte);
int write (int fd, char *buf, int nbyte);

#if defined(sun)
int accept (int s, struct sockaddr *addr, int *addrlen);
int bind (int s, struct sockaddr *name, int namelen);
void bzero (char *b, int length);
int close (int fd);
int getpeername (int s, struct sockaddr *name, int *namelen);
int getsockname (int s, struct sockaddr *name, int *namelen);
int listen (int s, int backlog);
/*int read  ( int fd, char *buf, int nbyte  );
*/ int select (int width, fd_set * readfds, fd_set * writefds,
							      fd_set *
							      exceptfds,
							      struct timeval
							      *timeout);

#if defined(SYSV)
int setsockopt (int s, int level, int optname,
		const char *optval, int optlen);
#else
/*int setsockopt  ( int s, int level, int optname, void *optval,
int optlen  );*/
#endif

/*int socket  ( int domain, int type, int protocol  );*/
/*int write  ( int fd, char *buf, int nbyte  );
*/
#endif
