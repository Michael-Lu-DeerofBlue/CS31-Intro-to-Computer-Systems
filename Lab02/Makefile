#
# A makefile for CS31, Fall 2021, Lab 2
#
# to build:
#   make
# to clean-up:
#   make clean
#

# this makefile uses some variables to specify the
# compiler, the flags to the compiler:

CC = gcc

CFLAGS = -g -Wall -Wvla -Werror -Wno-error=unused-variable


all: lab2

lab1: lab2.c
	$(CC) $(CFLAGS) -o lab1 lab2.c

clean:
	$(RM) lab2
