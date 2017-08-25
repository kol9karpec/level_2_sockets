#include "./lib.h"

void die(const char * str, int _errno) {
	printf("%s: %s\n",str,strerror(_errno));
	exit(1);
}

char * printf_data_hex(char * buf,
		const unsigned int bufsize,
		const void * data,
		const unsigned int size) {
	char local_buffer[DEF_BUFSIZE] = {0};
	memset(buf,0,bufsize);

	unsigned int i = 0;
	unsigned int bufsize_left = bufsize;
	unsigned int delta = 0;
	unsigned char * _data = (unsigned char *)(data);

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
		printf_data_hex(print_buffer,
						BIG_BUFSIZE,
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

void bpf_attach(int _socket) {
	struct sock_fprog bpf_code[] = {
		BPF_STMT(BPF_LD+BPF_H+BPF_ABS,12),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K,0x86dd,2,7),
		BPF_STMT(BPF_LD+BPF_B+BPF_ABS,20),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K,0x6,10,4),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K,0x2c,5,11),
		BPF_STMT(BPF_LD+BPF_B+BPF_ABS,54),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K,0x6,10,11),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K,0x800,8,11),
		BPF_STMT(BPF_LD+BPF_B+BPF_ABS,23),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K,0x6,10,11),
		BPF_STMT(BPF_RET+BPF_K,262144),
		BPF_STMT(BPF_RET+BPF_K,0)
	};

	/*
	struct bpf_program _bpf_program = {0};

	_bpf_program.bf_len = sizeof(bpf_code) / sizeof(struct bpf_insn);
	_bpf_program.bf_insns = bpf_code;
	*/

	/*
	 * Not working, there is no BIOCSETF in linux
	 */

	/*if(ioctl(_socket, BIOCSETF, &_bpf_program) < 0) {
		die("bpf_attach()",errno);
	}*/

	struct sock_fprog filter = {0};

	if(setsockopt(_socket, SOL_SOCKET, SO_ATTACH_BPF, &prog_fd, sizeof(prog_fd)) != 0) {
		die("setsockopt()",errno);
	}
}
