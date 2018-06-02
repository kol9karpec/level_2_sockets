#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>

#include "gamelib.h"
#include "networking.h"
#define ARGS_NUM 2

static void print_help(FILE * file);

field_t play_field = {
	.size = 3,
	.matr = NULL
};

int main(const int argc, const char * argv[]) {
	int connection_socket = 0;
	signal(SIGINT, sigint_handler);

	if (argc < ARGS_NUM) {
		goto print_help;
	} else {
		if (strcmp(argv[1], "connect") == 0) {
			if((connection_socket = run_connect((char *)argv[2],
							atoi((char *)argv[3]),
							atoi((char *)argv[4]))) == -1) {
				goto error;
			}
			//START game
		} else if (strcmp(argv[1], "wait") == 0) {
			if((connection_socket = run_wait(atoi((char *)argv[2]))) == -1) {
				goto error;
			}
			//Start game
		} else {
			goto print_help;
		}
	}

	return 0;
error:
	printf("Error!");
	return 1;
print_help:
	print_help(stdout);
	return 1;
}

static void print_help(FILE * file) {
	fprintf(file, "Hello help!\n");
}
