
SUBDIRS = proto test
bin_PROGRAMS = ulin
ulin_SOURCES = main.cc handler.cc buffer.cc loadcsv.cc proto/ulin.pb.cc proto/test.pb.cc
ulin_CXXFLAGS = -std=c++11 -O2 -g -Wall -Wdisabled-optimization
ulin_LDFLAGS = -lboost_system -lboost_thread -luv -lprotobuf -lcsv
