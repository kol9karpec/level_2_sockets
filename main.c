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
#include <string.h>

#define BIG_BUFSIZE 10000
#define DEF_BUFSIZE 256
#define DEF_PKTBUFSIZE 2000
#define BYTES_IN_ROW 16

void die(const char * str, int _errno);

/* Print data, pointed by @data into the @buf as hex values (i.e. 00 FF 00 ...)
 * 16 bytes in a row, separated by a space.
 *
 * If @bufsize < @size - data will be truncated.
 * @return the same value as buf if succedes, NULL otherwise
 */
char * printf_data_hex(char * buf,
		const unsigned int bufsize,
		const void * data,
		const unsigned int size);

/* Prints packet, received by a level 2 socket if the following format:
 *
 *				Destination MAC: XX:XX:XX:XX:XX:XX
 *					 Source MAC: XX:XX:XX:XX:XX:XX
 *				------------------------------------------------
 *				XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX
 *				................................................
 *				XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX
 *				------------------------------------------------
 *
 * @buf - destination address
 * @bufsize - size of memory, allocated by a pointer @buf
 * @data - packet data
 * @size - size of data
 * @_sockaddr_ll - argument of @recvfrom() function.
 */
char * printf_packet(char * buf,
		const unsigned int bufsize,
		const void * data,
		const unsigned int size,
		const struct sockaddr_ll * _sockaddr_ll);

int main(const int argc, const char * argv[]) {
	char buffer[DEF_PKTBUFSIZE] = {0};
	char print_buffer[BIG_BUFSIZE] = {0};

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
		printf_data_hex(print_buffer,
						DEF_BUFSIZE,
						(void*)buffer,
						bytes_received);
		printf("%s\n",print_buffer);
		printf("--------------------------------------\n");
	}

	close(ethernet_socket);

	return 0;
}

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
	return NULL;
}
