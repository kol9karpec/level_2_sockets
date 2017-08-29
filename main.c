#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "./lib.h"


int main(const int argc, const char * argv[]) {
	int ethernet_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	if(ethernet_socket == -1) {
		die("socket()",errno);
	}

	to_promiscuous("eno1",ethernet_socket);

	signal(SIGINT,sigint_handler);

	while(1) {
		capture_packet(ethernet_socket,stdout);
	}

	/*Unreachable*/
	return 0;
}


