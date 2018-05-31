CC=gcc
CFLAGS= -Wall -Werror -pedantic
OBJS=main.o networking.o

main: $(OBJS)

clean: 
	@rm -f main $(OBJS) 

.PHONY: clean
