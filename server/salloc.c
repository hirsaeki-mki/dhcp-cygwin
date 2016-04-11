/* salloc.c

   Memory allocation for the DHCP server... */

/*
 * Copyright (c) 1996-2002 Internet Software Consortium.
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

#ifndef lint
static char copyright[] =
"$Id: salloc.c,v 1.2.2.4 2002/11/17 02:29:32 dhankins Exp $ Copyright (c) 1996-2002 The Internet Software Consortium.  All rights reserved.\n";
#endif /* not lint */

#include "dhcpd.h"
#include <omapip/omapip_p.h>

#if defined (COMPACT_LEASES)
struct lease *free_leases;

# if defined (DEBUG_MEMORY_LEAKAGE_ON_EXIT)
struct lease *lease_hunks;

void relinquish_lease_hunks ()
{
	struct lease *c, *n, **p, *f;
	int i;

	/* Account for all the leases on the free list. */
	for (n = lease_hunks; n; n = n -> next) {
	    for (i = 1; i < n -> starts + 1; i++) {
		p = &free_leases;
		for (c = free_leases; c; c = c -> next) {
		    if (c == &n [i]) {
			*p = c -> next;
			n -> ends++;
			break;
		    }
		    p = &c -> next;
		}
		if (!c) {
		    log_info ("lease %s refcnt %d",
			      piaddr (n [i].ip_addr), n [i].refcnt);
		    dump_rc_history (&n [i]);
		}
	    }
	}
		
	for (c = lease_hunks; c; c = n) {
		n = c -> next;
		if (c -> ends != c -> starts) {
			log_info ("lease hunk %lx leases %ld free %ld",
				  (unsigned long)c, (unsigned long)c -> starts,
				  (unsigned long)c -> ends);
		}
		dfree (c, MDL);
	}

	/* Free all the rogue leases. */
	for (c = free_leases; c; c = n) {
		n = c -> next;
		dfree (c, MDL);
	}
}
#endif

struct lease *new_leases (n, file, line)
	unsigned n;
	const char *file;
	int line;
{
	struct lease *rval;
#if defined (DEBUG_MEMORY_LEAKAGE_ON_EXIT)
	rval = dmalloc ((n + 1) * sizeof (struct lease), file, line);
	memset (rval, 0, sizeof (struct lease));
	rval -> starts = n;
	rval -> next = lease_hunks;
	lease_hunks = rval;
	rval++;
#else
	rval = dmalloc (n * sizeof (struct lease), file, line);
#endif
	return rval;
}

/* If we are allocating leases in aggregations, there's really no way
   to free one, although perhaps we can maintain a free list. */

isc_result_t dhcp_lease_free (omapi_object_t *lo,
			      const char *file, int line)
{
	struct lease *lease;
	if (lo -> type != dhcp_type_lease)
		return ISC_R_INVALIDARG;
	lease = (struct lease *)lo;
	memset (lease, 0, sizeof (struct lease));
	lease -> next = free_leases;
	free_leases = lease;
	return ISC_R_SUCCESS;
}

isc_result_t dhcp_lease_get (omapi_object_t **lp,
			     const char *file, int line)
{
	struct lease **lease = (struct lease **)lp;
	struct lease *lt;

	if (free_leases) {
		lt = free_leases;
		free_leases = lt -> next;
		*lease = lt;
		return ISC_R_SUCCESS;
	}
	return ISC_R_NOMEMORY;
}
#endif /* COMPACT_LEASES */

OMAPI_OBJECT_ALLOC (lease, struct lease, dhcp_type_lease)
OMAPI_OBJECT_ALLOC (class, struct class, dhcp_type_class)
OMAPI_OBJECT_ALLOC (pool, struct pool, dhcp_type_pool)

#if !defined (NO_HOST_FREES)	/* Scary debugging mode - don't enable! */
OMAPI_OBJECT_ALLOC (host, struct host_decl, dhcp_type_host)
#else
isc_result_t host_allocate (struct host_decl **p, const char *file, int line)
{
	return omapi_object_allocate ((omapi_object_t **)p,
				      dhcp_type_host, 0, file, line);
}

isc_result_t host_reference (struct host_decl **pptr, struct host_decl *ptr,
			       const char *file, int line)
{
	return omapi_object_reference ((omapi_object_t **)pptr,
				       (omapi_object_t *)ptr, file, line);
}

isc_result_t host_dereference (struct host_decl **ptr,
			       const char *file, int line)
{
	if ((*ptr) -> refcnt == 1) {
		log_error ("host dereferenced with refcnt == 1.");
#if defined (DEBUG_RC_HISTORY)
		dump_rc_history ();
#endif
		abort ();
	}
	return omapi_object_dereference ((omapi_object_t **)ptr, file, line);
}
#endif

struct lease_state *free_lease_states;

struct lease_state *new_lease_state (file, line)
	const char *file;
	int line;
{
	struct lease_state *rval;

	if (free_lease_states) {
		rval = free_lease_states;
		free_lease_states =
			(struct lease_state *)(free_lease_states -> next);
 		dmalloc_reuse (rval, file, line, 0);
	} else {
		rval = dmalloc (sizeof (struct lease_state), file, line);
		if (!rval)
			return rval;
	}
	memset (rval, 0, sizeof *rval);
	if (!option_state_allocate (&rval -> options, file, line)) {
		free_lease_state (rval, file, line);
		return (struct lease_state *)0;
	}
	return rval;
}

void free_lease_state (ptr, file, line)
	struct lease_state *ptr;
	const char *file;
	int line;
{
	if (ptr -> options)
		option_state_dereference (&ptr -> options, file, line);
	if (ptr -> packet)
		packet_dereference (&ptr -> packet, file, line);
	if (ptr -> shared_network)
		shared_network_dereference (&ptr -> shared_network,
					    file, line);

	data_string_forget (&ptr -> parameter_request_list, file, line);
	data_string_forget (&ptr -> filename, file, line);
	data_string_forget (&ptr -> server_name, file, line);
	ptr -> next = free_lease_states;
	free_lease_states = ptr;
	dmalloc_reuse (free_lease_states, (char *)0, 0, 0);
}

#if defined (DEBUG_MEMORY_LEAKAGE) || \
		defined (DEBUG_MEMORY_LEAKAGE_ON_EXIT)
void relinquish_free_lease_states ()
{
	struct lease_state *cs, *ns;

	for (cs = free_lease_states; cs; cs = ns) {
		ns = cs -> next;
		dfree (cs, MDL);
	}
	free_lease_states = (struct lease_state *)0;
}
#endif

struct permit *new_permit (file, line)
	const char *file;
	int line;
{
	struct permit *permit = ((struct permit *)
				 dmalloc (sizeof (struct permit), file, line));
	if (!permit)
		return permit;
	memset (permit, 0, sizeof *permit);
	return permit;
}

void free_permit (permit, file, line)
	struct permit *permit;
	const char *file;
	int line;
{
	if (permit -> type == permit_class)
		class_dereference (&permit -> class, MDL);
	dfree (permit, file, line);
}