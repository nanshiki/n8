dnl Process this file with autoconf to produce a configure script.
AC_INIT(src/n8.c)
AC_CONFIG_HEADERS(src/config.h)
AC_CONFIG_SUBDIRS(lib)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_INSTALL


dnl Checks for libraries.
AC_CHECK_LIB(iconv, iconv_open,,)
AC_CHECK_LIB(termcap, tgetent,,
AC_CHECK_LIB(ncurses, tgetent,,
AC_CHECK_LIB(curses, tgetent)))


dnl Checks for header files.
AC_HEADER_DIRENT
AC_CHECK_HEADERS(fcntl.h unistd.h sys/time.h)
AC_CHECK_HEADERS(stdarg.h varargs.h)
AC_CHECK_HEADERS(regex.h)
AC_CHECK_HEADERS(sys/utsname.h)


dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_PID_T
AC_STRUCT_TM


dnl Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
dnl AC_FUNC_ALLOCA
AC_CHECK_FUNCS(getcwd mkdir rmdir select uname)


AC_CONFIG_FILES(Makefile etc/Makefile src/Makefile)
AC_OUTPUT
