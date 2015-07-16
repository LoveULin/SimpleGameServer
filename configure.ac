#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ULinGameServer], [1.0.0], [188002324@qq.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AM_PROG_AR
AC_CONFIG_SRCDIR([main.cc])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lboost_log':
AC_CHECK_LIB([boost_log], [add_file_log])
# FIXME: Replace `main' with a function in `-lboost_log_setup':
AC_CHECK_LIB([boost_log_setup], [main])
# FIXME: Replace `main' with a function in `-lboost_system':
AC_CHECK_LIB([boost_system], [main])
# FIXME: Replace `main' with a function in `-lboost_thread':
AC_CHECK_LIB([boost_thread], [main])
AC_CHECK_LIB([csv], [csv_parse])
AC_CHECK_LIB([protobuf], [ParseFromString])
AC_CHECK_LIB([uv], [uv_run])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memmove])

AC_CONFIG_FILES([Makefile
                 proto/Makefile
                 src/Makefile
                 test/Makefile])
AC_OUTPUT
