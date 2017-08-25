CC=gcc
CFLAGS= -Wall -Werror 
OBJS=main.o lib.o 

main: $(OBJS)

clean: 
	@rm -f main.o main $(OBJS) 

.PHONY: clean
