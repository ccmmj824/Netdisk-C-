OBJS:=$(wildcard *.c)
ELF:=server
CC:=gcc
CFLAGS:=-g -Wall
$(ELF):$(OBJS)
	gcc $^ -o $@ -pthread -lcrypt -I ../include
clean:
		rm -rf  $(ELF)

