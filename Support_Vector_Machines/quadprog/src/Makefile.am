lib_LTLIBRARIES = libQuadProgpp.la 
libQuadProgpp_la_SOURCES = QuadProg++.cc Array.cc
include_HEADERS = QuadProg++.hh Array.hh 

bin_PROGRAMS = test

test_SOURCES = main.cc 
test_LDADD = libQuadProgpp.la

if HAVE_BOOST

lib_LTLIBRARIES += libuQuadProgpp.la
libuQuadProgpp_la_SOURCES = uQuadProg++.cc
libuQuadProgpp_la_CPPFLAGS = @BOOST_CPPFLAGS@
libuQuadProgpp_la_LDFLAGS = @BOOST_LDFLAGS@
include_HEADERS += uQuadProg++.hh

bin_PROGRAMS += utest

utest_SOURCES = umain.cc 
utest_CPPFLAGS = @BOOST_CPPFLAGS@
utest_LDFLAGS = @BOOST_LDFLAGS@
utest_LDADD = libuQuadProgpp.la

endif