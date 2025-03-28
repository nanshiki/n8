dnl Process this file with autoconf to produce a configure script.
AC_INIT(generic.c)
AC_CONFIG_HEADERS(config.h)


dnl Checks for programs.
AC_PROG_CC


dnl Checks for header files.
AC_HEADER_DIRENT
AC_CHECK_HEADERS(fcntl.h unistd.h sys/time.h termcap.h)
AC_CHECK_HEADERS(stdarg.h varargs.h)
AC_CHECK_HEADERS(termio.h termios.h)
AC_CHECK_HEADERS(sys/poll.h)
AC_CHECK_HEADERS(regex.h)


dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_STRUCT_TM

dnl AC_CHECK_TYPE(speed_t, short)


dnl Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
dnl AC_FUNC_ALLOCA
AC_CHECK_FUNCS(mkdir select strstr strdup regcomp flock realpath strsep)

dnl  AC_FUNC_UTIME_NULL


AC_CHECK_LIB(termcap, tgetent,,
AC_CHECK_LIB(ncurses, tgetent,,
AC_CHECK_LIB(curses, tgetent)))

AC_CHECK_FUNCS(tparm tparam)


AC_CONFIG_FILES(Makefile)
AC_OUTPUT()
