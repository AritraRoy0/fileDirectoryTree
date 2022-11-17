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

ft: ft.o ft_client.o dynarray.o path.o dirNode.o fileNode.o
	$(CC) ft.o ft_client.o dynarray.o path.o dirNode.o fileNode.o -o ft

ft_client.o: ft_client.c ft.h a4def.h
	$(CC) -c ft_client.c

ft.o: ft.c dirNode.h fileNode.h a4def.h dynarray.h ft.h path.h
	$(CC) -c ft.c

dynarray.o: dynarray.c dynarray.h
	$(CC) -c dynarray.c

path.o: path.c path.h
	$(CC) -c path.c

fileNode.o: fileNode.c path.h dynarray.h fileNode.h dirNode.h a4def.h
	$(CC) -c fileNode.c

dirNode.o: dirNode.c path.h dynarray.h fileNode.h dirNode.h a4def.h
	$(CC) -c dirNode.c