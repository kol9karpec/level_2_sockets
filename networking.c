#include "networking.h"

void die(const char * str, int _errno) {
	printf("%s: %s\n",str,strerror(_errno));
	exit(1);
}

char * printf_data_hex(char * buf,
		const unsigned int bufsize,
		const void * data,
		const unsigned int size) {
	char local_buffer[DEF_BUFSIZE] = {0};

	unsigned int i = 0;
	unsigned int bufsize_left = bufsize;
	unsigned int delta = 0;
	unsigned char * _data = (unsigned char *)(data);

	memset(buf, 0, bufsize);

	for(;((i < size) && (bufsize_left > 0));i++) {
		if(((i % BYTES_IN_ROW) == 0) && (i != 0))
			strncat(buf,"\n",bufsize_left--);

		delta = snprintf(local_buffer,
				DEF_BUFSIZE,
				"%02X ",
				_data[i]);

		if(delta > bufsize_left) {
			fprintf(stdout,"Not enoght size in buffer!\n");
			return NULL;
		} else {
			strncat(buf,local_buffer,delta);
			bufsize_left -= delta;
		}
	}

	return buf;
}

void sigint_handler(int _socket) {
	close(_socket);
	exit(0);
}

int run_wait(short our_port) {
	int s = open_socket();
	char buf[DATAGRAM_SIZE] = {0};
	int bufsize = 0;
	char receive_buf[DATAGRAM_SIZE] = {0};
	int receive_buf_len = 0;
	char src_ip_addr[16] = {0};
	short src_port = 0;

	packet_header_t header = {
		.type = CONNECTION
	};

	connection_packet_t packet = {
		.type = RESP,
		.code = ACCEPT
	};

	packet_header_t * rc_header = NULL;
	connection_packet_t * rc_packet = NULL;

	receive_buf_len = receive_packet(s, our_port, receive_buf, DATAGRAM_SIZE,
			&src_port, src_ip_addr);
	if (receive_buf_len < 0) {
		printf("receive_packet error!\n");
		return -1;
	}

	rc_header = (packet_header_t *)receive_buf;
	rc_packet = (connection_packet_t *)(receive_buf + sizeof(packet_header_t));

	if (rc_header->type == CONNECTION && rc_packet->type == REQ) {
		printf("Connection request received!\n");
	}

	memcpy(buf + bufsize, &header, sizeof(header));
	bufsize += sizeof(header);
	memcpy(buf + bufsize, &packet, sizeof(packet));
	bufsize += sizeof(packet);

	printf("ip_addr to sent = %s\n", src_ip_addr);
	printf("src_port = %d\n", src_port);
	send_packet(s, src_ip_addr, src_port, our_port, buf, bufsize);

	return 0;
}

int run_connect(char * ip_addr, short dest_port, short our_port) {
	int s = open_socket();
	char buf[DATAGRAM_SIZE] = {0};
	int bufsize = 0;
	char receive_buf[DATAGRAM_SIZE] = {0};
	int receive_buf_len = 0;

	packet_header_t header = {
		.type = CONNECTION
	};

	connection_packet_t packet = {
		.type = REQ,
		.code = ACCEPT
	};

	packet_header_t * rc_header = NULL;
	connection_packet_t * rc_packet = NULL;

	memcpy(buf + bufsize, &header, sizeof(header));
	bufsize += sizeof(header);
	memcpy(buf + bufsize, &packet, sizeof(packet));
	bufsize += sizeof(packet);

	send_packet(s, ip_addr, dest_port, our_port, buf, bufsize);

	receive_buf_len = receive_packet(s, our_port, receive_buf, DATAGRAM_SIZE,
			NULL, NULL);
	if (receive_buf_len < 0) {
		printf("receive_packet error!\n");
		return -1;
	}

	rc_header = (packet_header_t *)receive_buf;
	rc_packet = (connection_packet_t *)(receive_buf + sizeof(packet_header_t));

	if (rc_header->type == CONNECTION && rc_packet->type == RESP &&\
			rc_packet->code == ACCEPT) {
		printf("Connection accepted!\n");
	} else {
		printf("Connection denied!\n");
	}

	return 0;
}

int open_socket() {
	int s = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);
	int value = 1;
	if(s == -1)
	{
		perror("Failed to create socket");
		return -1;
	}
	setsockopt(s, IPPROTO_IP, SO_REUSEADDR, &value, sizeof(int));

	return s;
}

int send_packet(int sock_fd, char * ip_addr, short dest_port, short src_port,
		void * data, int size) {
	char datagram[DATAGRAM_SIZE],
		source_ip[32];

	unsigned datagram_size = sizeof(struct tcphdr) + size;

	struct sockaddr_in sin;
	struct tcphdr *tcph = (struct tcphdr *)(datagram);
	struct pseudo_header psh;
	char * pseudogram;
	int psize = sizeof(struct pseudo_header) + datagram_size;

	memset(datagram, 0, DATAGRAM_SIZE);
	memcpy(datagram + sizeof(struct tcphdr), data, size);

	strcpy(source_ip, "127.0.0.1");
	sin.sin_family = AF_INET;
	sin.sin_port = htons(dest_port);
	sin.sin_addr.s_addr = inet_addr(ip_addr);

	tcph->source = htons(src_port);
	tcph->dest = htons(dest_port);
	tcph->seq = 0;
	tcph->ack_seq = 0;
	tcph->doff = 5;
	tcph->fin=0;
	tcph->syn=1;
	tcph->rst=0;
	tcph->psh=0;
	tcph->ack=0;
	tcph->urg=0;
	tcph->window = htons(5840); /* maximum allowed window size */
	tcph->check = 0;
	tcph->urg_ptr = 0;

	psh.source_address = inet_addr(source_ip);
	psh.dest_address = sin.sin_addr.s_addr;
	psh.placeholder = 0;
	psh.protocol = IPPROTO_TCP;
	psh.tcp_length = htons(sizeof(struct tcphdr) + size);

	pseudogram = malloc(psize);

	memcpy(pseudogram, (char*)&psh, sizeof(struct pseudo_header));
	memcpy(pseudogram + sizeof(struct pseudo_header), tcph,
			sizeof(struct tcphdr) + size);

	tcph->check = csum((unsigned short*) pseudogram , psize);

	if (sendto(sock_fd, datagram, datagram_size,
				0, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		perror("sendto failed");
		return -1;
	} else {
		printf("Packet send. Length : %d \n" , datagram_size);
	}

	return 0;
}

int receive_packet(int sock_fd, short port, void * dest, unsigned size,
		short * src_port, char * src_ip) {
	struct tcphdr * tcph = NULL;
	struct iphdr * iphdr = NULL;
	char * data = NULL;
	char buf[DATAGRAM_SIZE];
	int len = 0;
	int data_len = 0;
	struct sockaddr_in server_addr = {0};
	char * src_ip_addr = NULL;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(bind(sock_fd, (struct sockaddr *)(&server_addr),
				sizeof(server_addr))) {
		perror("bind() error");
		return -1;
	}

	do {
		tcph = (struct tcphdr *)(buf + sizeof(struct iphdr));
		len = recv(sock_fd, buf, DATAGRAM_SIZE, 0);
	} while(ntohs(tcph->dest) != port);

	if (src_ip) {
		iphdr = (struct iphdr *)(buf);
		src_ip_addr = inet_ntoa(*((struct in_addr *)(&iphdr->saddr)));
		memcpy(src_ip, src_ip_addr, strlen(src_ip_addr));
	}
	if (src_port)
		*src_port = ntohs(tcph->source);
	data = (char *)(buf + sizeof(struct tcphdr) + sizeof(struct iphdr));

	data_len = len - sizeof(struct iphdr) - sizeof(struct tcphdr);
	memcpy(dest, data, data_len % size);

	return data_len;
}

/*
 *     Generic checksum calculation function
 */
unsigned short csum(unsigned short *ptr, int nbytes) 
{
	register long sum = 0;
	unsigned short oddbyte;
	register short answer;

	while(nbytes > 1) {
		sum += *ptr++;
		nbytes -= 2;
	}

	if(nbytes == 1) {
		oddbyte = 0;
		*((u_char*)&oddbyte) = *(u_char*)ptr;
		sum += oddbyte;
	}

	sum = (sum>>16) + (sum & 0xffff);
	sum = sum + (sum>>16);
	answer = (short)~sum;

	return(answer);
}
