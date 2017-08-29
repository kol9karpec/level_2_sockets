#include "./lib.h"

static void print_delimiter(FILE * stream, char delim, int count);

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

void fprintf_packet(FILE * stream,
		const void * data,
		const unsigned int size,
		const struct sockaddr_ll * _sockaddr_ll) {
	fprintf(stream, 
			"MAC: %02x:%02x:%02x:%02x:%02x:%02x",
			_sockaddr_ll->sll_addr[0],
			_sockaddr_ll->sll_addr[1],
			_sockaddr_ll->sll_addr[2],
			_sockaddr_ll->sll_addr[3],
			_sockaddr_ll->sll_addr[4],
			_sockaddr_ll->sll_addr[5]);
	print_delimiter(stream,'-',3*BYTES_IN_ROW);
	fprintf_data_hex(stream,
			data,
			size);
	print_delimiter(stream,'-',3*BYTES_IN_ROW);
}

void capture_packet(int _socket, FILE * stream) {
	struct sockaddr_ll src_addrll;
	socklen_t src_addrll_len = sizeof(src_addrll);
	int bytes_received = 0;

	char buffer[DEF_PKTBUFSIZE] = {0};

	if((bytes_received = recvfrom(_socket,
					buffer,
					sizeof(buffer),
					0,
					(struct sockaddr *)&src_addrll,
					&src_addrll_len)) < 0) {
			die("socket()",errno);
	}

	fprintf_packet(stream,buffer,bytes_received,&src_addrll);
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

static inline void print_delimiter(FILE * stream, char delim, int count) {
	fprintf(stream,"\n");
	unsigned int i = 0;
	for(;i<count;i++) {
		fprintf(stream,"%c",delim);
	}
	
	fprintf(stream,"\n");
}
