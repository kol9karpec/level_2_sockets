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

int run_wait() {
	int s = open_socket();

	char buf[DATAGRAM_SIZE] = {0};
	int bufsize = 0;

	char receive_buf[DATAGRAM_SIZE] = {0};
	int receive_buf_len = 0;
	char src_ip_addr[16] = {0};

	packet_header_t header = {
		.type = CONNECTION
	};

	connection_packet_t packet = {
		.type = RESP,
		.code = ACCEPT
	};

	packet_header_t * rc_header = (packet_header_t *)receive_buf;
	connection_packet_t * rc_packet = (connection_packet_t *)(receive_buf +
			sizeof(packet_header_t));

	do {
		receive_buf_len = receive_packet(s, receive_buf, DATAGRAM_SIZE,
				src_ip_addr);
		if (receive_buf_len < 0) {
			perror("receive_packet() error");
			return -1;
		}
	} while(rc_header->type != CONNECTION || rc_packet->type != REQ);
	printf("Connection request received!\n");

	memcpy(buf, &header, sizeof(header));
	bufsize += sizeof(header);
	memcpy(buf + bufsize, &packet, sizeof(packet));
	bufsize += sizeof(packet);

	if (send_packet(s, src_ip_addr, buf, bufsize)) {
		perror("send_packet() error");
		return -1;
	}

	return s;
}

int run_connect(char * ip_addr) {
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

	packet_header_t * rc_header = (packet_header_t *)receive_buf;
	connection_packet_t * rc_packet = (connection_packet_t *)(receive_buf +
			sizeof(packet_header_t));

	memcpy(buf + bufsize, &header, sizeof(packet_header_t));
	bufsize += sizeof(packet_header_t);
	memcpy(buf + bufsize, &packet, sizeof(connection_packet_t));
	bufsize += sizeof(connection_packet_t);

	if(send_packet(s, ip_addr, buf, bufsize) < 0) {
		perror("send_packet() error");
		return -1;
	}

	do {
		receive_buf_len = receive_packet(s, receive_buf, DATAGRAM_SIZE, NULL);
		if (receive_buf_len < 0) {
			printf("receive_packet error!\n");
			return -1;
		}

	} while (rc_header->type != CONNECTION || rc_packet->type != RESP);

	if (rc_packet->code == ACCEPT) {
		printf("Connection accepted!\n");
	} else {
		printf("Connection denied!\n");
	}

	return s;
}

int open_socket() {
	int value = 1;
	int s = socket(AF_INET, SOCK_RAW, IPPROTO_TTT);
	if(s == -1)
	{
		perror("Failed to create socket");
		return -1;
	}

	if(setsockopt(s, IPPROTO_IP, SO_REUSEADDR, &value, sizeof(int)) < 0 )
	{
		perror("setsockopt() error");
		return -1;
	}

	return s;
}

int send_packet(int sock_fd, char * dest_ip_addr, void * data, int size) {
	char datagram[DATAGRAM_SIZE];
	unsigned datagram_size = size;
	struct sockaddr_in sin = {
		.sin_family = AF_INET,
		.sin_port = 1010,
		.sin_addr.s_addr = inet_addr(dest_ip_addr),
	};

	memcpy(datagram, data, size);
	if (sendto(sock_fd, datagram, datagram_size, 0,
				(struct sockaddr *)&sin,
				sizeof(struct sockaddr_in)) == -1) {
		perror("sendto() failed");
		return -1;
	}

	return 0;
}

int receive_packet(int sock_fd, void * dest, unsigned size, char * src_ip) {
	char receive_buf[DATAGRAM_SIZE];
	struct iphdr * iph = (struct iphdr *)receive_buf;
	int len = 0;

	void * data = receive_buf + sizeof(struct iphdr);
	int data_len = 0;
	char * buf_src_ip;

	len = recv(sock_fd, receive_buf, DATAGRAM_SIZE, 0);
	if(len < 0) {
		perror("recv() failed");
		return -1;
	}

	if(src_ip) {
		buf_src_ip = inet_ntoa(*((struct in_addr *)&iph->saddr));
		strcpy(src_ip, buf_src_ip);
	}

	data_len = len - sizeof(struct iphdr);
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
