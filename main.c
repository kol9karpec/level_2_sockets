#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

#define DEF_BUFSIZE 256
#define DEF_PKTBUFSIZE 2000

void die(const char * str, int _errno);
void printf_data_hex(const void * data, const unsigned int size);

int main(void) {
	char buffer[DEF_PKTBUFSIZE] = {0};

	int raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(raw_socket == -1) {
		die("socket()",errno);
	}

	struct sockaddr_ll src_addrll;
	socklen_t src_addrll_len = sizeof(src_addrll);
	int bytes_received = 0;

	while(1) {
		if((bytes_received = recvfrom(raw_socket,
					buffer,
					sizeof(buffer),
					0,
					(struct sockaddr *)&src_addrll,
					&src_addrll_len)) < 0) {
			die("socket()",errno);
		}

		printf("Interface index: %d\n",src_addrll.sll_ifindex);
		printf("Hardware type: %u\n",src_addrll.sll_hatype);
		printf("Packet type: %u\n",(unsigned int)src_addrll.sll_pkttype);
		printf("Length addr: %u\n",(unsigned int)src_addrll.sll_halen);
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
		//printf("%.*s\n",bytes_received,buffer);
	}

	close(raw_socket);

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
