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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dnsproxy.h"

/* parse -- Simple configuration file parser. Takes a filename and
 * reads pairs of 'key value' or 'key = value'.
 */

int
parse(const char *fname)
{
	FILE *f;
	char buf[1024];
	char *s, *key, *arg;

	if ((f = fopen(fname, "r")) == NULL)
		return 0;

	while (fgets(buf, sizeof(buf), f) != NULL) {

		if ((s = strchr(buf, '#')) != NULL)
			*s = '\0';

		key = strtok(buf, " \t=");
		arg = strtok(NULL, " \t\n");
		if (!key || !arg)
			continue;

		DPRINTF(("Found key '%s' arg '%s'\n", key, arg));

		if (!strcmp(key, "authoritative")) {
			authoritative = strdup(arg);
			continue;
		}
		if (!strcmp(key, "authoritative-timeout")) {
			authoritative_timeout = strtol(arg, NULL, 10);
			continue;
		}
		if (!strcmp(key, "authoritative-port")) {
			authoritative_port = strtol(arg, NULL, 10);
			continue;
		}
		if (!strcmp(key, "recursive")) {
			recursive = strdup(arg);
			continue;
		}
		if (!strcmp(key, "recursive-timeout")) {
			recursive_timeout = strtol(arg, NULL, 10);
			continue;
		}
		if (!strcmp(key, "recursive-port")) {
			recursive_port = strtol(arg, NULL, 10);
			continue;
		}
		if (!strcmp(key, "statistics")) {
			stats_timeout = strtol(arg, NULL, 10);
			continue;
		}
		if (!strcmp(key, "listen")) {
			listenat = strdup(arg);
			continue;
		}
		if (!strcmp(key, "port")) {
			port = strtol(arg, NULL, 10);
			continue;
		}
		if (!strcmp(key, "chroot")) {
			chrootdir = strdup(arg);
			continue;
		}
		if (!strcmp(key, "user")) {
			user = strdup(arg);
			continue;
		}
		if (!strcmp(key, "internal")) {
			add_internal(arg);
			continue;
		}

		info("Unable to parse '%s'", buf);
	}

	fclose(f);

	return 1;
}
