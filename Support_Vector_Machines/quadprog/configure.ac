# -*- Autoconf -*-
# This file is part of QuadProg++: a C++ library implementing
# the algorithm of Goldfarb and Idnani for the solution of a (convex) 
# Quadratic Programming problem by means of an active-set dual method.
# Copyright (C) 2007-2009 Luca Di Gaspero.
# Copyright (C) 2009 Eric Moyer. 
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([QuadProg], [1.2], [l.digaspero@uniud.it])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.6.3 foreign])

AC_LANG_CPLUSPLUS
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.
AX_BOOST_BASE([1.34], [have_boost="yes"], [have_boost="no"])
AM_CONDITIONAL(HAVE_BOOST, test "$have_boost" != no)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE

# Checks for library functions.
AC_CHECK_FUNCS([pow sqrt])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
