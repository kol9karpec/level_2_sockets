#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "networking.h"
#define ARGS_NUM 2

static void print_help(FILE * file);

int main(const int argc, const char * argv[]) {
	int connection_socket = 0;
	signal(SIGINT, sigint_handler);

	if (argc < ARGS_NUM) {
		goto print_help;
	} else {
		if (strcmp(argv[1], "connect") == 0) {
			if((connection_socket = run_connect((char *)argv[2], (char *)argv[3])) == -1) {
				goto error;
			}
		} else if (strcmp(argv[1], "wait") == 0) {
			if((connection_socket = run_wait((char *)argv[2])) == -1) {
				goto error;
			}
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
