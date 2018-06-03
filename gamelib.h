#ifndef _GAMELIB_H_
#define _GAMELIB_H_

#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "networking.h"

extern char psym[2];

typedef enum {
	CONNECTION,
	GAME
} packet_type_t;

typedef enum {
	MOVE,
	ACK
} game_packet_type_t;

typedef enum {
	REQ,
	RESP
} connection_packet_type_t;
typedef enum {
	ACCEPT,
	DENY
} connection_status_code_t;

typedef enum {
	X,
	O
} player_type_t;

typedef enum {
	WAITING,
	MOVING
} player_status_t;

typedef struct {
	packet_type_t type;
} packet_header_t;

typedef struct {
	game_packet_type_t type;
	int number;
	char c;
} game_packet_t;

typedef struct {
	connection_packet_type_t type;
	connection_status_code_t code;
} connection_packet_t;

typedef struct {
	unsigned size;
	char * matr;
} field_t;

void field_draw(field_t * field, FILE * out_dest);
void field_init(field_t * field);
void fill_numbers(field_t * field);
void field_free(field_t * field);

/*
 * param[in] number - number of cell 1 to 9
 */
int char_set(field_t * field, int number, char c);
char char_get(field_t * field, int number);

extern field_t play_field;

void switch_status(player_status_t * status);

int start_game(player_type_t type);
int move(int num, player_type_t type, field_t * field);
int wait_move(player_type_t type, field_t * field);

/*
 * Returns 0 - if game is not finished yet
 *			1 - if player X win
 *			2 - if player O win
 *			3 - if draw
 */
int field_status_check(field_t * field);
#endif
