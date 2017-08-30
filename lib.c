#include "./lib.h"

static void print_delimiter(FILE * stream, char delim, int count);
static void print_mac(FILE * stream, const uint8_t * mac);
static void parse_ehternet(FILE * stream, const void * data);
static void parse_ip(FILE * stream, const void * data);
//static void parse_ipv6(FILE * stream, void * data);
//static void parse_tcp(FILE * stream, void * data);
//static void parse_udp(FILE * stream, void * data);

static void fprintf_binary(FILE * stream,
		const void * data,
		unsigned int size);

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
	parse_ehternet(stream,data);
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
	printf("\n");
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

static inline void print_mac(FILE * stream, const uint8_t * mac) {
	fprintf(stream,
			"%02x:%02x:%02x:%02x:%02x:%02x",
			mac[0],	mac[1],	mac[2],	mac[3],	mac[4],	mac[5]);
}

static void parse_ehternet(FILE * stream, const void * data) {
	struct ether_header eth_header;
	memset(&eth_header,0,sizeof(eth_header));

	eth_header = *(struct ether_header*)(data);

	fprintf(stream,"---Ethernet header---\n");
	fprintf(stream,"DST: ");
	print_mac(stream,eth_header.ether_dhost);
	fprintf(stream,"\tSRC: ");
	print_mac(stream,eth_header.ether_shost);
	fprintf(stream,"\tTYPE: ");

	data += sizeof(struct ether_header);

	switch(ntohs(eth_header.ether_type)) {
		case ETHERTYPE_IP:
			fprintf(stream,"IP");
			print_delimiter(stream,'~',3*BYTES_IN_ROW);
			parse_ip(stream,data);
			break;
		case ETHERTYPE_PUP:
			fprintf(stream,"Xerox PUP");
			print_delimiter(stream,'~',3*BYTES_IN_ROW);
			break;
		case ETHERTYPE_SPRITE:
			fprintf(stream,"Sprite");
			print_delimiter(stream,'~',3*BYTES_IN_ROW);
			break;
		case ETHERTYPE_ARP:
			fprintf(stream,"Address resolution");
			print_delimiter(stream,'~',3*BYTES_IN_ROW);
			break;
		case ETHERTYPE_REVARP:
			fprintf(stream,"Reverse ARP");
			print_delimiter(stream,'~',3*BYTES_IN_ROW);
			break;
		case ETHERTYPE_AT:
			fprintf(stream,"AppleTalk protocol");
			print_delimiter(stream,'~',3*BYTES_IN_ROW);
			break;
		case ETHERTYPE_AARP:
			fprintf(stream,"AppleTalk ARP");
			print_delimiter(stream,'~',3*BYTES_IN_ROW);
			break;
		case ETHERTYPE_VLAN:
			fprintf(stream,"IEEE 802.1Q VLAN tagging");
			print_delimiter(stream,'~',3*BYTES_IN_ROW);
			break;
		case ETHERTYPE_IPX:
			fprintf(stream,"IPX");
			print_delimiter(stream,'~',3*BYTES_IN_ROW);
			break;
		case ETHERTYPE_IPV6:
			fprintf(stream,"IPV6");
			print_delimiter(stream,'~',3*BYTES_IN_ROW);
			break;
		case ETHERTYPE_LOOPBACK:
			fprintf(stream,"Loopback");
			print_delimiter(stream,'~',3*BYTES_IN_ROW);
			break;
		default:
			fprintf(stream,"Unknown packet type: %#06x",
					ntohs(eth_header.ether_type));
	}
}

static void parse_ip(FILE * stream, const void * data) {
	struct iphdr ip_header;
	memset(&ip_header,0,sizeof(ip_header));

	ip_header = *(struct iphdr*)(data);

	fprintf(stream,"---IP header---\n");
	fprintf(stream,"Version: %d\n",ip_header.version);
	fprintf(stream,"Internet Header Length: %d\n",ip_header.ihl);
	fprintf(stream,"Type of service: ");
	fprintf_binary(stream,(void*)&ip_header.tos,sizeof(ip_header.tos));
	fprintf(stream,"\nTotal length: %d\n",ntohs(ip_header.tot_len));
	fprintf(stream,"Identification: %#06x\n",ntohs(ip_header.id));
	fprintf(stream,"Fragment Offset: %#06x\n",ntohs(ip_header.frag_off));
	fprintf(stream,"TTL: %d\n",ip_header.ttl);
	fprintf(stream,"Protocol: %d\n",ip_header.protocol);
	fprintf(stream,"Checksum: %#06x\n",ip_header.check);

	struct in_addr buf_addr = {ip_header.saddr};
	fprintf(stream,"Source: %s\n",inet_ntoa(buf_addr));
	buf_addr.s_addr = ip_header.daddr;
	fprintf(stream,"Destination: %s",inet_ntoa(buf_addr));
}

/*
static void parse_ipv6(FILE * stream, void * data) {

}

static void parse_tcp(FILE * stream, void * data) {

}

static void parse_udp(FILE * stream, void * data) {

}
*/

static inline void fprintf_binary(FILE * stream,
		const void * data,
		unsigned int size) {
	unsigned char * _data = (unsigned char *)data;
	unsigned int i = 0, j = 0;
	unsigned char cur_byte = 0;

	for(;i<size;i++) {
		cur_byte = _data[i];
		for(j=0;j<8;j++) {
			fprintf(stream,"%c",((cur_byte & 0x80) ? '1' : '0'));
			cur_byte <<= 1;
		}
	}
}
