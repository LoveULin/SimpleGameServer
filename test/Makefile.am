
bin_PROGRAMS = client
client_SOURCES = client.cc ../buffer.cc ../handler.cc ../proto/ulin.pb.cc ../proto/test.pb.cc
client_CXXFLAGS = -std=c++11 -O2 -g -Wall -Wdisabled-optimization -DBOOST_LOG_DYN_LINK
client_LDFLAGS = -pthread -lboost_log -lboost_log_setup -lboost_system -lboost_thread -luv -lprotobuf
