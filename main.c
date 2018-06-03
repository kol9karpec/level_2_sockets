#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>

#include "gamelib.h"
#include "networking.h"
#include "common.h"

#define ARGS_NUM 1

static void print_help(FILE * file);

int main(const int argc, const char * argv[]) {
	signal(SIGINT, sigint_handler);

	if (argc < ARGS_NUM) {
		goto print_help;
	} else {
		_connection_socket_fd = open_socket();

		if (strcmp(argv[1], "connect") == 0) {
			int res = run_connect((char *)argv[2]);
			if(res == -1) {
				goto error;
			} else if (res == -2) {
				printf("Connection denied!\n");
			}

			start_game(X);
		} else if (strcmp(argv[1], "wait") == 0) {
			if(run_wait()) {
				goto error;
			}

			start_game(O);
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
