
SUBDIRS = proto src test
bin_PROGRAMS = ulin
ulin_SOURCES = daemon.cc
ulin_LDADD = $(top_builddir)/src/libulin.a
ulin_CXXFLAGS = -std=c++11 -O2 -g -Wall -Wdisabled-optimization -DBOOST_LOG_DYN_LINK -I$(top_srcdir)/include
ulin_LDFLAGS = -pthread -lboost_log -lboost_log_setup -lboost_system -lboost_thread -luv -lprotobuf -lcsv
