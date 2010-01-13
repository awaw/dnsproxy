/* $Id: stats.c,v 1.10 2004/04/05 12:47:07 armin Exp $ */
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
#include <string.h>
#include "dnsproxy.h"

static struct event timeout;
static struct timeval tv;

/* ARGSUSED */
static void
statistics_timeout(int fd, short event, void *arg)
{
	/* reschedule timer event */
	if (event_add(&timeout, &tv) == -1)
		fatal("event_add: %s", strerror(errno));

	/* print statistics */
	info("ActiveQr AuthorQr RecursQr AllQuery Answered");
	info("%8ld %8ld %8ld %8ld %8ld", active_queries, authoritative_queries,
	    recursive_queries, all_queries, answered_queries);
	info("TimeoutQ DroppedQ DroppedA LateAnsw HashColl");
	info("%8ld %8ld %8ld %8ld %8ld", removed_queries, dropped_queries,
	    dropped_answers, late_answers, hash_collisions);
}

void
statistics_start(void)
{
	if (stats_timeout > 0) {
		tv.tv_sec=stats_timeout;
		tv.tv_usec=0;

		evtimer_set(&timeout, statistics_timeout, NULL);
		if (evtimer_add(&timeout, &tv) == -1)
			fatal("evtimer_add: %s", strerror(errno));
	}
}
