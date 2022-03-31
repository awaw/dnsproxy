/*
 * Copyright (c) 2003,2004,2005,2010 Armin Wolfermann
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

#ifndef _DNSPROXY_H_
#define _DNSPROXY_H_

#if HAVE_SYS_TIME_H
# include <sys/time.h>
# include <time.h>
#else
# include <time.h>
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#if HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#include <stdarg.h>

#include <event.h>

#ifdef DEBUG
#define DPRINTF(x) do { printf x ; } while (0)
#else
#define DPRINTF(x)
#endif

#ifdef GLOBALS
#define GLOBAL(a) a
#define GLOBAL_INIT(a,b) a = b
#else
#define GLOBAL(a) extern a
#define GLOBAL_INIT(a,b) extern a
#endif

struct request {
	unsigned short		id;

	struct sockaddr_in	client;
	unsigned short		clientid;
	unsigned char		recursion;

	struct event		timeout;

	struct request		**prev;
	struct request		*next;
};

GLOBAL_INIT(unsigned int authoritative_port, 53);
GLOBAL_INIT(unsigned int authoritative_timeout, 10);
GLOBAL_INIT(unsigned int recursive_port, 53);
GLOBAL_INIT(unsigned int recursive_timeout, 90);
GLOBAL_INIT(unsigned int stats_timeout, 3600);
GLOBAL_INIT(unsigned int port, 53);

GLOBAL(char *authoritative);
GLOBAL(char *chrootdir);
GLOBAL(char *listenat);
GLOBAL(char *recursive);
GLOBAL(char *user);

GLOBAL(unsigned long active_queries);
GLOBAL(unsigned long all_queries);
GLOBAL(unsigned long authoritative_queries);
GLOBAL(unsigned long recursive_queries);
GLOBAL(unsigned long removed_queries);
GLOBAL(unsigned long dropped_queries);
GLOBAL(unsigned long answered_queries);
GLOBAL(unsigned long dropped_answers);
GLOBAL(unsigned long late_answers);
GLOBAL(unsigned long hash_collisions);

/* dnsproxy.c */
void signal_handler(int, short, void *);

/* daemon.c */
int daemon(int, int);

/* hash.c */
void hash_add_request(struct request *);
void hash_remove_request(struct request *);
struct request *hash_find_request(unsigned short);

/* internal.c */
int add_internal(char *);
int is_internal(struct in_addr);

/* log.c */
void log_syslog(const char *);
void info(const char *, ...);
void error(const char *, ...);
void fatal(const char *, ...);

/* parse.c */
int parse(const char *);

/* statistics.c */
void statistics_start(void);

#endif /* _DNSPROXY_H_ */
