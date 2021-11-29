AC_INIT(dnsproxy, 1.17)
AC_PREREQ(2.60)

AC_CONFIG_HEADERS(config.h)
AM_INIT_AUTOMAKE

dnl ------------------------------------------------------------------
dnl Check for programs
dnl ------------------------------------------------------------------

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_CACHE_SAVE

dnl ------------------------------------------------------------------
dnl Check for headers
dnl ------------------------------------------------------------------

AC_USE_SYSTEM_EXTENSIONS
AC_C_CONST
AC_HEADER_STDC
AC_HEADER_TIME
AC_CHECK_HEADERS(sys/types.h sys/socket.h sys/time.h)
AC_CHECK_HEADERS(netinet/in.h arpa/inet.h)
AC_CHECK_HEADERS(errno.h syslog.h)
AC_TYPE_SIGNAL
AC_CACHE_SAVE

dnl ------------------------------------------------------------------
dnl Check for functions
dnl ------------------------------------------------------------------

AC_CHECK_FUNCS(strchr memcpy)
AC_CHECK_FUNCS(setregid setresgid setresuid setreuid)
AC_CACHE_SAVE

dnl ------------------------------------------------------------------
dnl Check for additional libraries
dnl ------------------------------------------------------------------

AC_SEARCH_LIBS([gethostbyname], [nsl])
AC_SEARCH_LIBS([socket], [socket], [], [
	AC_CHECK_LIB([socket], [socket], [LIBS="-lsocket -lnsl $LIBS"], [],
		[-lnsl])
])
AC_CACHE_SAVE

dnl ------------------------------------------------------------------
dnl Enable extended warnings while developing
dnl ------------------------------------------------------------------

AC_ARG_ENABLE(warnings, [
AC_HELP_STRING([--enable-warnings],
[enable all sorts of warnings for debugging])],
[
	CFLAGS="${CFLAGS} -Wall -Werror -Wcast-qual -Wmissing-declarations \
		-W -Wmissing-prototypes -Wnested-externs -Wshadow \
		-Wwrite-strings -Wno-unused -Wno-sign-compare"
])
AC_SUBST(CFLAGS)

dnl ------------------------------------------------------------------
dnl Maximum EDNS packet size
dnl ------------------------------------------------------------------

AC_ARG_ENABLE([edns], [
AC_HELP_STRING([--enable-edns=N],
[maximum EDNS packet size (default is 4096)])],
[], [enable_edns=4096])
AS_IF([test $enable_edns = no], [enable_edns=4096])
AC_DEFINE_UNQUOTED([MAXEDNS],[$enable_edns],[Maximum EDNS packet size])

dnl ------------------------------------------------------------------
dnl Check for libevent library
dnl ------------------------------------------------------------------

lefound="no"
LIBS="$LIBS -levent"
AC_MSG_CHECKING(for libevent)
AC_ARG_WITH(libevent,
	AC_HELP_STRING(
		[--with-libevent=PATH],
		[directory prefix where libevent is found]
	),
	[
		if test -f $withval/include/event.h; then
			CFLAGS="${CFLAGS} -I$withval/include"
		elif test -f $withval/event.h; then
			CFLAGS="${CFLAGS} -I$withval"
		else
			AC_ERROR([event.h not found])
		fi
		if test -f $withval/lib; then
			LDFLAGS="${LDFLAGS} -L$withval/lib"
		else
			LDFLAGS="${LDFLAGS} -L$withval"
		fi
		AC_MSG_RESULT([using $withval])
		lefound="yes"
	],
	[
		saved_CFLAGS=$CFLAGS
		saved_LDFLAGS=$LDFLAGS
		for testdir in "" $prefix /usr/local /opt/csw; do
			if test -z "$testdir"; then
				CFLAGS="$saved_CFLAGS"
				LDFLAGS="$saved_LDFLAGS"
			else
				CFLAGS="$saved_CFLAGS -I$testdir/include"
				LDFLAGS="$saved_LDFLAGS -L$testdir/lib"
			fi

			AC_TRY_LINK([#include <sys/time.h>
			#include <sys/types.h>
			#include <event.h>],
			[ event_init(); ],
		        [ lefound="$testdir" ],
			[ lefound="no" ])

			if test "$lefound" != "no"; then
				if test -z "$testdir"; then
					AC_MSG_RESULT([found])
				else
					AC_MSG_RESULT([found in $lefound])
				fi
				break
			fi
		done
	]
)

if test "$lefound" = "no"; then
	AC_MSG_ERROR([
	This software requires the libevent library
	available at http://libevent.org/

	You may specify it's directory prefix with
	./configure --with-libevent=/prefix/of/libevent
	])
fi

dnl ------------------------------------------------------------------
dnl Check for available nroff and manpage macro package
dnl ------------------------------------------------------------------

AC_PATH_PROGS([NROFF], [nroff awf], [/bin/false], [$PATH:/usr/ucb])
if ${NROFF} -mdoc dnsproxy.8 >/dev/null 2>&1; then
	MAN=mdoc
else
	MAN=man
fi
AC_SUBST(MAN)

dnl ------------------------------------------------------------------
dnl Generate Makefile by default. Others only if their .in file
dnl exists in the current directory, which happens in my workdir
dnl but not for distributed tarballs.
dnl ------------------------------------------------------------------

AC_CONFIG_FILES(Makefile)
if test -r "dnsproxy.8.in"; then
	AC_CONFIG_FILES(dnsproxy.8)
fi

AC_OUTPUT()

AC_MSG_RESULT()
AC_MSG_RESULT([Configured successfully. Now run make.])
AC_MSG_RESULT()
