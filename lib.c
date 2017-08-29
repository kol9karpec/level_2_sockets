#include "./lib.h"

void die(const char * str, int _errno) {
	printf("%s: %s\n",str,strerror(_errno));
	exit(1);
}

void fprintf_data_hex(FILE * stream,
		const void * data,
		const unsigned int size) {
	unsigned int i = 0;
	unsigned char * _data = (unsigned char *)(data);

	for(;i < size;i++) {
		if(((i % BYTES_IN_ROW) == 0) && (i != 0))
			fprintf(stream, "\n");

		fprintf(stream, "%02x ", _data[i]);
	}
}

char * printf_packet(char * buf,
		const unsigned int bufsize,
		const void * data,
		const unsigned int size,
		const struct sockaddr_ll * _sockaddr_ll) {
	//TODO: Implement the function
	return NULL;
}

void capture_packet(int _socket) {
	struct sockaddr_ll src_addrll;
	socklen_t src_addrll_len = sizeof(src_addrll);
	int bytes_received = 0;

	char print_buffer[BIG_BUFSIZE] = {0};
	char buffer[DEF_PKTBUFSIZE] = {0};

	if((bytes_received = recvfrom(_socket,
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
		printf("------------------------------------------------\n");
		fprintf_data_hex(stdout,
						(void*)buffer,
						bytes_received);
		printf("%s\n",print_buffer);
		printf("------------------------------------------------\n");

}

void sigint_handler(int _socket) {
	close(_socket);
	exit(0);
}


void to_promiscuous(const char * _if_name, const int _socket) {
	/*Putting eno1 into the promiscuous mode*/
	struct ifreq ifr;
	strncpy(ifr.ifr_name,_if_name,strlen(_if_name)+1);
	if(ioctl(_socket, SIOCGIFFLAGS, &ifr)<0) {
		die("ioctl()",errno);
	}

	ifr.ifr_flags |= IFF_PROMISC;
	if( ioctl(_socket, SIOCSIFFLAGS, &ifr) != 0 ) {
		die("ioctl()",errno);
	}

	/*Putting socket into promiscuous mode*/
	if(ioctl(_socket, SIOCGIFINDEX, &ifr)<0) {
		die("ioctl()",errno);
	}

	struct packet_mreq mr;
	memset(&mr, 0, sizeof(mr));
	mr.mr_ifindex = ifr.ifr_ifindex;
	mr.mr_type = PACKET_MR_PROMISC;
	if(setsockopt(_socket,
				SOL_PACKET,
				PACKET_ADD_MEMBERSHIP,
				&mr,
				sizeof(mr)) < 0) {
		die("setsockopt()",errno);
	}
}

