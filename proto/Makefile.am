
all: test.pb.cc ulin.pb.cc

test.pb.cc: test.proto
	protoc --cpp_out=./ test.proto

ulin.pb.cc: ulin.proto
	protoc --cpp_out=./ ulin.proto
