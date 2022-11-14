#--------------------------------------------------------------------
# Makefile for Assignment 4, Part 3 sample implementation
# Author: Christopher Moretti
#--------------------------------------------------------------------

CC=gcc217

all: ft

clean:
	rm -f ft

clobber: clean
	rm -f ft_client.o *~

ft: ft.o ft_client.o
	$(CC) ft.o ft_client.o -o ft

ft_client.o: ft_client.c ft.h a4def.h
	$(CC) -c ft_client.c

ft.o: ft.c dirNode.h fileNode.h a4def.h
	$(CC) -c ft.c
