#!/bin/sh

g++ -std=c++11 -g3 -DBOOST_SYSTEM_NO_DEPRECATED main.cc -o ../a.out -lboost_system -lboost_thread -luv && echo 'starting...' && ../a.out
