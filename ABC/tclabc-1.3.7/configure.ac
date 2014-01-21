dnl Configure source for tclabc
dnl Copyright (C) 1999-2012 Jean-Francois Moine.

AC_INIT(tclabc, 1.3.7)
AC_CONFIG_SRCDIR(abcparse.h)

AC_PROG_INSTALL
AC_PROG_CC

if test "X$GCC" = "Xyes" ; then
	CFLAGS="$CFLAGS -Wall -pipe"
fi

dnl Check for sound card
AC_MSG_CHECKING(checking for sound card... )
if test -f /usr/include/linux/soundcard.h \
	-o -f /usr/include/machine/soundcard.h; then
	AC_DEFINE(HAVE_SOUND)
	AC_MSG_RESULT(yes)

dnl Check for SB AWE 32
	AC_MSG_CHECKING(checking for SB AWE 32... )
	if test -f /usr/include/awe_voice.h \
		-o -f /usr/include/linux/awe_voice.h; then
		AC_DEFINE(HAVE_AWE)
		AC_MSG_RESULT(yes)
	else
		AC_MSG_RESULT(no)
	fi
dnl Check for the ALSA library
	AC_MSG_CHECKING(checking for the ALSA library... )
	if test -f /usr/include/sys/asoundlib.h; then
		AC_DEFINE(HAVE_ALSA)
		SHLIB_LD_LIBS="$SHLIB_LD_LIBS -lasound"
		AC_MSG_RESULT(yes)
	else
		AC_MSG_RESULT(no)
	fi
else
	AC_MSG_RESULT(no)
fi

dnl Check for tcl/tk
AC_MSG_CHECKING(checking for tcl/tk... )
TCLTKDIR=/usr/lib/tcl8.5/
AC_ARG_WITH(tcltk,
  [  --with-tcltk=directory  define where tclConfig.sh is installed],
  [TCLTKDIR="$withval"])
if test -f $TCLTKDIR/tclConfig.sh; then
	AC_MSG_RESULT(yes)
	tcltk=yes
	. $TCLTKDIR/tclConfig.sh
	SHLIB_CFLAGS="$SHLIB_CFLAGS $TCL_SHLIB_CFLAGS"
else
	AC_MSG_RESULT(no)
	tcltk=no
fi

AC_SUBST(SHLIB_CFLAGS)
AC_SUBST(TCL_SHLIB_LD)
AC_SUBST(TCL_INCLUDE_SPEC)
AC_SUBST(TCL_LIB_SPEC)
AC_SUBST(SHLIB_LD_LIBS)
AC_SUBST(TCL_SHLIB_SUFFIX)

VERSION=$PACKAGE_VERSION
AC_SUBST(VERSION)
AC_DEFINE_UNQUOTED(VERSION, "$VERSION")

AC_CONFIG_FILES(Makefile abcbarchk abctrans mid2abc tkabc pkgIndex.tcl)
AC_CONFIG_HEADERS(config.h)
AC_OUTPUT

if test $tcltk = no; then
	echo ""
	echo "* The Tcl/Tk configuration script tclConfig.sh"
	echo "* was not found by configure under $TCLTKDIR."
	echo "* If you do have installed Tcl/Tk under a different directory,"
	echo "* you may rerun the configure script with the option"
	echo "* --with-tcltk=mydir"
	echo ""
	exit 1
fi
