#!/bin/make
CFLAGS = $(shell xml2-config --cflags) -g
LDFLAGS= $(shell xml2-config --libs)
CC = gcc
CXX = g++

.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $<

.cpp.o:
	$(CXX) $(CFLAGS) -c -o $*.o $<

all: gpxtool

gpxtool: gpxtool.o
	$(CC) -o $@ $^ $(LDFLAGS) 

clean:
	rm *.o gpxtool