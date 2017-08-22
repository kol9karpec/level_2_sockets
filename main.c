#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

#define DEF_BUFSIZE 256

void die(const char * str);

int main(void) {
	int raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	if(raw_socket == -1) {
		die(strerror(errno));
	}

	while(1) {

	}



	close(raw_socket);

	return 0;
}

void die(const char * str) {
	printf("%s\n",str);
	exit(1);
}
