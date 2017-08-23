#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

#define DEF_BUFSIZE 256
#define DEF_PKTBUFSIZE 2000

void die(const char * str, int _errno);

char * printf_data_hex(char * buf,
		const unsigned int bufsize,
		const void * data,
		const unsigned int size);

void printf_packet(const void * data,
		const unsigned int size,
		const struct sockaddr_ll * _sockaddr_ll);

int main(const int argc, const char * argv[]) {
	char buffer[DEF_PKTBUFSIZE] = {0};

	int ethernet_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(ethernet_socket == -1) {
		die("socket()",errno);
	}

	struct sockaddr_ll src_addrll;
	socklen_t src_addrll_len = sizeof(src_addrll);
	int bytes_received = 0;

	while(1) {
		if((bytes_received = recvfrom(ethernet_socket,
					buffer,
					sizeof(buffer),
					0,
					(struct sockaddr *)&src_addrll,
					&src_addrll_len)) < 0) {
			die("socket()",errno);
		}

		printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
				src_addrll.sll_addr[0],
				src_addrll.sll_addr[1],
				src_addrll.sll_addr[2],
				src_addrll.sll_addr[3],
				src_addrll.sll_addr[4],
				src_addrll.sll_addr[5]);
		printf("--------------------------------------\n");
		printf_data_hex((void*)buffer,bytes_received);
		printf("--------------------------------------\n");
	}

	close(ethernet_socket);

	return 0;
}

void die(const char * str, int _errno) {
	printf("%s: %s\n",str,strerror(_errno));
	exit(1);
}

void printf_data_hex(const void * data, const unsigned int size) {
	unsigned int i = 0;
	unsigned char * _data = (unsigned char * )(data);
	for(;i<size;i++) {
		if(i % 16 == 0) printf("\n");
		printf("%02X ",_data[i]);
	}
	printf("\n");
}

void printf_packet(const void * data,
		const unsigned int size,
		const struct sockaddr_ll * _sockaddr_ll) {
		
}
