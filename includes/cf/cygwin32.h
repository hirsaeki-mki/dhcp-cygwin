/* cygwin32.h

   System dependencies for Win32, compiled with Cygwin32...   This
   doesn't work yet, so don't get too excited! */

/*
 * Copyright (c) 1996-1999 Internet Software Consortium.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of The Internet Software Consortium nor the names
 *    of its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INTERNET SOFTWARE CONSORTIUM AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE INTERNET SOFTWARE CONSORTIUM OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This software has been written for the Internet Software Consortium
 * by Ted Lemon in cooperation with Vixie Enterprises and Nominum, Inc.
 * To learn more about the Internet Software Consortium, see
 * ``http://www.isc.org/''.  To learn more about Vixie Enterprises,
 * see ``http://www.vix.com''.   To learn more about Nominum, Inc., see
 * ``http://www.nominum.com''.
 */

#include <sys/time.h>

#define IN
#define OUT
#undef fd_set
#undef FD_SET
#undef FD_CLR
#undef FD_ZERO
#undef FD_ISSET
#undef FD_ISCLR
#undef FD_SETSIZE
#define IFNAMSIZ 16

#define __INSIDE_CYGWIN_NET__
#include <cygwin/in.h>
#include <cygwin/if.h>
#include <sys/socket.h>
#include <netdb.h>
#ifndef IFF_POINTOPOINT
#define IFF_POINTOPOINT 0x10
#endif
#ifndef FD_SETSIZE
#define FD_SETSIZE 256
#endif	// ifndef FD_SETSIZE
#ifndef FD_SET

// typedef long fd_mask;

#undef NFDBITS
#define NFDBITS	(sizeof(fd_mask) * NBBY)	/* bits per mask */
#ifndef howmany
#define	howmany(x, y)	(((x)+((y)-1))/(y))
#endif /* howmany */

typedef	struct fd_set {
	fd_mask	fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} fd_set;

#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)	memset((char *)(p), '\0', sizeof(*(p)))

#endif /* FD_SET */
#ifdef NEED_PRAND_CONF
const char *cmds[] = {
	"/bin/ps -axlw 2>&1",
	"/sbin/arp -an 2>&1",
	"/bin/netstat -an 2>&1",
	"/bin/df  2>&1",
	"/usr/bin/dig com. soa +ti=1 +retry=0 2>&1",
	"/usr/bin/uptime  2>&1",
	"/bin/netstat -s 2>&1",
	"/usr/bin/dig . soa +ti=1 +retry=0 2>&1",
	"/usr/bin/vmstat  2>&1",
	"/usr/bin/w  2>&1",
	NULL
};
const char *files[] = {
	"/proc/stat",
	"/proc/rtc",
	"/proc/meminfo",
	"/proc/interrupts",
	"/proc/self/status",
	"/var/log/messages",
	"/var/log/wtmp",
	"/var/log/lastlog",
	NULL
};
const char *dirs[] = {
	"/tmp",
	"/usr/tmp",
	".",
	"/",
	"/var/spool",
	"/dev",
	"/var/spool/mail",
	"/home",
	"/usr/home",
	NULL
};
#endif	/* NEED_PRAND_CONF */
#define GET_HOST_ID_MISSING 1

#include <syslog.h>
#include <string.h>
#include <paths.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <setjmp.h>
#include <limits.h>

#include <sys/wait.h>
#include <signal.h>

#define NO_H_ERRNO

#include <sys/param.h>

/* Varargs stuff... */
#include <stdarg.h>
#define VA_DOTDOTDOT ...
#define va_dcl
#define VA_start(list, last) va_start (list, last)
#define vsnprintf(buf, size, fmt, list) vsprintf (buf, fmt, list)
#define NO_SNPRINTF

#ifndef _PATH_DHCPD_PID
#define _PATH_DHCPD_PID	"/var/run/dhcpd.pid"
#endif
#ifndef _PATH_DHCPD_DB
#define _PATH_DHCPD_DB "/var/spool/dhcp/dhcpd.leases"
#endif
#ifndef _PATH_DHCPD_CONF
#define _PATH_DHCPD_CONF "/etc/dhcpd.conf"
#endif
#ifndef _PATH_DHCLIENT_PID
#define _PATH_DHCLIENT_PID "/var/run/dhclient.pid"
#endif
#ifndef _PATH_DHCLIENT_DB
#define _PATH_DHCLIENT_DB "/var/spool/dhcp/dhclient.leases"
#endif
#ifndef _PATH_DHCLIENT_CONF
#define _PATH_DHCLIENT_CONF "/etc/dhclient.conf"
#endif
#ifndef _PATH_DHCRELAY_PID
#define _PATH_DHCRELAY_PID "/var/run/dhcrelay.pid"
#endif

#ifndef _PATH_RESOLV_CONF
#define _PATH_RESOLV_CONF "/etc/resolv.conf"
#endif

#define int8_t		char
#define int16_t		short 
#define int32_t		long 

#define u_int8_t	unsigned char		/* Not quite POSIX... */
#define u_int16_t	unsigned short 
#define u_int32_t	unsigned long 

#define EOL	'\n'
#define VOIDPTR void *

/* Time stuff... */
#define TIME time_t
#define GET_TIME(x)	time ((x))

#if defined (USE_DEFAULT_NETWORK)
#  define USE_SOCKETS
#endif

#ifdef __alpha__
#define PTRSIZE_64BIT
#endif
