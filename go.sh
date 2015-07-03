#!/bin/sh

protoc --cpp_out=../ ./proto/*.proto

g++ -std=c++11 -g3 -DBOOST_SYSTEM_NO_DEPRECATED *.cc -o ../a.out -lboost_system -lboost_thread -luv -lprotobuf && echo 'starting...' && ../a.out
