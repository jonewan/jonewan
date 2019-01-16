#include "netcat.h"		

/* globals: */
#ifdef POSIX_SETJMP
sigjmp_buf jbuf;		/* timer crud */
#else
jmp_buf jbuf;			/* timer crud */
#endif
int jval = 0;			/* timer crud */
int netfd = -1;
int ofd = 0;			/* hexdump output fd */

#ifdef HAVE_BIND
extern int h_errno;
/* stolen almost wholesale from bsd herror.c */
static char * h_errs[] = {
  "Error 0",				/* but we *don't* use this */
  "Unknown host",			/* 1 HOST_NOT_FOUND */
  "Host name lookup failure",		/* 2 TRY_AGAIN */
  "Unknown server error",		/* 3 NO_RECOVERY */
  "No address associated with name",	/* 4 NO_ADDRESS */
};
#else
int h_errno;			/* just so we *do* have it available */
#endif /* HAVE_BIND */
int gatesidx = 0;		/* LSRR hop count */
int gatesptr = 4;		/* initial LSRR pointer, settable */
USHORT Single = 1;		/* zero if scanning */
unsigned int insaved = 0;	/* stdin-buffer size for multi-mode */
unsigned int wrote_out = 0;	/* total stdout bytes */
unsigned int wrote_net = 0;	/* total net bytes */

/* will malloc up the following globals: */
struct timeval * timer1 = NULL;
struct timeval * timer2 = NULL;
#ifdef INET6
SAI6 * lclend6 = NULL;		/* sockaddr_in6 structs */
SAI6 * remend6 = NULL;
#endif
SAI * lclend = NULL;		/* sockaddr_in structs */
SAI * remend = NULL;
HINF ** gates = NULL;		/* LSRR hop hostpoop */
char * optbuf = NULL;		/* LSRR or sockopts */
char * bigbuf_in = NULL;		/* data buffers */
char * bigbuf_net = NULL;
fd_set * ding1 = NULL;			/* for select loop */
fd_set * ding2 = NULL;
PINF * portpoop = NULL;		/* for getportpoop / getservby* */
unsigned char * stage = NULL;	/* hexdump line buffer */

/* global cmd flags: */
USHORT o_alla = 0;
USHORT o_allowbroad = 0;
unsigned int o_interval = 0;
USHORT o_listen = 0;
USHORT o_nflag = 1;//默认不指定dns
USHORT o_wfile = 0;
USHORT o_udpmode = 0;
USHORT o_verbose = 0;
USHORT o_holler_stderr = 1;
unsigned int o_wait = 0;
USHORT o_zero = 0;
int o_quit = 1; /* 0 == quit-now; >0 == quit after o_quit seconds */
/* o_tn in optional section */

const char wrote_txt[] = " sent %d, rcvd %d";
static char hexnibs[20] = "0123456789abcdef  ";

static char unknown[] = "(UNKNOWN)";
static char p_tcp[] = "tcp";	/* for getservby* */
static char p_udp[] = "udp";

/* support routines -- the bulk of this thing.  Placed in such an order that
   we don't have to forward-declare anything: */

/* holler :
   fake varargs -- need to do this way because we wind up calling through
   more levels of indirection than vanilla varargs can handle, and not all
   machines have vfprintf/vsyslog/whatever!  6 params oughta be enough. */
void holler(str, p1, p2, p3, p4, p5, p6)
char * str;
char * p1, *p2, *p3, *p4, *p5, *p6;
{
  FILE *o_holler_out = (o_holler_stderr ? stderr : stdout);//标准输出还是标准错误？其实都是屏幕
  if (o_verbose) {
    fprintf (o_holler_out, str, p1, p2, p3, p4, p5, p6);
#ifdef HAVE_BIND
    if (h_errno) {		/* if host-lookup variety of error ... */
      if (h_errno > 4)		/* oh no you don't, either */
	fprintf (o_holler_out, "preposterous h_errno: %d", h_errno);
      else
	fprintf (o_holler_out, h_errs[h_errno]);	/* handle it here */
      h_errno = 0;				/* and reset for next call */
    }
#endif
    if (errno) {		/* this gives funny-looking messages, but */
      perror (" ");		/* it's more portable than sys_errlist[]... */
    } else			/* xxx: do something better?  */
      fprintf (o_holler_out, "\n");
    fflush (o_holler_out);
  }
} /* holler */

/*free malloc bufs*/
static void freeBufs(void) {
	if (timer1) { free(timer1); timer1 = NULL; }
	if (timer2) { free(timer2); timer2 = NULL; }
#ifdef INET6
	if (lclend6) { free(lclend6); lclend6 = NULL; }
	if (remend6) { free(remend6); remend6 = NULL; }
#endif
	if (lclend) { free(lclend); lclend = NULL; }
	if (remend) { free(remend); remend = NULL; }
	if (gates) { free(gates); gates = NULL; }
	if (optbuf) { free(optbuf); optbuf = NULL; }
	if (bigbuf_in) { free(bigbuf_in); bigbuf_in = NULL; }
	if (bigbuf_net) { free(bigbuf_net); bigbuf_net = NULL; }
	if (ding1) { free(ding1); ding1 = NULL; }
	if (ding2) { free(ding2); ding2 = NULL; }
	if (portpoop) { free(portpoop); portpoop = NULL; }
	if (stage) { free(stage); stage = NULL; }
}

/* bail :
   error-exit handler, callable from anywhere */
void bail(str, p1, p2, p3, p4, p5, p6)//错误退出的处理函数
char * str;
char * p1, *p2, *p3, *p4, *p5, *p6;
{
  o_verbose = 1;
  holler (str, p1, p2, p3, p4, p5, p6);
  close (netfd);
  freeBufs();
  exit (1);
} /* bail */

/* catch :
   no-brainer interrupt handler */
void catch (void)//无脑中断函数
{
  errno = 0;
  if (o_verbose > 1)		/* normally we don't care */
    bail (wrote_txt, wrote_net, wrote_out);
  bail ("");
}

/* quit :
   handler for a "-q" timeout (exit 0 instead of 1) */
void quit(void)
{
	close(netfd);
	freeBufs();

	exit(0);
}

/* timeout and other signal handling cruft */
void tmtravel(void)
{
  signal (SIGALRM, SIG_IGN);
  alarm (0);
  if (jval == 0)
    bail ("spurious timer interrupt!");
#ifdef POSIX_SETJMP
  siglongjmp (jbuf, jval);
#else
  longjmp (jbuf, jval);
#endif
}

/* arm_timer :
   set the timer.  Zero secs arg means unarm */
void arm_timer(unsigned int num, unsigned int secs)
{
  if (secs == 0) {			/* reset */
    signal (SIGALRM, SIG_IGN);
    alarm (0);
    jval = 0;
  } else {				/* set */
    signal (SIGALRM, tmtravel);
    alarm (secs);
    jval = num;
  } /* if secs */
} /* arm_timer */

/* Hmalloc :
   malloc up what I want, rounded up to *4, and pre-zeroed.  Either succeeds
   or bails out on its own, so that callers don't have to worry about it. */
char * Hmalloc(unsigned int size)
{
  unsigned int s = (size + 4) & 0xfffffffc;	/* 4GB?! */
  char * p = malloc (s);
  if (p != NULL)
    memset (p, 0, s);
  else
    bail ("Hmalloc %d failed", s);
  return (p);
} /* Hmalloc */

/* findline :
   find the next newline in a buffer; return inclusive size of that "line",
   or the entire buffer size, so the caller knows how much to then write().
   Not distinguishing \n vs \r\n for the nonce; it just works as is... */
unsigned int findline(char * buf, unsigned int siz)
{
  register char * p;
  register int x;
  if (! buf)			/* various sanity checks... */
    return (0);
  if (siz > BIGSIZ)
    return (0);
  x = siz;
  for (p = buf; x > 0; x--) {
    if (*p == '\n') {
      x = (int) (p - buf);
      x++;			/* 'sokay if it points just past the end! */
Debug (("findline returning %d", x))
      return (x);
    }
    p++;
  } /* for */
Debug (("findline returning whole thing: %d", siz))
  return (siz);
} /* findline */

/* comparehosts :
   cross-check the host_poop we have so far against new gethostby*() info,
   and holler about mismatches.  Perhaps gratuitous, but it can't hurt to
   point out when someone's DNS is fukt.  Returns 1 if mismatch, in case
   someone else wants to do something about it. */
int comparehosts(HINF * poop, struct hostent * hp)
{
  errno = 0;
  h_errno = 0;
/* The DNS spec is officially case-insensitive, but for those times when you
   *really* wanna see any and all discrepancies, by all means define this. */
#ifdef ANAL			
  if (strcmp (poop->name, hp->h_name) != 0) {		/* case-sensitive */
#else
  if (strcasecmp (poop->name, hp->h_name) != 0) {	/* normal */
#endif
    holler ("DNS fwd/rev mismatch: %s != %s", poop->name, hp->h_name);
    return (1);
  }
  return (0);
/* ... do we need to do anything over and above that?? */
} /* comparehosts */

#ifdef INET6
char *inet_ntoa6(struct in6_addr *s)
{
  static char buf[1024];

  if (IN6_IS_ADDR_V4MAPPED(s))
    inet_ntop(AF_INET, s+12, buf, sizeof(buf));
  else
    inet_ntop(AF_INET6, s, buf, sizeof(buf));

  return buf;
}

int comparehosts6(HINF6 * poop, struct hostent * hp)
{
  errno = 0;
  h_errno = 0;
/* The DNS spec is officially case-insensitive, but for those times when you
   *really* wanna see any and all discrepancies, by all means define this. */
#ifdef ANAL			
  if (strcmp (poop->name, hp->h_name) != 0) {		/* case-sensitive */
#else
  if (strcasecmp (poop->name, hp->h_name) != 0) {	/* normal */
#endif
    holler ("DNS fwd/rev mismatch: %s != %s", poop->name, hp->h_name);
    return (1);
  }
  return (0);
/* ... do we need to do anything over and above that?? */
} /* comparehosts */
#endif

/* gethostpoop :
   resolve a host 8 ways from sunday; return a new host_poop struct with its
   info.  The argument can be a name or [ascii] IP address; it will try its
   damndest to deal with it.  "numeric" governs whether we do any DNS at all,
   and we also check o_verbose for what's appropriate work to do. */
HINF * gethostpoop(char * name, USHORT numeric)
{
  struct hostent * hostent;
  struct in_addr iaddr;
  register HINF * poop = NULL;
  register int x;
  int rc;

/* I really want to strangle the twit who dreamed up all these sockaddr and
   hostent abstractions, and then forced them all to be incompatible with
   each other so you *HAVE* to do all this ridiculous casting back and forth.
   If that wasn't bad enough, all the doc insists on referring to local ports
   and addresses as "names", which makes NO sense down at the bare metal.

   What an absolutely horrid paradigm, and to think of all the people who
   have been wasting significant amounts of time fighting with this stupid
   deliberate obfuscation over the last 10 years... then again, I like
   languages wherein a pointer is a pointer, what you put there is your own
   business, the compiler stays out of your face, and sheep are nervous.
   Maybe that's why my C code reads like assembler half the time... */

/* If we want to see all the DNS stuff, do the following hair --
   if inet_addr, do reverse and forward with any warnings; otherwise try
   to do forward and reverse with any warnings.  In other words, as long
   as we're here, do a complete DNS check on these clowns.  Yes, it slows
   things down a bit for a first run, but once it's cached, who cares? */

  errno = 0;
  h_errno = 0;
  if (name)
    poop = (HINF *) Hmalloc (sizeof (HINF));
  if (! poop)
    bail ("gethostpoop fuxored");
  strcpy (poop->name, unknown);		/* preload it */
/* see wzv:workarounds.c for dg/ux return-a-struct inet_addr lossage */
  rc = inet_aton(name, &iaddr);

  if (rc == 0) {	/* here's the great split: names... */
    if (numeric)
      bail ("Can't parse %s as an IP address", name);
    hostent = gethostbyname (name);
    if (! hostent)
/* failure to look up a name is fatal, since we can't do anything with it */
      bail ("%s: forward host lookup failed: ", name);
    strncpy (poop->name, hostent->h_name, MAXHOSTNAMELEN - 2);
    for (x = 0; hostent->h_addr_list[x] && (x < 8); x++) {
      memcpy (&poop->iaddrs[x], hostent->h_addr_list[x], sizeof (IA));
      strncpy (poop->addrs[x], inet_ntoa (poop->iaddrs[x]),
	sizeof (poop->addrs[0]));
    } /* for x -> addrs, part A */
    if (! o_verbose)			/* if we didn't want to see the */
      return (poop);			/* inverse stuff, we're done. */
/* do inverse lookups in separate loop based on our collected forward addrs,
   since gethostby* tends to crap into the same buffer over and over */
    for (x = 0; poop->iaddrs[x].s_addr && (x < 8); x++) {
      hostent = gethostbyaddr ((char *)&poop->iaddrs[x],
				sizeof (IA), AF_INET);
      if ((! hostent) || (! hostent-> h_name))
	holler ("Warning: inverse host lookup failed for %s: ", poop->addrs[x]);
      else
	(void) comparehosts (poop, hostent);
    } /* for x -> addrs, part B */

  } else {  /* not INADDR_NONE: numeric addresses... */
    memcpy (poop->iaddrs, &iaddr, sizeof (IA));
    strncpy (poop->addrs[0], inet_ntoa (iaddr), sizeof (poop->addrs));
    if (numeric)			/* if numeric-only, we're done */
      return (poop);
    if (! o_verbose)			/* likewise if we don't want */
      return (poop);			/* the full DNS hair */
    hostent = gethostbyaddr ((char *) &iaddr, sizeof (IA), AF_INET);
/* numeric or not, failure to look up a PTR is *not* considered fatal */
    if (! hostent)
	holler ("%s: inverse host lookup failed: ", name);
    else {
	strncpy (poop->name, hostent->h_name, MAXHOSTNAMELEN - 2);
	hostent = gethostbyname (poop->name);
	if ((! hostent) || (! hostent->h_addr_list[0]))
	  holler ("Warning: forward host lookup failed for %s: ", poop->name);
	else
	  (void) comparehosts (poop, hostent);
    } /* if hostent */
  } /* INADDR_NONE Great Split */

/* whatever-all went down previously, we should now have a host_poop struct
   with at least one IP address in it. */
  h_errno = 0;
  return (poop);
} /* gethostpoop */

#ifdef INET6
HINF6 *gethost6poop(char *name, USHORT numeric)
{
	struct hostent *hostent;
	struct in6_addr iaddr;
	register HINF6 *poop = NULL;
	register int x;

/* If we want to see all the DNS stuff, do the following hair --
   if inet_addr, do reverse and forward with any warnings; otherwise try
   to do forward and reverse with any warnings.  In other words, as long
   as we're here, do a complete DNS check on these clowns.  Yes, it slows
   things down a bit for a first run, but once it's cached, who cares? */

	errno = 0;
	h_errno = 0;
	if (name)
		poop = (HINF6 *) Hmalloc(sizeof(HINF6));
	if (!poop)
		bail("gethost6poop fuxored");
	strcpy(poop->name, unknown);	/* preload it */

	if (! inet_pton(AF_INET6, name, &iaddr)) {	/* here's the great split: names... */
		if (numeric)
			bail("Can't parse %s as an IP address", name);
		hostent = gethostbyname2(name, AF_INET6);
		if (!hostent)
/* failure to look up a name is fatal, since we can't do anything with it */
			bail("%s: forward host lookup failed: ", name);
		strncpy(poop->name, hostent->h_name, MAXHOSTNAMELEN - 2);
		for (x = 0; hostent->h_addr_list[x] && (x < 8); x++) {
			memcpy(&poop->iaddrs[x], hostent->h_addr_list[x],
			       sizeof(IA6));
			strncpy(poop->addrs[x], inet_ntoa6(&poop->iaddrs[x]),
				sizeof(poop->addrs[0]));
		}		/* for x -> addrs, part A */
		if (!o_verbose)	/* if we didn't want to see the */
			return (poop);	/* inverse stuff, we're done. */
/* do inverse lookups in separate loop based on our collected forward addrs,
   since gethostby* tends to crap into the same buffer over and over */
		for (x = 0; !IN6_IS_ADDR_UNSPECIFIED(&poop->iaddrs[x]) && (x < 8); x++) {
			hostent = gethostbyaddr((char *) &poop->iaddrs[x],
						sizeof(IA6), AF_INET6);
			if ((!hostent) || (!hostent->h_name))
				holler
				    ("Warning: inverse host lookup failed for %s: ", poop->addrs[x]);
			else
				(void) comparehosts6(poop, hostent);
		}		/* for x -> addrs, part B */

	} else {		/* not INADDR_NONE: numeric addresses... */
		inet_ntop(AF_INET6, &iaddr, poop->addrs[0], sizeof(poop->addrs[0]));
		memcpy(poop->iaddrs, &iaddr, sizeof(IA6));
		if (numeric)	/* if numeric-only, we're done */
			return (poop);
		if (!o_verbose)	/* likewise if we don't want */
			return (poop);	/* the full DNS hair */
		hostent =
		    gethostbyaddr((char *) &iaddr, sizeof(IA6), AF_INET6);
/* numeric or not, failure to look up a PTR is *not* considered fatal */
		if (!hostent)
			holler("%s: inverse host lookup failed: ", name);
		else {
			strncpy(poop->name, hostent->h_name,
				MAXHOSTNAMELEN - 2);
			hostent = gethostbyname2(poop->name, AF_INET6);
			if ((!hostent) || (!hostent->h_addr_list[0]))
				holler
				    ("Warning: forward host lookup failed for %s: ", poop->name);
			else
				(void) comparehosts6(poop, hostent);
		}		/* if hostent */
	}			/* INADDR_NONE Great Split */

/* whatever-all went down previously, we should now have a host_poop struct
   with at least one IP address in it. */
	h_errno = 0;
	return (poop);
}				/* gethost6poop */
#endif

/* getportpoop :
   Same general idea as gethostpoop -- look up a port in /etc/services, fill
   in global port_poop, but return the actual port *number*.  Pass ONE of:
	pstring to resolve stuff like "23" or "exec";
	pnum to reverse-resolve something that's already a number.
   If o_nflag is on, fill in what we can but skip the getservby??? stuff.
   Might as well have consistent behavior here, and it *is* faster. */
USHORT getportpoop(char * pstring, unsigned int pnum)
{
  struct servent * servent;
  register int x;
  register int y;
  char * whichp = p_tcp;
  if (o_udpmode)
    whichp = p_udp;
  portpoop->name[0] = '?';		/* fast preload */
  portpoop->name[1] = '\0';

/* case 1: reverse-lookup of a number; placed first since this case is much
   more frequent if we're scanning */
  if (pnum) {
    if (pstring)			/* one or the other, pleeze */
      return (0);
    x = pnum;
    /* disabled, see bug #98902. if this is *really* slowing someone
     * down I'll reconsider. */
    /* if (o_nflag) */			/* go faster, skip getservbyblah */
      /* goto gp_finish; */
    y = htons (x);			/* gotta do this -- see Fig.1 below */
    servent = getservbyport (y, whichp);
    if (servent) {
      y = ntohs (servent->s_port);
      if (x != y)			/* "never happen" */
	holler ("Warning: port-bynum mismatch, %d != %d", x, y);
      strncpy (portpoop->name, servent->s_name, sizeof (portpoop->name));
    } /* if servent */
    goto gp_finish;
  } /* if pnum */

/* case 2: resolve a string, but we still give preference to numbers instead
   of trying to resolve conflicts.  None of the entries in *my* extensive
   /etc/services begins with a digit, so this should "always work" unless
   you're at 3com and have some company-internal services defined... */
  if (pstring) {
    if (pnum)				/* one or the other, pleeze */
      return (0);
    x = atoi (pstring);
    if (x)
      return (getportpoop (NULL, x));	/* recurse for numeric-string-arg */
    if (o_nflag)			/* can't use names! */
      return (0);
    servent = getservbyname (pstring, whichp);
    if (servent) {
      strncpy (portpoop->name, servent->s_name, sizeof (portpoop->name));
      x = ntohs (servent->s_port);
      goto gp_finish;
    } /* if servent */
  } /* if pstring */

  return (0);				/* catches any problems so far */

/* Obligatory netdb.h-inspired rant: servent.s_port is supposed to be an int.
   Despite this, we still have to treat it as a short when copying it around.
   Not only that, but we have to convert it *back* into net order for
   getservbyport to work.  Manpages generally aren't clear on all this, but
   there are plenty of examples in which it is just quietly done.  More BSD
   lossage... since everything getserv* ever deals with is local to our own
   host, why bother with all this network-order/host-order crap at all?!
   That should be saved for when we want to actually plug the port[s] into
   some real network calls -- and guess what, we have to *re*-convert at that
   point as well.  Fuckheads. */

gp_finish:
/* Fall here whether or not we have a valid servent at this point, with
   x containing our [host-order and therefore useful, dammit] port number */
  sprintf (portpoop->anum, "%d", x);	/* always load any numeric specs! */
  portpoop->num = (x & 0xffff);		/* ushort, remember... */
  return (portpoop->num);
} /* getportpoop */

/* nextport :
   Come up with the next port to try, be it random or whatever.  "block" is
   a ptr to randports array, whose bytes [so far] carry these meanings:
	0	ignore
	1	to be tested
	2	tested [which is set as we find them here]
   returns a USHORT random port, or 0 if all the t-b-t ones are used up. */
USHORT nextport(char * block)
{
  register unsigned int x;
  register unsigned int y;

  y = 70000;			/* high safety count for rnd-tries */
  while (y > 0) {
    x = (RAND() & 0xffff);
    if (block[x] == 1) {	/* try to find a not-done one... */
      block[x] = 2;
      break;
    }
    x = 0;			/* bummer. */
    y--;
  } /* while y */
  if (x)
    return (x);

  y = 65535;			/* no random one, try linear downsearch */
  while (y > 0) {		/* if they're all used, we *must* be sure! */
    if (block[y] == 1) {
      block[y] = 2;
      break;
    }
    y--;
  } /* while y */
  if (y)
    return (y);			/* at least one left */

  return (0);			/* no more left! */
} /* nextport */

/* loadports :
   set "to be tested" indications in BLOCK, from LO to HI.  Almost too small
   to be a separate routine, but makes main() a little cleaner... */
void loadports(char * block, USHORT lo, USHORT hi)
{
  USHORT x;

  if (! block)
    bail ("loadports: no block?!");
  if ((! lo) || (! hi))
    bail ("loadports: bogus values %d, %d", lo, hi);
  x = hi;
  while (lo <= x) {
    block[x] = 1;
    x--;
  }
} /* loadports */

#ifdef GAPING_SECURITY_HOLE
char * pr00gie = NULL;			/* global ptr to -e arg */
int doexec_use_sh = 0;			/* `-c' or `-e' option? */

/* doexec_new :
   fiddle all the file descriptors around, and hand off to another prog.  Sort
   of like a one-off "poor man's inetd".  This is the only section of code
   that would be security-critical, which is why it's ifdefed out by default.
   Use at your own hairy risk; if you leave shells lying around behind open
   listening ports you deserve to lose!! */
doexec_new(int fd)
{
  dup2 (fd, 0);				/* the precise order of fiddlage */
  close (fd);				/* is apparently crucial; this is */
  dup2 (0, 1);				/* swiped directly out of "inetd". */
  dup2 (0, 2);

  /* A POSIX-conformant system must have `/bin/sh'. */
Debug (("gonna exec \"%s\" using /bin/sh...", pr00gie))
  execl ("/bin/sh", "sh", "-c", pr00gie, NULL);
  bail ("exec %s failed", pr00gie);	/* this gets sent out.  Hmm... */
} /* doexec_new */

/* doexec :
   fiddle all the file descriptors around, and hand off to another prog.  Sort
   of like a one-off "poor man's inetd".  This is the only section of code
   that would be security-critical, which is why it's ifdefed out by default.
   Use at your own hairy risk; if you leave shells lying around behind open
   listening ports you deserve to lose!! */
doexec(int fd)
{
  register char * p;

  dup2 (fd, 0);				/* the precise order of fiddlage */
  close (fd);				/* is apparently crucial; this is */
  dup2 (0, 1);				/* swiped directly out of "inetd". */
  dup2 (0, 2);
  p = strrchr (pr00gie, '/');		/* shorter argv[0] */
  if (p)
    p++;
  else
    p = pr00gie;
Debug (("gonna exec %s as %s...", pr00gie, p))
  execl (pr00gie, p, NULL);
  bail ("exec %s failed", pr00gie);	/* this gets sent out.  Hmm... */
} /* doexec */
#endif /* GAPING_SECURITY_HOLE */

/* doconnect :
   do all the socket stuff, and return an fd for one of
	an open outbound TCP connection
	a UDP stub-socket thingie
   with appropriate socket options set up if we wanted source-routing, or
	an unconnected TCP or UDP socket to listen on.
   Examines various global o_blah flags to figure out what-all to do. */
int doconnect(IA * rad, USHORT rp, IA * lad, USHORT lp)
{
  register int nnetfd;
  register int rr;
  int x, y;
  errno = 0;

/* grab a socket; set opts */
newskt:
  if (o_udpmode)
    nnetfd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  else
    nnetfd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (nnetfd < 0)
    bail ("Can't get socket");
  if (nnetfd == 0)		/* if stdin was closed this might *be* 0, */
    goto newskt;		/* so grab another.  See text for why... */
  x = 1;
  rr = setsockopt (nnetfd, SOL_SOCKET, SO_REUSEADDR, &x, sizeof (x));
  if (rr == -1)
    holler ("nnetfd reuseaddr failed");		/* ??? */
#ifdef SO_BROADCAST
  if (o_allowbroad) {
    rr = setsockopt (nnetfd, SOL_SOCKET, SO_BROADCAST, &x, sizeof (x));
    if (rr == -1)
       holler ("nnetfd reuseaddr failed");         /* ??? */
  }
#endif
#ifdef SO_REUSEPORT	/* doesnt exist everywhere... */
  rr = setsockopt (nnetfd, SOL_SOCKET, SO_REUSEPORT, &x, sizeof (x));
  if (rr == -1)
    holler ("nnetfd reuseport failed");		/* ??? */
#endif
#if 0
/* If you want to screw with RCVBUF/SNDBUF, do it here.  Liudvikas Bukys at
   Rochester sent this example, which would involve YET MORE options and is
   just archived here in case you want to mess with it.  o_xxxbuf are global
   integers set in main() getopt loop, and check for rr == 0 afterward. */
  rr = setsockopt(nnetfd, SOL_SOCKET, SO_RCVBUF, &o_rcvbuf, sizeof o_rcvbuf);
  rr = setsockopt(nnetfd, SOL_SOCKET, SO_SNDBUF, &o_sndbuf, sizeof o_sndbuf);
#endif
  
  /* fill in all the right sockaddr crud */
    lclend->sin_family = AF_INET;

/* fill in all the right sockaddr crud */
  lclend->sin_family = AF_INET;
  remend->sin_family = AF_INET;

/* if lad/lp, do appropriate binding */
  if (lad)
    memcpy (&lclend->sin_addr.s_addr, lad, sizeof (IA));
  if (lp)
    lclend->sin_port = htons (lp);
  rr = 0;
  if (lad || lp) {
    x = (int) lp;
/* try a few times for the local bind, a la ftp-data-port... */
    for (y = 4; y > 0; y--) {
      rr = bind (nnetfd, (SA *)lclend, sizeof (SA));
      if (rr == 0)
	break;
      if (errno != EADDRINUSE)
	break;
      else {
	holler ("retrying local %s:%d", inet_ntoa (lclend->sin_addr), lp);
	sleep (2);
	errno = 0;			/* clear from sleep */
      } /* if EADDRINUSE */
    } /* for y counter */
  } /* if lad or lp */
  if (rr)
    bail ("Can't grab %s:%d with bind",
	inet_ntoa(lclend->sin_addr), lp);

  if (o_listen)
    return (nnetfd);			/* thanks, that's all for today */

  memcpy (&remend->sin_addr.s_addr, rad, sizeof (IA));
  remend->sin_port = htons (rp);

/* rough format of LSRR option and explanation of weirdness.
Option comes after IP-hdr dest addr in packet, padded to *4, and ihl > 5.
IHL is multiples of 4, i.e. real len = ip_hl << 2.
	type 131	1	; 0x83: copied, option class 0, number 3
	len		1	; of *whole* option!
	pointer		1	; nxt-hop-addr; 1-relative, not 0-relative
	addrlist...	var	; 4 bytes per hop-addr
	pad-to-32	var	; ones, i.e. "NOP"

If we want to route A -> B via hops C and D, we must add C, D, *and* B to the
options list.  Why?  Because when we hand the kernel A -> B with list C, D, B
the "send shuffle" inside the kernel changes it into A -> C with list D, B and
the outbound packet gets sent to C.  If B wasn't also in the hops list, the
final destination would have been lost at this point.

When C gets the packet, it changes it to A -> D with list C', B where C' is
the interface address that C used to forward the packet.  This "records" the
route hop from B's point of view, i.e. which address points "toward" B.  This
is to make B better able to return the packets.  The pointer gets bumped by 4,
so that D does the right thing instead of trying to forward back to C.

When B finally gets the packet, it sees that the pointer is at the end of the
LSRR list and is thus "completed".  B will then try to use the packet instead
of forwarding it, i.e. deliver it up to some application.

Note that by moving the pointer yourself, you could send the traffic directly
to B but have it return via your preconstructed source-route.  Playing with
this and watching "tcpdump -v" is the best way to understand what's going on.

Only works for TCP in BSD-flavor kernels.  UDP is a loss; udp_input calls
stripoptions() early on, and the code to save the srcrt is notdef'ed.
Linux is also still a loss at 1.3.x it looks like; the lsrr code is { }...
*/

/* if any -g arguments were given, set up source-routing.  We hit this after
   the gates are all looked up and ready to rock, any -G pointer is set,
   and gatesidx is now the *number* of hops */
  if (gatesidx) {		/* if we wanted any srcrt hops ... */
/* don't even bother compiling if we can't do IP options here! */
#ifdef IP_OPTIONS
    if (! optbuf) {		/* and don't already *have* a srcrt set */
      char * opp;		/* then do all this setup hair */
      optbuf = Hmalloc (48);
      opp = optbuf;
      *opp++ = IPOPT_LSRR;					/* option */
      *opp++ = (char)
	(((gatesidx + 1) * sizeof (IA)) + 3) & 0xff;		/* length */
      *opp++ = gatesptr;					/* pointer */
/* opp now points at first hop addr -- insert the intermediate gateways */
      for ( x = 0; x < gatesidx; x++) {
	memcpy (opp, gates[x]->iaddrs, sizeof (IA));
	opp += sizeof (IA);
      }
/* and tack the final destination on the end [needed!] */
      memcpy (opp, rad, sizeof (IA));
      opp += sizeof (IA);
      *opp = IPOPT_NOP;			/* alignment filler */
    } /* if empty optbuf */
/* calculate length of whole option mess, which is (3 + [hops] + [final] + 1),
   and apply it [have to do this every time through, of course] */
    x = ((gatesidx + 1) * sizeof (IA)) + 4;
    rr = setsockopt (nnetfd, IPPROTO_IP, IP_OPTIONS, optbuf, x);
    if (rr == -1)
      bail ("srcrt setsockopt fuxored");
#else /* IP_OPTIONS */
    holler ("Warning: source routing unavailable on this machine, ignoring");
#endif /* IP_OPTIONS*/
  } /* if gatesidx */

/* wrap connect inside a timer, and hit it */
  arm_timer (1, o_wait);
#ifdef POSIX_SETJMP
  if (sigsetjmp (jbuf,1) == 0) {
    rr = connect (nnetfd, (SA *)remend, sizeof (SA));
  } else {				/* setjmp: connect failed... */
    rr = -1;
    errno = ETIMEDOUT;			/* fake it */
  }
#else
  if (setjmp (jbuf) == 0) {
    rr = connect (nnetfd, (SA *)remend, sizeof (SA));
  } else {				/* setjmp: connect failed... */
    rr = -1;
    errno = ETIMEDOUT;			/* fake it */
  }
#endif
  arm_timer (0, 0);
  if (rr == 0)
    return (nnetfd);
  close (nnetfd);			/* clean up junked socket FD!! */
  return (-1);
} /* doconnect */

/* dolisten :
   just like doconnect, and in fact calls a hunk of doconnect, but listens for
   incoming and returns an open connection *from* someplace.  If we were
   given host/port args, any connections from elsewhere are rejected.  This
   in conjunction with local-address binding should limit things nicely... */
int dolisten(IA * rad, USHORT rp, IA * lad, USHORT lp)
{
  register int nnetfd;
  register int rr;
  HINF * whozis = NULL;
  int x;
  char * cp;
  USHORT z;
  errno = 0;

/* Pass everything off to doconnect, who in o_listen mode just gets a socket */
  nnetfd = doconnect (rad, rp, lad, lp);
  if (nnetfd <= 0)
    return (-1);
  if (o_udpmode) {			/* apparently UDP can listen ON */
    if (! lp)				/* "port 0",  but that's not useful */
      bail ("UDP listen needs -p arg");
  } else {
    rr = listen (nnetfd, 1);		/* gotta listen() before we can get */
    if (rr < 0)				/* our local random port.  sheesh. */
      bail ("local listen fuxored");
  }

/* Various things that follow temporarily trash bigbuf_net, which might contain
   a copy of any recvfrom()ed packet, but we'll read() another copy later. */

/* I can't believe I have to do all this to get my own goddamn bound address
   and port number.  It should just get filled in during bind() or something.
   All this is only useful if we didn't say -p for listening, since if we
   said -p we *know* what port we're listening on.  At any rate we won't bother
   with it all unless we wanted to see it, although listening quietly on a
   random unknown port is probably not very useful without "netstat". */
  if (o_verbose) {
    x = sizeof (SA);		/* how 'bout getsockNUM instead, pinheads?! */
    rr = getsockname (nnetfd, (SA *) lclend, &x);
    if (rr < 0)
      holler ("local getsockname failed");
    strcpy (bigbuf_net, "listening on [");	/* buffer reuse... */
    if (lclend->sin_addr.s_addr)
      strcat (bigbuf_net, inet_ntoa (lclend->sin_addr));
    else
      strcat (bigbuf_net, "any");
    strcat (bigbuf_net, "] %d ...");
    z = ntohs (lclend->sin_port);
    holler (bigbuf_net, z);
  } /* verbose -- whew!! */

/* UDP is a speeeeecial case -- we have to do I/O *and* get the calling
   party's particulars all at once, listen() and accept() don't apply.
   At least in the BSD universe, however, recvfrom/PEEK is enough to tell
   us something came in, and we can set things up so straight read/write
   actually does work after all.  Yow.  YMMV on strange platforms!  */
  if (o_udpmode) {
    x = sizeof (SA);		/* retval for recvfrom */
    arm_timer (2, o_wait);		/* might as well timeout this, too */
#ifdef POSIX_SETJMP
    if (sigsetjmp (jbuf,1) == 0) {	/* do timeout for initial connect */
#else
    if (setjmp (jbuf) == 0) {	/* do timeout for initial connect */
#endif
      rr = recvfrom		/* and here we block... */
	(nnetfd, bigbuf_net, BIGSIZ, MSG_PEEK, (SA *) remend, &x);
Debug (("dolisten/recvfrom ding, rr = %d, netbuf %s ", rr, bigbuf_net))
    } else
      goto dol_tmo;		/* timeout */
    arm_timer (0, 0);
/* I'm not completely clear on how this works -- BSD seems to make UDP
   just magically work in a connect()ed context, but we'll undoubtedly run
   into systems this deal doesn't work on.  For now, we apparently have to
   issue a connect() on our just-tickled socket so we can write() back.
   Again, why the fuck doesn't it just get filled in and taken care of?!
   This hack is anything but optimal.  Basically, if you want your listener
   to also be able to send data back, you need this connect() line, which
   also has the side effect that now anything from a different source or even a
   different port on the other end won't show up and will cause ICMP errors.
   I guess that's what they meant by "connect".
   Let's try to remember what the "U" is *really* for, eh? */
    rr = connect (nnetfd, (SA *)remend, sizeof (SA));
    goto whoisit;
  } /* o_udpmode */

/* fall here for TCP */
  x = sizeof (SA);		/* retval for accept */
  arm_timer (2, o_wait);		/* wrap this in a timer, too; 0 = forever */
#ifdef POSIX_SETJMP
  if (sigsetjmp (jbuf,1) == 0) {
    rr = accept (nnetfd, (SA *)remend, &x);
  } else
    goto dol_tmo;		/* timeout */
#else
  if (setjmp (jbuf) == 0) {
    rr = accept (nnetfd, (SA *)remend, &x);
  } else
    goto dol_tmo;		/* timeout */
#endif
  arm_timer (0, 0);
  close (nnetfd);		/* dump the old socket */
  nnetfd = rr;			/* here's our new one */

whoisit:
  if (rr < 0)
    goto dol_err;		/* bail out if any errors so far */

/* If we can, look for any IP options.  Useful for testing the receiving end of
   such things, and is a good exercise in dealing with it.  We do this before
   the connect message, to ensure that the connect msg is uniformly the LAST
   thing to emerge after all the intervening crud.  Doesn't work for UDP on
   any machines I've tested, but feel free to surprise me. */
#ifdef IP_OPTIONS
  if (! o_verbose)			/* if we wont see it, we dont care */
    goto dol_noop;
  optbuf = Hmalloc (40);
  x = 40;
  rr = getsockopt (nnetfd, IPPROTO_IP, IP_OPTIONS, optbuf, &x);
  if (rr < 0)
    holler ("getsockopt failed");
Debug (("ipoptions ret len %d", x))
  if (x) {				/* we've got options, lessee em... */
    unsigned char * q = (unsigned char *) optbuf;
    char * p = bigbuf_net;		/* local variables, yuk! */
    char * pp = &bigbuf_net[128];	/* get random space farther out... */
    memset (bigbuf_net, 0, 256);	/* clear it all first */
    while (x > 0) {
	sprintf (pp, "%2.2x ", *q);	/* clumsy, but works: turn into hex */
	strcat (p, pp);			/* and build the final string */
	q++; p++;
	x--;
    }
    holler ("IP options: %s", bigbuf_net);
  } /* if x, i.e. any options */
dol_noop:
#endif /* IP_OPTIONS */

/* find out what address the connection was *to* on our end, in case we're
   doing a listen-on-any on a multihomed machine.  This allows one to
   offer different services via different alias addresses, such as the
   "virtual web site" hack. */
  memset (bigbuf_net, 0, 64);
  cp = &bigbuf_net[32];
  x = sizeof (SA);
  rr = getsockname (nnetfd, (SA *) lclend, &x);
  if (rr < 0)
    holler ("post-rcv getsockname failed");
  strcpy (cp, inet_ntoa (lclend->sin_addr));

/* now check out who it is.  We don't care about mismatched DNS names here,
   but any ADDR and PORT we specified had better fucking well match the caller.
   Converting from addr to inet_ntoa and back again is a bit of a kludge, but
   gethostpoop wants a string and there's much gnarlier code out there already,
   so I don't feel bad.
   The *real* question is why BFD sockets wasn't designed to allow listens for
   connections *from* specific hosts/ports, instead of requiring the caller to
   accept the connection and then reject undesireable ones by closing.  In
   other words, we need a TCP MSG_PEEK. */
  z = ntohs (remend->sin_port);
  strcpy (bigbuf_net, inet_ntoa (remend->sin_addr));
  whozis = gethostpoop (bigbuf_net, o_nflag);
  errno = 0;
  x = 0;				/* use as a flag... */
  if (rad)	/* xxx: fix to go down the *list* if we have one? */
    if (memcmp (rad, whozis->iaddrs, sizeof (SA)))
      x = 1;
  if (rp)
    if (z != rp)
      x = 1;
  if (x)					/* guilty! */
    bail ("invalid connection to [%s] from %s [%s] %d",
	cp, whozis->name, whozis->addrs[0], z);
  holler ("connect to [%s] from %s [%s] %d",		/* oh, you're okay.. */
	cp, whozis->name, whozis->addrs[0], z);
  return (nnetfd);				/* open! */

dol_tmo:
  errno = ETIMEDOUT;			/* fake it */
dol_err:
  close (nnetfd);
  return (-1);
} /* dolisten */

/* udptest :
   fire a couple of packets at a UDP target port, just to see if it's really
   there.  On BSD kernels, ICMP host/port-unreachable errors get delivered to
   our socket as ECONNREFUSED write errors.  On SV kernels, we lose; we'll have
   to collect and analyze raw ICMP ourselves a la satan's probe_udp_ports
   backend.  Guess where one could swipe the appropriate code from...

   Use the time delay between writes if given, otherwise use the "tcp ping"
   trick for getting the RTT.  [I got that idea from pluvius, and warped it.]
   Return either the original fd, or clean up and return -1. */
int udptest(int fd, IA * where)
{
  register int rr;

  rr = write (fd, bigbuf_in, 1);
  if (rr != 1)
    holler ("udptest first write failed?! errno %d", errno);
  if (o_wait)
    sleep (o_wait);
  else {
/* use the tcp-ping trick: try connecting to a normally refused port, which
   causes us to block for the time that SYN gets there and RST gets back.
   Not completely reliable, but it *does* mostly work. */
    o_udpmode = 0;			/* so doconnect does TCP this time */
/* Set a temporary connect timeout, so packet filtration doesnt cause
   us to hang forever, and hit it */
    o_wait = 5;				/* enough that we'll notice?? */
    rr = doconnect (where, SLEAZE_PORT, 0, 0);
    if (rr > 0)
      close (rr);			/* in case it *did* open */
    o_wait = 0;				/* reset it */
    o_udpmode++;			/* we *are* still doing UDP, right? */
  } /* if o_wait */
  errno = 0;				/* clear from sleep */
  rr = write (fd, bigbuf_in, 1);
  if (rr == 1)				/* if write error, no UDP listener */
    return (fd);
  close (fd);				/* use it or lose it! */
  return (-1);
} /* udptest */

#ifdef INET6
/* doconnect6 :
   do all the socket stuff, and return an fd for one of
	an open outbound TCP connection
	a UDP stub-socket thingie
   with appropriate socket options set up if we wanted source-routing, or
	an unconnected TCP or UDP socket to listen on.
   Examines various global o_blah flags to figure out what-all to do. */
int doconnect6(IA6 *rad, USHORT rp, IA6 *lad, USHORT lp)
{
	register int nnetfd;
	register int rr;
	int x, y;
	errno = 0;

/* grab a socket; set opts */
      newskt:
	if (o_udpmode)
		nnetfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	else
		nnetfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if (nnetfd < 0)
		bail("Can't get socket");
	if (nnetfd == 0)	/* if stdin was closed this might *be* 0, */
		goto newskt;	/* so grab another.  See text for why... */
	x = 1;
	rr = setsockopt(nnetfd, SOL_SOCKET, SO_REUSEADDR, &x, sizeof(x));
	if (rr == -1)
		holler("nnetfd reuseaddr failed");	/* ??? */
#ifdef SO_REUSEPORT		/* doesnt exist everywhere... */
	rr = setsockopt(nnetfd, SOL_SOCKET, SO_REUSEPORT, &x, sizeof(x));
	if (rr == -1)
		holler("nnetfd reuseport failed");	/* ??? */
#endif
#if 0
/* If you want to screw with RCVBUF/SNDBUF, do it here.  Liudvikas Bukys at
   Rochester sent this example, which would involve YET MORE options and is
   just archived here in case you want to mess with it.  o_xxxbuf are global
   integers set in main() getopt loop, and check for rr == 0 afterward. */
	rr =
	    setsockopt(nnetfd, SOL_SOCKET, SO_RCVBUF, &o_rcvbuf,
		       sizeof o_rcvbuf);
	rr =
	    setsockopt(nnetfd, SOL_SOCKET, SO_SNDBUF, &o_sndbuf,
		       sizeof o_sndbuf);
#endif

/* fill in all the right sockaddr crud */
	lclend6->sin6_family = AF_INET6;
	remend6->sin6_family = AF_INET6;

/* if lad/lp, do appropriate binding */
	if (lad)
		memcpy(&lclend6->sin6_addr, lad, sizeof(IA6));
	else
		memcpy(&lclend6->sin6_addr, &in6addr_any, sizeof(IA6));
	if (lp)
		lclend6->sin6_port = htons(lp);
	rr = 0;
	if (lad || lp) {
		x = (int) lp;
/* try a few times for the local bind, a la ftp-data-port... */
		for (y = 4; y > 0; y--) {
			rr = bind(nnetfd, (SA *) lclend6, sizeof(SAI6));
			if (rr == 0)
				break;
			if (errno != EADDRINUSE)
				break;
			else {
				holler("retrying local %s:%d",
				       inet_ntoa6(&lclend6->sin6_addr), lp);
				sleep(2);
				errno = 0;	/* clear from sleep */
			}	/* if EADDRINUSE */
		}		/* for y counter */
	}			/* if lad or lp */
	if (rr)
		bail("Can't grab %s:%d with bind",
		     inet_ntoa6(&lclend6->sin6_addr), lp);

	if (o_listen)
		return (nnetfd);	/* thanks, that's all for today */

	memcpy(&remend6->sin6_addr, rad, sizeof(IA6));
	remend6->sin6_port = htons(rp);

	if (gatesidx) {		/* if we wanted any srcrt hops ... */
		holler
		    ("Warning: source routing unavailable on this machine, ignoring");
	}

	/* if gatesidx */
	/* wrap connect inside a timer, and hit it */
	arm_timer(1, o_wait);
	if (setjmp(jbuf) == 0) {
		rr = connect(nnetfd, (SA *) remend6, sizeof(SAI6));
	} else {		/* setjmp: connect failed... */
		rr = -1;
		errno = ETIMEDOUT;	/* fake it */
	}
	arm_timer(0, 0);
	if (rr == 0)
		return (nnetfd);
	close(nnetfd);		/* clean up junked socket FD!! */
	return (-1);
}				/* doconnect6 */

/* dolisten6 :
   just like doconnect6, and in fact calls a hunk of doconnect6, but listens for
   incoming and returns an open connection *from* someplace.  If we were
   given host/port args, any connections from elsewhere are rejected.  This
   in conjunction with local-address binding should limit things nicely... */
int dolisten6(IA6 *rad, USHORT rp, IA6 *lad, USHORT lp)
{
	register int nnetfd;
	register int rr;
	HINF6 *whozis = NULL;
	int x;
	char *cp;
	USHORT z;
	errno = 0;

/* Pass everything off to doconnect6, who in o_listen mode just gets a socket */
	nnetfd = doconnect6(rad, rp, lad, lp);
	if (nnetfd <= 0)
		return (-1);
	if (o_udpmode) {	/* apparently UDP can listen ON */
		if (!lp)	/* "port 0",  but that's not useful */
			bail("UDP listen needs -p arg");
	} else {
		rr = listen(nnetfd, 1);	/* gotta listen() before we can get */
		if (rr < 0)	/* our local random port.  sheesh. */
			bail("local listen fuxored");
	}

/* Various things that follow temporarily trash bigbuf_net, which might contain
   a copy of any recvfrom()ed packet, but we'll read() another copy later. */

/* I can't believe I have to do all this to get my own goddamn bound address
   and port number.  It should just get filled in during bind() or something.
   All this is only useful if we didn't say -p for listening, since if we
   said -p we *know* what port we're listening on.  At any rate we won't bother
   with it all unless we wanted to see it, although listening quietly on a
   random unknown port is probably not very useful without "netstat". */
	if (o_verbose) {
		x = sizeof(SAI6);	/* how 'bout getsockNUM instead, pinheads?! */
		rr = getsockname(nnetfd, (SA *) lclend6, &x);
		if (rr < 0)
			holler("local getsockname failed");
		strcpy(bigbuf_net, "listening on [");	/* buffer reuse... */
		if (!IN6_IS_ADDR_UNSPECIFIED(&lclend6->sin6_addr))
			strcat(bigbuf_net, inet_ntoa6(&lclend6->sin6_addr));
		else
			strcat(bigbuf_net, "any");
		strcat(bigbuf_net, "] %d ...");
		z = ntohs(lclend6->sin6_port);
		holler(bigbuf_net, z);
	}

	/* verbose -- whew!! */
	/* UDP is a speeeeecial case -- we have to do I/O *and* get the calling
	   party's particulars all at once, listen() and accept() don't apply.
	   At least in the BSD universe, however, recvfrom/PEEK is enough to tell
	   us something came in, and we can set things up so straight read/write
	   actually does work after all.  Yow.  YMMV on strange platforms!  */
	if (o_udpmode) {
		x = sizeof(SAI6);	/* retval for recvfrom */
		arm_timer(2, o_wait);	/* might as well timeout this, too */
		if (setjmp(jbuf) == 0) {	/* do timeout for initial connect */
			rr = recvfrom	/* and here we block... */
			    
			    (nnetfd, bigbuf_net, BIGSIZ, MSG_PEEK,
			     (SA *) remend6, &x);
			Debug(
			      ("dolisten/recvfrom ding, rr = %d, netbuf %s ",
			       rr, bigbuf_net))
		} else
			goto dol_tmo;	/* timeout */
		arm_timer(0, 0);
/* I'm not completely clear on how this works -- BSD seems to make UDP
   just magically work in a connect()ed context, but we'll undoubtedly run
   into systems this deal doesn't work on.  For now, we apparently have to
   issue a connect() on our just-tickled socket so we can write() back.
   Again, why the fuck doesn't it just get filled in and taken care of?!
   This hack is anything but optimal.  Basically, if you want your listener
   to also be able to send data back, you need this connect() line, which
   also has the side effect that now anything from a different source or even a
   different port on the other end won't show up and will cause ICMP errors.
   I guess that's what they meant by "connect".
   Let's try to remember what the "U" is *really* for, eh? */
		rr = connect(nnetfd, (SA *) remend6, sizeof(SAI6));
		goto whoisit;
	}

	/* o_udpmode */
	/* fall here for TCP */
	x = sizeof(SAI6);	/* retval for accept */
	arm_timer(2, o_wait);		/* wrap this in a timer, too; 0 = forever */
	if (setjmp(jbuf) == 0) {
		rr = accept(nnetfd, (SA *) remend6, &x);
	} else
		goto dol_tmo;	/* timeout */
	arm_timer(0, 0);
	close(nnetfd);		/* dump the old socket */
	nnetfd = rr;		/* here's our new one */

      whoisit:
	if (rr < 0)
		goto dol_err;	/* bail out if any errors so far */

/* find out what address the connection was *to* on our end, in case we're
   doing a listen-on-any on a multihomed machine.  This allows one to
   offer different services via different alias addresses, such as the
   "virtual web site" hack. */
	memset(bigbuf_net, 0, 64);
	cp = &bigbuf_net[32];
	x = sizeof(SAI6);
	rr = getsockname(nnetfd, (SA *) lclend6, &x);
	if (rr < 0)
		holler("post-rcv getsockname failed");
	strcpy(cp, inet_ntoa6(&lclend6->sin6_addr));

/* now check out who it is.  We don't care about mismatched DNS names here,
   but any ADDR and PORT we specified had better fucking well match the caller.
   Converting from addr to inet_ntoa and back again is a bit of a kludge, but
   gethostpoop wants a string and there's much gnarlier code out there already,
   so I don't feel bad.
   The *real* question is why BFD sockets wasn't designed to allow listens for
   connections *from* specific hosts/ports, instead of requiring the caller to
   accept the connection and then reject undesireable ones by closing.  In
   other words, we need a TCP MSG_PEEK. */
	z = ntohs(remend6->sin6_port);
	strcpy(bigbuf_net, inet_ntoa6(&remend6->sin6_addr));
	whozis = gethost6poop(bigbuf_net, o_nflag);
	errno = 0;
	x = 0;			/* use as a flag... */
	if (rad)		/* xxx: fix to go down the *list* if we have one? */
		if (memcmp(rad, whozis->iaddrs, sizeof(SAI6)))
			x = 1;
	if (rp)
		if (z != rp)
			x = 1;
	if (x)			/* guilty! */
		bail("invalid connection to [%s] from %s [%s] %d",
		     cp, whozis->name, whozis->addrs[0], z);
	holler("connect to [%s] from %s [%s] %d",	/* oh, you're okay.. */
	       cp, whozis->name, whozis->addrs[0], z);
	return (nnetfd);	/* open! */

      dol_tmo:
	errno = ETIMEDOUT;	/* fake it */
      dol_err:
	close(nnetfd);
	return (-1);
}				/* dolisten */

/* udptest :
   fire a couple of packets at a UDP target port, just to see if it's really
   there.  On BSD kernels, ICMP host/port-unreachable errors get delivered to
   our socket as ECONNREFUSED write errors.  On SV kernels, we lose; we'll have
   to collect and analyze raw ICMP ourselves a la satan's probe_udp_ports
   backend.  Guess where one could swipe the appropriate code from...

   Use the time delay between writes if given, otherwise use the "tcp ping"
   trick for getting the RTT.  [I got that idea from pluvius, and warped it.]
   Return either the original fd, or clean up and return -1. */
udptest6(int fd, IA6 *where)
{
	register int rr;

	rr = write(fd, bigbuf_in, 1);
	if (rr != 1)
		holler("udptest first write failed?! errno %d", errno);
	if (o_wait)
		sleep(o_wait);
	else {
/* use the tcp-ping trick: try connecting to a normally refused port, which
   causes us to block for the time that SYN gets there and RST gets back.
   Not completely reliable, but it *does* mostly work. */
		o_udpmode = 0;	/* so doconnect6 does TCP this time */
/* Set a temporary connect timeout, so packet filtration doesnt cause
   us to hang forever, and hit it */
		o_wait = 5;	/* enough that we'll notice?? */
		rr = doconnect6(where, SLEAZE_PORT, 0, 0);
		if (rr > 0)
			close(rr);	/* in case it *did* open */
		o_wait = 0;	/* reset it */
		o_udpmode++;	/* we *are* still doing UDP, right? */
	}			/* if o_wait */
	errno = 0;		/* clear from sleep */
	rr = write(fd, bigbuf_in, 1);
	if (rr == 1)		/* if write error, no UDP listener */
		return (fd);
	close(fd);		/* use it or lose it! */
	return (-1);
}				/* udptest */
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
void oprint(int which, char * buf, int n)
{
  int bc;			/* in buffer count */
  int obc;			/* current "global" offset */
  int soc;			/* stage write count */
  register unsigned char * p;	/* main buf ptr; m.b. unsigned here */
  register unsigned char * op;	/* out hexdump ptr */
  register unsigned char * a;	/* out asc-dump ptr */
  register int x;
  register unsigned int y;

  if (! ofd)
    bail ("oprint called with no open fd?!");
  if (n == 0)
    return;

  op = stage;
  if (which) {
    *op = '<';
    obc = wrote_out;		/* use the globals! */
  } else {
    *op = '>';
    obc = wrote_net;
  }
  op++;				/* preload "direction" */
  *op = ' ';
  p = (unsigned char *) buf;
  bc = n;
  stage[59] = '#';		/* preload separator */
  stage[60] = ' ';

  while (bc) {			/* for chunk-o-data ... */
    x = 16;
    soc = 78;			/* len of whole formatted line */
    if (bc < x) {
      soc = soc - 16 + bc;	/* fiddle for however much is left */
      x = (bc * 3) + 11;	/* 2 digits + space per, after D & offset */
      op = &stage[x];
      x = 16 - bc;
      while (x) {
	*op++ = ' ';		/* preload filler spaces */
	*op++ = ' ';
	*op++ = ' ';
	x--;
      }
      x = bc;			/* re-fix current linecount */
    } /* if bc < x */

    bc -= x;			/* fix wrt current line size */
    sprintf (&stage[2], "%8.8x ", obc);		/* xxx: still slow? */
    obc += x;			/* fix current offset */
    op = &stage[11];		/* where hex starts */
    a = &stage[61];		/* where ascii starts */

    while (x) {			/* for line of dump, however long ... */
      y = (int)(*p >> 4);	/* hi half */
      *op = hexnibs[y];
      op++;
      y = (int)(*p & 0x0f);	/* lo half */
      *op = hexnibs[y];
      op++;
      *op = ' ';
      op++;
      if ((*p > 31) && (*p < 127))
	*a = *p;		/* printing */
      else
	*a = '.';		/* nonprinting, loose def */
      a++;
      p++;
      x--;
    } /* while x */
    *a = '\n';			/* finish the line */
    x = write (ofd, stage, soc);
    if (x < 0)
      bail ("ofd write err");
  } /* while bc */
} /* oprint */

#ifdef TELNET
USHORT o_tn = 0;		/* global -t option */

/* atelnet :
   Answer anything that looks like telnet negotiation with don't/won't.
   This doesn't modify any data buffers, update the global output count,
   or show up in a hexdump -- it just shits into the outgoing stream.
   Idea and codebase from Mudge@l0pht.com. */
void atelnet(unsigned char * buf, unsigned int size)/* has to be unsigned here! */
{
  static unsigned char obuf [4];  /* tiny thing to build responses into */
  register int x;
  register unsigned char y;
  register unsigned char * p;

  y = 0;
  p = buf;
  x = size;
  while (x > 0) {
    if (*p != 255)			/* IAC? */
      goto notiac;
    obuf[0] = 255;
    p++; x--;
    if ((*p == 251) || (*p == 252))	/* WILL or WONT */
      y = 254;				/* -> DONT */
    if ((*p == 253) || (*p == 254))	/* DO or DONT */
      y = 252;				/* -> WONT */
    if (y) {
      obuf[1] = y;
      p++; x--;
      obuf[2] = *p;			/* copy actual option byte */
      (void) write (netfd, obuf, 3);
/* if one wanted to bump wrote_net or do a hexdump line, here's the place */
      y = 0;
    } /* if y */
notiac:
    p++; x--;
  } /* while x */
} /* atelnet */
#endif /* TELNET */

/* readwrite :
   handle stdin/stdout/network I/O.  Bwahaha!! -- the select loop from hell.
   In this instance, return what might become our exit status. */
int readwrite(int fd)
{
	register int rr;		//表示读写的偏移量
	register char * zp;		/* 本地的缓冲区指针 */
	register char * np;		/* 网络连接传输用的缓冲区指针 */
	unsigned int rzleft;
	unsigned int rnleft;
	USHORT netretry;		/* net-read retry counter */
	USHORT wretry;		/* net-write sanity counter */

	 //如果传过来的文件描述符大于预设的文件描述符集最大限制，报错
	if (fd > FD_SETSIZE) {
		holler("Preposterous fd value %d", fd);
		return (1);
	}
	FD_SET(fd, ding1);		//在文件描述符集ding1中增加网络端口文件描述符fd
	netretry = 2;			//给两次机会，如果一段时间内文件描述符没有发生读写或者错误，用来重新循环一次
	rzleft = rnleft = 0;

  /* and now the big ol' select shoveling loop ... */
	while (FD_ISSET(fd, ding1)) {	/* 直到连接断开，否则一直循环 */
		wretry = 8200;			/* more than we'll ever hafta write */
		*ding2 = *ding1;			/* FD_COPY ain't portable... */
	/* some systems, notably linux, crap into their select timers on return, so
	   we create a expendable copy and give *that* to select.  *Fuck* me ... */
		if (timer1)//由于一些linux系统在返回选择时间时很垃圾，所以创建一个时间副本
			memcpy(timer2, timer1, sizeof(struct timeval));//将timer1拷贝到timer2
		/*系统提供select函数来实现多路复用输入 / 输出模型
			int select(int maxfd,fd_set *rdset,fd_set *wrset,fd_set *exset,struct timeval *timeout);
			maxfd：需要监视的最大的文件描述符值+1
			rdset：需要检测的可读文件描述符的集合
			wrset：需要检测的可写文件描述符的集合
			exset：需要检测的异常文件描述符的集合
			struct timeval：用于描述一段时间长度，
			如果在这个时间内，需要监视的描述符没有事件发生则函数返回，返回值为0
			返回值：返回状态发生变化的描述符总数*/
		rr = select(16, ding2, 0, 0, timer2);//在timer2的时间内查看ding2文件描述符集合中是否有文件读写的变化
		if (rr < 0) {//如果rr < 0，说明发生异常，关闭文件描述符，直接返回
			if (errno != EINTR) {		
				holler("select fuxored");
				close(fd);
				return (1);
			}
		} /* select fuckup */
	/* if we have a timeout AND stdin is closed AND we haven't heard anything
	   from the net during that time, assume it's dead and close it too. */
		if (rr == 0) {//需要监视的描述符没有事件发生则函数返回
			if (!FD_ISSET(0, ding1))//查询标准输入是否在文件描述符集中，如果不在，则netretry--/************fileFD*************/
				netretry--;			//再给网络一次机会
			if (!netretry) {//如果netretry == 0则关闭文件描述符，返回
				if (o_verbose > 1)		/* normally we don't care */
					holler("net timeout");
				close(fd);
				return (0);			/* not an error! */
			}
		} /* select timeout */

	/* xxx: should we check the exception fds too?  The read fds seem to give
	   us the right info, and none of the examples I found bothered. */
	   /* Ding!!  Something arrived, go check all the incoming hoppers, net first */
		if (FD_ISSET(fd, ding2)) {		/* net: ding! */
			rr = read(fd, bigbuf_net, BIGSIZ);//从fd文件描述符的端口中向bigbuf_net里面读数据，bigbuf_net代表从网络上接收到的数据
			if (rr <= 0) {//read失败，清除文件描述符的标志位，循环结束，关闭文件
				FD_CLR(fd, ding1);		/* net closed, we'll finish up... */
				rzleft = 0;			/* can't write anymore: broken pipe */
			}
			else {
				rnleft = rr;//读成功，用rnleft记录从网络端口读取到的字节数
				np = bigbuf_net;//令np指向网络buffer的空间
#ifdef TELNET
				if (o_tn)
					atelnet(np, rr);		/* fake out telnet stuff */
#endif /* TELNET */
			} /* if rr */
			Debug(("got %d from the net, errno %d", rr, errno))
		} /* net:ding */

	/* if we're in "slowly" mode there's probably still stuff in the stdin
	   buffer, so don't read unless we really need MORE INPUT!  MORE INPUT! */
		if (rzleft)
			goto shovel;

		/* okay, suck more stdin */
		/*在这里将标准输入重定向为zip的文件描述符*/
		if (FD_ISSET(0, ding2)) {		/* stdin: ding! fileFD*/
			rr = read(0, bigbuf_in, BIGSIZ);//从标准输入中向bigbuf_in中读入数据，bigbuf_in代表的是从标准输入读入的数据 fileFD
			/* Considered making reads here smaller for UDP mode, but 8192-byte
			   mobygrams are kinda fun and exercise the reassembler. */
			if (rr <= 0) {//如果读的数据出错，或者读到文件结尾，则断开连接，关闭标准输入文件描述符
				FD_CLR(0, ding1);//将标准输入文件描述符从ding1中清除 fileFD
				close(0);//关闭标准输入文件描述符 fileFD
				/* if the user asked to exit on EOF, do it */
				if (o_quit == 0) {//根据设置的时间，做延时退出处理
					shutdown(netfd, 1);//关闭netfd套接字的所有写连接
					close(fd);//关闭文件描述符fd
					freeBufs();//free bufs
					exit(0);//退出程序
				}
				/* if user asked to die after a while, arrange for it */
				if (o_quit > 0) {
					shutdown(netfd, 1);//关闭netfd套接字的所有写连接
					signal(SIGALRM, quit);//用alarm函数设置的timer超时或setitimer函数设置的interval timer超时退出
					alarm(o_quit);//延时等待o_quit的时间
				}
			}
			else {
				rzleft = rr;//保存从标准输入读取到的数据个数
				zp = bigbuf_in;//使zp指向本地数据缓冲区
				/* special case for multi-mode -- we'll want to send this one buffer to every
				   open TCP port or every UDP attempt, so save its size and clean up stdin */
				if (!Single) {		/* we might be scanning... */
					insaved = rr;		/* save len */
					FD_CLR(0, ding1);		/* disable further junk from stdin /fileFD*/
					close(0);			/* really, I mean it /fileFD*/
				} /* Single */
			} /* if rr/read */
		} /* stdin:ding */

	shovel://写工作
		if ((rzleft > 8200) || (rnleft > 8200)) {
			holler("Bogus buffers: %d, %d", rzleft, rnleft);
			rzleft = rnleft = 0;
		}
		/* net write retries sometimes happen on UDP connections */
		if (!wretry) {			/* is something hung? */
			holler("too many output retries");
			return (1);
		}
		/*网络 -> 本地的写工作*/
		if (rnleft) {
			rr = write(1, np, rnleft);//从网络缓冲区np所指向的空间也就是bigbuf_net中向标准输出写入数据
			if (rr > 0) {
				if (o_wfile)//已经将-o选项移除，因此不存在此项
					oprint(1, np, rr);		
				np += rr;			//向后移动写指针，移动rr个单位
				rnleft -= rr;		//表示又将缓冲区中的rr个单位写过了
				wrote_out += rr;	//表示已经向标准输出又写了rr个单位
			}
			Debug(("wrote %d to stdout, errno %d", rr, errno))
		} /* rnleft */
		
		/* 本地 -> 网络的写工作*/
		if (rzleft) {
			rr = rzleft;
			rr = write(fd, zp, rr);	//从zp所指向的buffer也就是bigbuf_in中向网络块中写入rr个数据，代表发送数据
			if (rr > 0) {
				if (o_wfile)//已经将-o选项移除，因此不存在此项
					oprint(0, zp, rr);
				zp += rr;			//向后移动写指针，移动rr个单位
				rzleft -= rr;		//表示又将缓冲区中的rr个单位写过了
				wrote_net += rr;	//表示已经向网络端口又写了rr个单位
			}
			Debug(("wrote %d to net, errno %d", rr, errno))
		} /* rzleft */

		if ((rzleft) || (rnleft)) {		/* shovel that shit till they ain't */
			wretry--;			/* none left, and get another load */
			goto shovel;
		}
	} /* while ding1:netfd is open */

	close(fd);
	return 0;
} /* readwrite */

/* unescape :
   translate \-'s into -'s, returns start */
char * unescape(char *start)
{
  char * end = NULL;
  char * next = NULL;
  char * p = NULL;

  end = start + strlen(start);
  next = start;

  while (next = strstr (next+1, "\\-")) {
    p = next;
    while (p < end) /* copy string back one char, overwriting backslash */
      *(p++) = *(p+1);
    end--;
  }

  return start;
} /* unescape */

#ifdef HAVE_HELP		/* unless we wanna be *really* cryptic */
/* helpme :
   the obvious */
void helpme(void)
{
	o_verbose = 1;
	o_holler_stderr = 0;
	holler("netpack: [v1.00]\n\
Usage:	np 192.xxx.xxx.xxx:0000 [file[s] || dir[s]] \
options:");//listen for inbound:	np -l -p port [-options] [hostname] [port] [-f zipFileName] [-k password] [-a] [-o] [-0] -[1] -[9] dir[s] file[s]\n
	holler("\
	-1\tCompress faster\n\
	-9\tCompress better\n\
	-a\tAppend to existing file.cvtelog.zip \n\
	-o\tOverwrite existing file.cvtelog.zip \n\
	-f\tAssign the zip fileName \n\
	-k\tpassword the zip file\n");
#ifdef LISTEN_MODE //如果需要使用监听模式时定义LISTEN_MODE这个宏
	holler("-l\tlisten mode, for inbound connects\n\
	-p\tport	local port number\n");
#endif
	holler("\
	-h\tthis cruft\n\
	-n\tnumeric-only IP addresses, no DNS\n\
	-s\taddr	local source address\n\
	-0\tStore only\n");
#ifdef TELNET
	holler("\
	-t\tanswer TELNET negotiation");
#endif
	holler("\
	-u\tUDP mode\n\
	-v\tverbose [use twice to be more verbose]\n\
	-w\tsecs	timeout for connects and final net reads\n\
	-z\tzero-I/O mode [used for scanning]");
	bail("\n\nAthor by jonewan at 2019/01/11\nAny question send email to i_wangjiahao@cvte.com\n");
} /* helpme */
#endif /* HAVE_HELP */
