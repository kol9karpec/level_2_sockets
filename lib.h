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
#include <arpa/inet.h>
#include <net/bpf.h>
#include <fcntl.h>
#include <linux/filter.h>

#include <sys/types.h>
#include <sys/time.h>

#define BIG_BUFSIZE 10000
#define DEF_BUFSIZE 256
#define DEF_PKTBUFSIZE 2000
#define BYTES_IN_ROW 16

void die(const char * str, int _errno);
void sigint_handler(int _socket);
void capture_packet(int _socket);

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

void to_promiscuous(const char * _if_name, const int _socket);

void bpf_attach(int _socket);
#endif //_LIB_H_
