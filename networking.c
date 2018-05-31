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

int run_wait(char * port) {
	int s = open_socket();
	char buf[DATAGRAM_SIZE];
	while(1) {
		if(receive_packet(s, atoi(port), buf, DATAGRAM_SIZE)) {
			perror("receive_packet failed!");
			break;
		}
		printf("%s\n", buf);
	}
	return 0;
}

int run_connect(char * ip_addr, char * port) {
	int s = open_socket();
	char buf[DATAGRAM_SIZE] = {0};
	strcpy(buf, "Hello, world!");

	while(1) {
		send_packet(s, ip_addr, port, buf, strlen(buf));
		sleep(1);
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

int send_packet(int sock_fd, char * ip_addr, char * port,
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
	sin.sin_port = htons(atoi(port));
	sin.sin_addr.s_addr = inet_addr(ip_addr);

	tcph->source = htons(1234);
	tcph->dest = htons(atoi(port));
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

int receive_packet(int sock_fd, short port, void * dest, unsigned size) {
	struct tcphdr * tcph = NULL;
	char * data = NULL;
	char buf[DATAGRAM_SIZE];
	int len = 0;
	struct sockaddr_in server_addr = {0};

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(bind(sock_fd, (struct sockaddr *)(&server_addr),
				sizeof(server_addr))) {
		perror("bind() error");
		return -1;
	}

	do {
		len = recv(sock_fd, buf, DATAGRAM_SIZE, 0);
		tcph = (struct tcphdr *)(buf + sizeof(struct iphdr));
		data = (char *)(buf + sizeof(struct tcphdr) + sizeof(struct iphdr));
	} while(ntohs(tcph->dest) != port);

	memcpy(dest, data,
			(len - sizeof(struct iphdr) - sizeof(struct tcphdr)) % size);
	return 0;
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
