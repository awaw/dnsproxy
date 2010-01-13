/* $Id: internal.c,v 1.9 2004/04/05 12:47:07 armin Exp $ */
/*
 * Copyright (c) 2003,2004 Armin Wolfermann
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <config.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dnsproxy.h"

struct internal {
	unsigned int	addr;
	unsigned int	mask;
	struct internal	*next;
};

static struct internal *internals = NULL;

int
add_internal(char *s)
{
	char *p;
	int mask;
	struct in_addr addr;
	struct internal *ipa;

	if (s == NULL)
		return 0;

	if ((p = strchr(s, '/')) != NULL) {
		mask = strtol(p+1, NULL, 10) % 32;
		*p = '\0';
	} else {
		mask = 32;
	}

	if (inet_pton(AF_INET, s, &addr) != 1)
		return 0;

	if ((ipa = calloc(1, sizeof(struct internal))) == NULL)
		fatal("calloc: %s", strerror(errno));

	memcpy(&ipa->addr, &addr, 4);
	ipa->mask = htonl(0xffffffff << (32 - mask));
	ipa->addr &= ipa->mask;
	ipa->next = internals;
	internals = ipa;

	DPRINTF(("add_internal %s/%d (%08x/%08x)\n", inet_ntoa(addr), mask,
	    ipa->addr, ipa->mask));

	return 1;
}

int
is_internal(struct in_addr arg)
{
	unsigned int addr;
	struct internal *p;

	DPRINTF(("is_internal(%s)\n", inet_ntoa(arg)));
	memcpy(&addr, &arg, 4);

	for (p = internals; p != NULL; p = p->next) {
		DPRINTF(("%08x == %08x & %08x ?\n", p->addr, addr, p->mask));
		if (p->addr == (addr & p->mask))
			return 1;
	}

	return 0;
}
