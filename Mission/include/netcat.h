#ifndef _NETCAT_H_
#define _NETCAT_H_

/* Netcat 1.10 RELEASE 960320

   A damn useful little "backend" utility begun 950915 or thereabouts,
   as *Hobbit*'s first real stab at some sockets programming.  Something that
   should have and indeed may have existed ten years ago, but never became a
   standard Unix utility.  IMHO, "nc" could take its place right next to cat,
   cp, rm, mv, dd, ls, and all those other cryptic and Unix-like things.

   Read the README for the whole story, doc, applications, etc.

   Layout:
	conditional includes:
	includes:
	handy defines:
	globals:
	malloced globals:
	cmd-flag globals:
	support routines:
	readwrite select loop:
	main:

  bluesky:
	parse ranges of IP address as well as ports, perhaps
	RAW mode!
	backend progs to grab a pty and look like a real telnetd?!
	backend progs to do various encryption modes??!?!
*/

#include "generic.h"		/* same as with L5, skey, etc */

/* conditional includes -- a very messy section which you may have to dink
   for your own architecture [and please send diffs...]: */
   /* #undef _POSIX_SOURCE*/		/* might need this for something? */
#undef HAVE_BIND		/* ASSUMPTION -- seems to work everywhere! */
#define HAVE_HELP		/* undefine if you dont want the help text */
/* #define ANAL*/			/* if you want case-sensitive DNS matching */

#ifdef AESCRYPT
#include <mix/mix.h>
#ifndef HAVE_STDLIB_H
#define HAVE_STDLIB_H 1
#endif
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#else
#include <malloc.h>
#endif
#ifdef HAVE_SELECT_H		/* random SV variants need this */
#include <sys/select.h>
#endif

/* have to do this *before* including types.h. xxx: Linux still has it wrong */
#ifdef FD_SETSIZE		/* should be in types.h, butcha never know. */
#undef FD_SETSIZE		/* if we ever need more than 16 active */
#endif				/* fd's, something is horribly wrong! */
#define FD_SETSIZE 1024		/* <-- this'll give us a long anyways, wtf */
#include <sys/types.h>		/* *now* do it.  Sigh, this is broken */

#ifdef HAVE_RANDOM		/* aficionados of ?rand48() should realize */
#define SRAND srandom		/* that this doesn't need *strong* random */
#define RAND random		/* numbers just to mix up port numbers!! */
#else
#define SRAND srand
#define RAND rand
#endif /* HAVE_RANDOM */

/* #define POSIX_SETJMP*/		/* If you want timeouts to work under the */
				/* posixly correct, yet non-standard glibc-2.x*/
				/* then define this- you may also need it for */
				/* IRIX, and maybe some others */
#ifdef LINUX
#define POSIX_SETJMP
#endif

/* includes: */
#include <sys/time.h>		/* timeval, time_t */
#include <setjmp.h>		/* jmp_buf et al */
#include <sys/socket.h>		/* basics, SO_ and AF_ defs, sockaddr, ... */
#include <netinet/in.h>		/* sockaddr_in, htons, in_addr */
#include <netinet/in_systm.h>	/* misc crud that netinet/ip.h references */
#include <netinet/ip.h>		/* IPOPT_LSRR, header stuff */
#include <netdb.h>		/* hostent, gethostby*, getservby* */
#include <arpa/inet.h>		/* inet_ntoa */
#include <stdio.h>
#include <string.h>		/* strcpy, strchr, yadda yadda */
#include <errno.h>
#include <signal.h>
#include <fcntl.h>		/* O_WRONLY et al */
#include <unistd.h>
#ifdef LINUX			/* Linux needs the HERE, oh well. */
#include <resolv.h>
#endif

/* handy stuff: */
#define SA struct sockaddr	/* socket overgeneralization braindeath */
#define SAI struct sockaddr_in	/* ... whoever came up with this model */
#define IA struct in_addr	/* ... should be taken out and shot, */
				/* ... not that TLI is any better.  sigh.. */
#ifdef INET6
#define SS struct sockaddr
#define SAI6 struct sockaddr_in6
#define IA6 struct in6_addr
#endif

#define SLEAZE_PORT 31337	/* for UDP-scan RTT trick, change if ya want */
#define USHORT unsigned short	/* use these for options an' stuff */
#define BIGSIZ 8192		/* big buffers */

#ifdef AESCRYPT
int krypton;
#endif

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif
#ifdef MAXHOSTNAMELEN
#undef MAXHOSTNAMELEN		/* might be too small on aix, so fix it */
#endif
#define MAXHOSTNAMELEN 256

struct host_poop {
	char name[MAXHOSTNAMELEN];	/* dns name */
	char addrs[8][24];		/* ascii-format IP addresses */
	struct in_addr iaddrs[8];	/* real addresses: in_addr.s_addr: ulong */
};
#define HINF struct host_poop

#ifdef INET6
struct host6_poop {
	char name[MAXHOSTNAMELEN];	/* dns name */
	char addrs[8][64];		/* ascii-format IP addresses */
	struct in6_addr iaddrs[8];	/* real addresses: in_addr.s_addr: ulong */
};
#define HINF6 struct host6_poop
#endif

struct port_poop {
	char name[64];		/* name in /etc/services */
	char anum[8];		/* ascii-format number */
	USHORT num;			/* real host-order number */
};
#define PINF struct port_poop

/* Debug macro: squirt whatever message and sleep a bit so we can see it go
   by.  need to call like Debug ((stuff)) [with no ; ] so macro args match!
   Beware: writes to stdOUT... */
#ifdef DEBUG
#define Debug(x) printf x; printf ("\n"); fflush (stdout); sleep (1);
#else
#define Debug(x)	/* nil... */
#endif

/* support routines -- the bulk of this thing.  Placed in such an order that
	we don't have to forward-declare anything: */

	/* holler :
		fake varargs -- need to do this way because we wind up calling through
		more levels of indirection than vanilla varargs can handle, and not all
		machines have vfprintf/vsyslog/whatever!  6 params oughta be enough. */
//void holler(char * str, char * p1, char * p2, char * p3, char * p4, char * p5, char * p6);

/* bail :
   error-exit handler, callable from anywhere */
//void bail(char * str, char * p1, char * p2, char * p3, char * p4, char * p5, char * p6);

/* catch :
   no-brainer interrupt handler */
void catch (void);

/* quit :
   handler for a "-q" timeout (exit 0 instead of 1) */
void quit(void);

/* timeout and other signal handling cruft */
void tmtravel(void);

/* arm_timer :
   set the timer.  Zero secs arg means unarm */
void arm_timer(unsigned int num, unsigned int secs);

/* Hmalloc :
   malloc up what I want, rounded up to *4, and pre-zeroed.  Either succeeds
   or bails out on its own, so that callers don't have to worry about it. */
char * Hmalloc(unsigned int size);

/* findline :
   find the next newline in a buffer; return inclusive size of that "line",
   or the entire buffer size, so the caller knows how much to then write().
   Not distinguishing \n vs \r\n for the nonce; it just works as is... */
unsigned int findline(char * buf, unsigned int siz);

/* comparehosts :
   cross-check the host_poop we have so far against new gethostby*() info,
   and holler about mismatches.  Perhaps gratuitous, but it can't hurt to
   point out when someone's DNS is fukt.  Returns 1 if mismatch, in case
   someone else wants to do something about it. */
int comparehosts(HINF * poop, struct hostent * hp);

#ifdef INET6
	char *inet_ntoa6(struct in6_addr *s);
	int comparehosts6(HINF6 * poop, struct hostent * hp);
#endif

/* gethostpoop :
   resolve a host 8 ways from sunday; return a new host_poop struct with its
   info.  The argument can be a name or [ascii] IP address; it will try its
   damndest to deal with it.  "numeric" governs whether we do any DNS at all,
   and we also check o_verbose for what's appropriate work to do. */
HINF * gethostpoop(char * name, USHORT numeric);

#ifdef INET6
	HINF6 *gethost6poop(char *name, USHORT numeric);
#endif

/* getportpoop :
   Same general idea as gethostpoop -- look up a port in /etc/services, fill
   in global port_poop, but return the actual port *number*.  Pass ONE of:
	pstring to resolve stuff like "23" or "exec";
	pnum to reverse-resolve something that's already a number.
   If o_nflag is on, fill in what we can but skip the getservby??? stuff.
   Might as well have consistent behavior here, and it *is* faster. */
USHORT getportpoop(char * pstring, unsigned int pnum);

/* nextport :
   Come up with the next port to try, be it random or whatever.  "block" is
   a ptr to randports array, whose bytes [so far] carry these meanings:
	0	ignore
	1	to be tested
	2	tested [which is set as we find them here]
   returns a USHORT random port, or 0 if all the t-b-t ones are used up. */
USHORT nextport(char * block);

/* loadports :
   set "to be tested" indications in BLOCK, from LO to HI.  Almost too small
   to be a separate routine, but makes main() a little cleaner... */
void loadports(char * block, USHORT lo, USHORT hi);

#ifdef GAPING_SECURITY_HOLE
//char * pr00gie = NULL;			/* global ptr to -e arg */
//int doexec_use_sh = 0;			/* `-c' or `-e' option? */

/* doexec_new :
   fiddle all the file descriptors around, and hand off to another prog.  Sort
   of like a one-off "poor man's inetd".  This is the only section of code
   that would be security-critical, which is why it's ifdefed out by default.
   Use at your own hairy risk; if you leave shells lying around behind open
   listening ports you deserve to lose!! */
doexec_new(int fd);

/* doexec :
   fiddle all the file descriptors around, and hand off to another prog.  Sort
   of like a one-off "poor man's inetd".  This is the only section of code
   that would be security-critical, which is why it's ifdefed out by default.
   Use at your own hairy risk; if you leave shells lying around behind open
   listening ports you deserve to lose!! */
doexec(int fd);

#endif /* GAPING_SECURITY_HOLE */

/* doconnect :
   do all the socket stuff, and return an fd for one of
	an open outbound TCP connection
	a UDP stub-socket thingie
   with appropriate socket options set up if we wanted source-routing, or
	an unconnected TCP or UDP socket to listen on.
   Examines various global o_blah flags to figure out what-all to do. */
int doconnect(IA * rad, USHORT rp, IA * lad, USHORT lp);

/* dolisten :
   just like doconnect, and in fact calls a hunk of doconnect, but listens for
   incoming and returns an open connection *from* someplace.  If we were
   given host/port args, any connections from elsewhere are rejected.  This
   in conjunction with local-address binding should limit things nicely... */
int dolisten(IA * rad, USHORT rp, IA * lad, USHORT lp);

/* udptest :
	fire a couple of packets at a UDP target port, just to see if it's really
	there.  On BSD kernels, ICMP host/port-unreachable errors get delivered to
	our socket as ECONNREFUSED write errors.  On SV kernels, we lose; we'll have
	to collect and analyze raw ICMP ourselves a la satan's probe_udp_ports
	backend.  Guess where one could swipe the appropriate code from...

	Use the time delay between writes if given, otherwise use the "tcp ping"
	trick for getting the RTT.  [I got that idea from pluvius, and warped it.]
	Return either the original fd, or clean up and return -1. */
int udptest(int fd, IA * where);

#ifdef INET6
/* doconnect6 :
   do all the socket stuff, and return an fd for one of
	an open outbound TCP connection
	a UDP stub-socket thingie
   with appropriate socket options set up if we wanted source-routing, or
	an unconnected TCP or UDP socket to listen on.
   Examines various global o_blah flags to figure out what-all to do. */
int doconnect6(IA6 *rad, USHORT rp, IA6 *lad, USHORT lp);

/* dolisten6 :
   just like doconnect6, and in fact calls a hunk of doconnect6, but listens for
   incoming and returns an open connection *from* someplace.  If we were
   given host/port args, any connections from elsewhere are rejected.  This
   in conjunction with local-address binding should limit things nicely... */
int dolisten6(IA6 *rad, USHORT rp, IA6 *lad, USHORT lp);

/* udptest :
   fire a couple of packets at a UDP target port, just to see if it's really
   there.  On BSD kernels, ICMP host/port-unreachable errors get delivered to
   our socket as ECONNREFUSED write errors.  On SV kernels, we lose; we'll have
   to collect and analyze raw ICMP ourselves a la satan's probe_udp_ports
   backend.  Guess where one could swipe the appropriate code from...

   Use the time delay between writes if given, otherwise use the "tcp ping"
   trick for getting the RTT.  [I got that idea from pluvius, and warped it.]
   Return either the original fd, or clean up and return -1. */
udptest6(int fd, IA6 *where);

#endif

/* oprint :
   Hexdump bytes shoveled either way to a running logfile, in the format:
D offset       -  - - - --- 16 bytes --- - - -  -     # .... ascii .....
   where "which" sets the direction indicator, D:
	0 -- sent to network, or ">"
	1 -- rcvd and printed to stdout, or "<"
   and "buf" and "n" are data-block and length.  If the current block generates
   a partial line, so be it; we *want* that lockstep indication of who sent
   what when.  Adapted from dgaudet's original example -- but must be ripping
   *fast*, since we don't want to be too disk-bound... */
void oprint(int which, char * buf, int n);

#ifdef TELNET
/* atelnet :
   Answer anything that looks like telnet negotiation with don't/won't.
   This doesn't modify any data buffers, update the global output count,
   or show up in a hexdump -- it just shits into the outgoing stream.
   Idea and codebase from Mudge@l0pht.com. */
void atelnet(unsigned char * buf, unsigned int size);

#endif /* TELNET */

/* readwrite :
   handle stdin/stdout/network I/O.  Bwahaha!! -- the select loop from hell.
   In this instance, return what might become our exit status. */
int readwrite(int fd);

/* unescape :
   translate \-'s into -'s, returns start */
char * unescape(char *start);

#ifdef HAVE_HELP		/* unless we wanna be *really* cryptic */
/* helpme :
   the obvious */
void helpme(void);

#endif /* HAVE_HELP */

/* None genuine without this seal!  _H*/

int netcatProcess(int argc, char ** argv);

#endif 