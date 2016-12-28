/*
 * Copyright (c) 2003,2004,2005,2010,2016 Armin Wolfermann
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
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define GLOBALS 1
#include "dnsproxy.h"

#define RD(x) (*(x + 2) & 0x01)

static unsigned short queryid = 0;
#define QUERYID queryid++

static struct sockaddr_in authoritative_addr;
static struct sockaddr_in recursive_addr;
static int sock_query;
static int sock_answer;
static int dnsproxy_sig;

extern int event_gotsig;
extern int (*event_sigcb)(void);

#ifdef DEBUG
char *malloc_options = "AGZ";
#endif

/* signal_handler -- Called by libevent if a signal arrives.
 */

void
signal_handler(int sig, short event, void *arg)
{
	(void)event_loopexit(NULL);
	fatal("exiting on signal %d", sig);
}

/* timeout -- Called by the event loop when a query times out. Removes the
 * query from the queue.
 */

/* ARGSUSED */
static void
timeout(int fd, short event, void *arg)
{
	hash_remove_request((struct request *)arg);
	free((struct request *)arg);
	++removed_queries;
}

/* do_query -- Called by the event loop when a packet arrives at our
 * listening socket. Read the packet, create a new query, append it to the
 * queue and send it to the correct server.
 */

/* ARGSUSED */
static void
do_query(int fd, short event, void *arg)
{
	char buf[MAXEDNS];
	int byte = 0;
	struct sockaddr_in fromaddr;
	unsigned int fromlen = sizeof(fromaddr);
	struct request *req;
	struct timeval tv;

	++all_queries;

	/* Reschedule event */
	event_add((struct event *)arg, NULL);

	/* read packet from socket */
	if ((byte = recvfrom(fd, buf, sizeof(buf), 0,
			    (struct sockaddr *)&fromaddr, &fromlen)) == -1) {
		error("recvfrom failed: %s", strerror(errno));
		++dropped_queries;
		return;
	}

	/* check for minimum dns packet length */
	if (byte < 12) {
		error("query too short from %s",
		    inet_ntoa(fromaddr.sin_addr));
		++dropped_queries;
		return;
	}

	/* allocate new request */
	if ((req = calloc(1, sizeof(struct request))) == NULL) {
		error("calloc: %s", strerror(errno));
		++dropped_queries;
		return;
	}

	/* fill the request structure */
	req->id = QUERYID;
	memcpy(&req->client, &fromaddr, sizeof(struct sockaddr_in));
	memcpy(&req->clientid, &buf[0], 2);

	/* where is this query coming from? */
	if (is_internal(fromaddr.sin_addr)) {
		req->recursion = RD(buf);
		DPRINTF(("Internal query RD=%d\n", req->recursion));
	} else {
		/* no recursion for foreigners */
		req->recursion = 0;
		DPRINTF(("External query RD=%d\n", RD(buf)));
	}

	/* insert it into the hash table */
	hash_add_request(req);

	/* overwrite the original query id */
	memcpy(&buf[0], &req->id, 2);

	if (req->recursion) {

		/* recursive queries timeout in 90s */
		event_set(&req->timeout, -1, 0, timeout, req);
		tv.tv_sec=recursive_timeout; tv.tv_usec=0;
		event_add(&req->timeout, &tv);

		/* send it to our recursive server */
		if ((byte = sendto(sock_answer, buf, (unsigned int)byte, 0,
				    (struct sockaddr *)&recursive_addr,
				    sizeof(struct sockaddr_in))) == -1) {
			error("sendto failed: %s", strerror(errno));
			++dropped_queries;
			return;
		}

		++recursive_queries;

	} else {

		/* authoritative queries timeout in 10s */
		event_set(&req->timeout, -1, 0, timeout, req);
		tv.tv_sec=authoritative_timeout; tv.tv_usec=0;
		event_add(&req->timeout, &tv);

		/* send it to our authoritative server */
		if ((byte = sendto(sock_answer, buf, (unsigned int)byte, 0,
				    (struct sockaddr *)&authoritative_addr,
				    sizeof(struct sockaddr_in))) == -1) {
			error("sendto failed: %s", strerror(errno));
			++dropped_queries;
			return;
		}

		++authoritative_queries;
	}
}

/* do_answer -- Process a packet coming from our authoritative or recursive
 * server. Find the corresponding query and send answer back to querying
 * host.
 */

/* ARGSUSED */
static void
do_answer(int fd, short event, void *arg)
{
	char buf[MAXEDNS];
	int byte = 0;
	struct request *query = NULL;

	/* Reschedule event */
	event_add((struct event *)arg, NULL);

	/* read packet from socket */
	if ((byte = recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL)) == -1) {
		error("recvfrom failed: %s", strerror(errno));
		++dropped_answers;
		return;
	}

	/* check for minimum dns packet length */
	if (byte < 12) {
		error("answer too short");
		++dropped_answers;
		return;
	}

	/* find corresponding query */
	if ((query = hash_find_request(*((unsigned short *)&buf))) == NULL) {
		++late_answers;
		return;
	}
	event_del(&query->timeout);
	hash_remove_request(query);

	/* restore original query id */
	memcpy(&buf[0], &query->clientid, 2);

	/* send answer back to querying host */
	if (sendto(sock_query, buf, (unsigned int)byte, 0,
			    (struct sockaddr *)&query->client,
			    sizeof(struct sockaddr_in)) == -1) {
		error("sendto failed: %s", strerror(errno));
		++dropped_answers;
	} else
		++answered_queries;

	free(query);
}

/* main -- dnsproxy main function
 */

int
main(int argc, char *argv[])
{
	int ch;
	struct passwd *pw = NULL;
	struct sockaddr_in addr;
	struct event evq, eva;
	struct event evsigint, evsigterm, evsighup;
	const char *config = "/etc/dnsproxy.conf";
	int daemonize = 0;

	/* Process commandline arguments */
	while ((ch = getopt(argc, argv, "c:dhV")) != -1) {
		switch (ch) {
		case 'c':
			config = optarg;
			break;
		case 'd':
			daemonize = 1;
			break;
		case 'V':
			fprintf(stderr, PACKAGE_STRING "\n");
			exit(0);
		/* FALLTHROUGH */
		case 'h':
		default:
			fprintf(stderr,
			"usage: dnsproxy [-c file] [-dhV]\n"		\
			"\t-c file  Read configuration from file\n"	\
			"\t-d       Detach and run as a daemon\n"	\
			"\t-h       This help text\n"			\
			"\t-V       Show version information\n");
			exit(1);
		}
	}

	/* Parse configuration and check required parameters */
	if (!parse(config))
		fatal("unable to parse configuration");

	if (!authoritative || !recursive)
		fatal("No authoritative or recursive server defined");

	if (!listenat)
		listenat = strdup("0.0.0.0");

	/* Create and bind query socket */
	if ((sock_query = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		fatal("unable to create socket: %s", strerror(errno));

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_addr.s_addr = inet_addr(listenat);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;

	if (bind(sock_query, (struct sockaddr *)&addr, sizeof(addr)) != 0)
		fatal("unable to bind socket: %s", strerror(errno));

	/* Create and bind answer socket */
	if ((sock_answer = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		fatal("unable to create socket: %s", strerror(errno));

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;

	if (bind(sock_answer, (struct sockaddr *)&addr, sizeof(addr)) != 0)
		fatal("unable to bind socket: %s", strerror(errno));

	/* Fill sockaddr_in structs for both servers */
	memset(&authoritative_addr, 0, sizeof(struct sockaddr_in));
	authoritative_addr.sin_addr.s_addr = inet_addr(authoritative);
	authoritative_addr.sin_port = htons(authoritative_port);
	authoritative_addr.sin_family = AF_INET;

	memset(&recursive_addr, 0, sizeof(struct sockaddr_in));
	recursive_addr.sin_addr.s_addr = inet_addr(recursive);
	recursive_addr.sin_port = htons(recursive_port);
	recursive_addr.sin_family = AF_INET;

	/* Daemonize if requested and switch to syslog */
	if (daemonize) {
		if (daemon(0, 0) == -1)
			fatal("unable to daemonize");
		log_syslog("dnsproxy");
	}

	/* Find less privileged user */
	if (user) {
		pw = getpwnam(user);
		if (!pw)
			fatal("unable to find user %s", user);
	}

	/* Do a chroot if requested */
	if (chrootdir) {
		if (chroot(chrootdir) == -1)
			fatal("unable to chroot to %s", chrootdir);
		if (chdir("/") == -1)
			fatal("unable to chdir");
	}

	/* Drop privileges */
	if (user) {
		if (setgroups(1, &pw->pw_gid) < 0)
			fatal("setgroups: %s", strerror(errno));
#if defined(HAVE_SETRESGID)
		if (setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) < 0)
			fatal("setresgid: %s", strerror(errno));
#elif defined(HAVE_SETREGID)
		if (setregid(pw->pw_gid, pw->pw_gid) < 0)
			fatal("setregid: %s", strerror(errno));
#else
		if (setegid(pw->pw_gid) < 0)
			fatal("setegid: %s", strerror(errno));
		if (setgid(pw->pw_gid) < 0)
			fatal("setgid: %s", strerror(errno));
#endif
#if defined(HAVE_SETRESUID)
		if (setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid) < 0)
			fatal("setresuid: %s", strerror(errno));
#elif defined(HAVE_SETREUID)
		if (setreuid(pw->pw_uid, pw->pw_uid) < 0)
			fatal("setreuid: %s", strerror(errno));
#else
		if (seteuid(pw->pw_uid) < 0)
			fatal("seteuid: %s", strerror(errno));
		if (setuid(pw->pw_uid) < 0)
			fatal("setuid: %s", strerror(errno));
#endif
	}

	event_init();

	/* Take care of signals */
	signal_set(&evsigint, SIGINT, signal_handler, NULL);
	signal_set(&evsigterm, SIGTERM, signal_handler, NULL);
	signal_set(&evsighup, SIGHUP, signal_handler, NULL);
	signal_add(&evsigint, NULL);
	signal_add(&evsigterm, NULL);
	signal_add(&evsighup, NULL);

	/* Zero counters and start statistics timer */
	statistics_start();

	/* Install query and answer event handlers */
	event_set(&evq, sock_query, EV_READ, do_query, &evq);
	event_set(&eva, sock_answer, EV_READ, do_answer, &eva);
	event_add(&evq, NULL);
	event_add(&eva, NULL);

	/* Start libevent main loop */
	event_dispatch();

	return 0;
}
