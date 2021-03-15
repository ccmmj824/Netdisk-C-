OBJS:=$(wildcard *.c)
ELF:=client
CC:=gcc
CFLAGS:=-g -Wall
$(ELF):$(OBJS)
		gcc $^ -o $@ -pthread

clean:
		rm -rf  $(ELF)

