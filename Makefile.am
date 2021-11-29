sbin_PROGRAMS = dnsproxy
dnsproxy_SOURCES = dnsproxy.c dnsproxy.h \
                   daemon.c hash.c internal.c log.c parse.c stats.c

man_MANS = dnsproxy.1

distclean-local:
	rm -rf autom4te.cache dnsproxy-[0-9]*
	rm -f aclocal.m4 compile config.* configure depcomp install-sh \
              INSTALL Makefile.in missing
