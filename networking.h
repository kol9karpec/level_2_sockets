#ifndef _NETWORKING_H_
#define _NETWORKING_H_

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
#include "gamelib.h"
#include "common.h"

#define BIG_BUFSIZE 10000
#define DEF_BUFSIZE 256
#define DEF_PKTBUFSIZE 2000
#define BYTES_IN_ROW 16
#define DATAGRAM_SIZE 4096
#define IPPROTO_TTT 145

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

/*
 * Waiting game connection request
 *
 * @return -1 if failed, socket file descriptor if succeeded
 */
int run_wait();

/*
 * Requests game connection
 *
 * @return -1 if failed, socket file desctiptor if succeeded
 */
int run_connect(char * ip_addr);

/*
 * Sends a ttt packet
 *
 * param[in] sock_fs socket file descriptor created with params
 *		(AF_INET, SOCK_RAW, IPPROTO_TTT)
 * param[in] dest_ip_addr pointer to a string with ip addr
 * param[in] data pointer to memory with the ttt packet with header
 * param[in] size of memory, pointed to by data
 *
 * @return 0 if succeeded, -1 if failed, -2 if connection denied
 */
int send_packet(int sock_fd, char * dest_ip_addr, void * data, int size);

/*
 * Receives a ttt packet
 *
 * NOTE: Function writes to dest a received packet without ip header
 *
 * param[in] sock_fs socket file descriptor created with params
 *		(AF_INET, SOCK_RAW, IPPROTO_TTT)
 * param[in] dest pointer to allocated memory where to put received packet
 * param[in] size size of allocated memory, pointed to by dest
 * param[in] src_ip pointer to memory with size>=16 to put ip address of the
 *		received packet if you want to save the addr, NULL if not
 *
 * @return length of packet, put to dest if succeeded, -1 if failed
 */
int receive_packet(int sock_fd, void * dest, unsigned size, char * src_ip);
int open_socket();

#endif /* _LIB_H_ */
