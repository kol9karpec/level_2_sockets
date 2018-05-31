#ifndef _LIB_H_
#define _LIB_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <linux/if_packet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define BIG_BUFSIZE 10000
#define DEF_BUFSIZE 256
#define DEF_PKTBUFSIZE 2000
#define BYTES_IN_ROW 16
#define DATAGRAM_SIZE 4096

struct pseudo_header
{
	u_int32_t source_address;
	u_int32_t dest_address;
	u_int8_t placeholder;
	u_int8_t protocol;
	u_int16_t tcp_length;
};

unsigned short csum(unsigned short *ptr, int nbytes);

void die(const char * str, int _errno);
void sigint_handler(int _socket);

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

int run_wait(char * port);
int run_connect(char * ip_addr, char * port);

int send_packet(int sock_fd, char * ip_addr, char * port,
		void * data, int size);
int receive_packet(int sock_fd, short port, void * dest, unsigned size);
int open_socket();

#endif /* _LIB_H_ */
