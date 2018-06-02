CC=gcc
CFLAGS= -Wall -Werror 
OBJS=main.o networking.o gamelib.o

main: $(OBJS)

clean: 
	@rm -f main $(OBJS) 

.PHONY: clean
