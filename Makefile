DEBUG_PRINTS=1
CC=gcc
CFLAGS=-Wall -Werror -DDEBUG_PRINTS=$(DEBUG_PRINTS)
OBJS=main.o networking.o gamelib.o common.o

main: $(OBJS)

clean: 
	@rm -f main $(OBJS) 

.PHONY: clean
