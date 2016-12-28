/*
 * Copyright (c) 2003,2004,2005 Armin Wolfermann
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
#include <syslog.h>
#include "dnsproxy.h"

static int log_on_syslog = 0;

void
log_syslog(const char *tag)
{
	openlog(tag, LOG_NDELAY, LOG_DAEMON);
	++log_on_syslog;
}

static void
log_printf(int level, const char *fmt, va_list ap)
{
	if (log_on_syslog)
		vsyslog(level, fmt, ap);
	else {
		(void)vfprintf(stderr, fmt, ap);
		if (strchr(fmt, '\n') == NULL)
			fprintf(stderr, "\n");
	}
}

void
info(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	log_printf(LOG_INFO, fmt, ap);
	va_end(ap);
}

void
error(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	log_printf(LOG_ERR, fmt, ap);
	va_end(ap);
}

void
fatal(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	log_printf(LOG_ERR, fmt, ap);
	va_end(ap);

	exit(1);
}
