#ifndef _GAMELIB_H_
#define _GAMELIB_H_

#include <stdio.h>
#include <stdlib.h>

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

typedef struct {
	packet_type_t type;
} packet_header_t;

typedef struct {
	game_packet_type_t type;
	int row;
	int col;
	char c;
} game_packet_t;

typedef struct {
	connection_packet_type_t type;
	connection_status_code_t code;
} connection_packet_t;

typedef struct {
	unsigned size;
	char ** matr;
} field_t;

void field_draw(field_t * field, FILE * out_dest);
void field_init(field_t * field);
void fill_char(field_t * field, char symbol);
void field_free(field_t * field);
int char_set(field_t * field, unsigned i, unsigned j, char c);

extern field_t play_field;

void start_game(player_type_t type);
#endif
