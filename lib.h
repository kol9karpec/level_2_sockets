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
#include <linux/if_packet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <linux/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <net/bpf.h>

#define BIG_BUFSIZE 10000
#define DEF_BUFSIZE 256
#define DEF_PKTBUFSIZE ETHER_MAX_LEN
#define BYTES_IN_ROW 16

void die(const char * str, int _errno);
void sigint_handler(int _socket);
void capture_packet(int _socket, FILE * stream);

/* Print data, pointed by @data into the @stream as hex values (i.e. 00 FF 00 ...)
 * BYTES_IN_ROW bytes in a row, separated by a space.
 *
 */
void fprintf_data_hex(FILE * stream,
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
 * @stream - output stream
 * @data - packet data
 * @size - size of data
 * @_sockaddr_ll - argument of @recvfrom() function.
 */
void fprintf_packet(FILE * stream,
		const void * data,
		const unsigned int size,
		const struct sockaddr_ll * _sockaddr_ll);

void to_promiscuous(const char * _if_name, const int _socket);
#endif //_LIB_H_
