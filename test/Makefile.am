
bin_PROGRAMS = client
client_SOURCES = client.cc handler.cc
client_LDADD = $(top_builddir)/src/libulin.a
client_CXXFLAGS = -std=c++11 -O2 -g -Wall -Wdisabled-optimization -DBOOST_LOG_DYN_LINK -I$(top_srcdir)/include
client_LDFLAGS = -pthread -lboost_log -lboost_log_setup -lboost_system -lboost_thread -luv -lprotobuf
