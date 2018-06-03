#include "gamelib.h"

char psym[2] = {'X', 'O'};

void field_draw(field_t * field, FILE * out_dest) {
	size_t i = 0;
	size_t j = 0;
	unsigned size = field->size;
	char * matr = field->matr;

	for(; i < size; i++) {
		for(j = 0; j < size; j++) {
			fprintf(out_dest, "%c\t", matr[i*size + j]);
		}
		fprintf(out_dest, "\n");
	}
}

void field_init(field_t * field) {
	unsigned size = field->size;
	char ** matr = &field->matr;

	*matr = malloc(sizeof(char) * size * size);
}

void fill_numbers(field_t * field) {
	size_t i = 0;
	size_t j = 0;
	unsigned size = field->size;
	char * matr = field->matr;

	for(; i < size; i++) {
		for(j = 0; j < size; j++) {
			matr[i*size + j] = i*size + j + 1 + '0';
		}
	}
}

void field_free(field_t * field) {
	free(field->matr);
}

int char_set(field_t * field, int number, char c) {
	char * matr = field->matr;

	unsigned size = field->size;
	if(number > size*size) {
		return -1;
	}

		matr[number-1] = c;

	return 0;
}

char char_get(field_t * field, int number) {
	return field->matr[number-1];
}


int start_game(player_type_t type) {
	player_status_t status;
	int game_status = 0;


	field_t play_field = {
		.size = 3,
		.matr = NULL
	};
	field_init(&play_field);
	fill_numbers(&play_field);
	field_draw(&play_field, stdout);

	if(type == X) {
		status = MOVING;
	} else {
		status = WAITING;
	}
	LOG("Game started as %s", type == X ? "X" : "Y");

	while (!game_status) {
		if (status == MOVING) {
			int my_move;
			printf("Enter your move: ");
			scanf("%d", &my_move);
			move(my_move, type, &play_field);
		} else {
			wait_move(type, &play_field);
		}

		game_status = field_status_check(&play_field);
		switch_status(&status);
	}

	switch(game_status) {
		case 1:
			if (type == X)
				printf("You win!\n");
			else
				printf("You lose!\n");
			break;
		case 2:
			if (type == O)
				printf("You win!\n");
			else
				printf("You lose!\n");
			break;
		case 3:
			printf("Draw!\n");
	}

	return 0;
}

void switch_status(player_status_t * status) {
	*status = (*status == WAITING) ? MOVING : WAITING;
	LOG("Status switched to %s", *status == WAITING ? "WAITING" : "MOVING");
}

int field_status_check(field_t * field) {
	unsigned size = field->size;
	char * matr = field->matr;
	int sym, i;

	for (sym = 0; sym < 2; sym++) {
		if ((matr[0] == psym[sym] && matr[1] == psym[sym] && matr[2] == psym[sym]) ||
				(matr[3] == psym[sym] && matr[4] == psym[sym] && matr[5] == psym[sym]) ||
				(matr[6] == psym[sym] && matr[7] == psym[sym] && matr[8] == psym[sym]) ||
				(matr[0] == psym[sym] && matr[3] == psym[sym] && matr[6] == psym[sym]) ||
				(matr[1] == psym[sym] && matr[4] == psym[sym] && matr[7] == psym[sym]) ||
				(matr[2] == psym[sym] && matr[5] == psym[sym] && matr[8] == psym[sym]) ||
				(matr[0] == psym[sym] && matr[4] == psym[sym] && matr[8] == psym[sym]) ||
				(matr[2] == psym[sym] && matr[4] == psym[sym] && matr[6] == psym[sym])) {
			return sym+1;
		}
	}

	int flag = 0;
	for (i=0; i<size; i++) {
		if(matr[i] != psym[sym] && matr[i] != psym[sym]) {
			flag = 1;
			break;
		}
	}

	if(flag)
		return 0;
	else
		return 3;
}

int move(int num, player_type_t type, field_t * field) {
	char buf[DATAGRAM_SIZE] = {0};
	int bufsize = 0;
	char symbol = (type == X) ? 'X' : 'O';

	char receive_buf[DATAGRAM_SIZE] = {0};
	int receive_buf_len = 0;
	char src_ip_addr[16] = {0};
	int _connection_socket_fd_1 = open_socket();

	if (char_get(field, num) == 'X' || char_get(field, num) == 'O') {
		LOG("Field is not empty!");
		return -1;
	} else {
		char_set(field, num, symbol);
		field_draw(field, stdout);
	}

	packet_header_t header = {
		.type = GAME
	};

	game_packet_t packet = {
		.type = MOVE,
		.number = num,
		.c = symbol
	};

	memcpy(buf, &header, sizeof(header));
	bufsize += sizeof(header);
	memcpy(buf + bufsize, &packet, sizeof(packet));
	bufsize += sizeof(packet);

	if (send_packet(_connection_socket_fd_1, _connection_ip_addr, buf, bufsize)) {
		perror("send_packet() error");
		return -1;
	}
	LOG("Move sent, number = %d, symbol = %c", num, symbol);

	packet_header_t * rc_header = (packet_header_t *)receive_buf;
	game_packet_t * rc_packet = (game_packet_t *)(receive_buf +
			sizeof(packet_header_t));

	do {
		receive_buf_len = receive_packet(_connection_socket_fd_1, receive_buf, DATAGRAM_SIZE,
				src_ip_addr);
		if (receive_buf_len < 0) {
			perror("receive_packet() error");
			return -1;
		}
	} while(rc_header->type != GAME || rc_packet->type != ACK);
	LOG("ACK received!");

	return 0;
}

int wait_move(player_type_t type, field_t * field) {
	char buf[DATAGRAM_SIZE] = {0};
	int bufsize = 0;

	char receive_buf[DATAGRAM_SIZE] = {0};
	int receive_buf_len = 0;
	char src_ip_addr[16] = {0};
	int _connection_socket_fd_1 = open_socket();

	packet_header_t * rc_header = (packet_header_t *)receive_buf;
	game_packet_t * rc_packet = (game_packet_t *)(receive_buf +
			sizeof(packet_header_t));

	do {
		receive_buf_len = receive_packet(_connection_socket_fd_1, receive_buf, DATAGRAM_SIZE,
				src_ip_addr);
		if (receive_buf_len < 0) {
			perror("receive_packet() error");
			return -1;
		}
	} while(rc_header->type != GAME || rc_packet->type != MOVE);
	LOG("MOVE received, num = %d, c = %c", rc_packet->number, rc_packet->c);

	char_set(field, rc_packet->number, rc_packet->c);
	printf("MOVE received, num = %d, c = %c\n", rc_packet->number, rc_packet->c);
	field_draw(field, stdout);

	packet_header_t header = {
		.type = GAME
	};

	game_packet_t packet = {
		.type = ACK,
	};

	memcpy(buf, &header, sizeof(header));
	bufsize += sizeof(header);
	memcpy(buf + bufsize, &packet, sizeof(packet));
	bufsize += sizeof(packet);

	if (send_packet(_connection_socket_fd_1, _connection_ip_addr, buf, bufsize)) {
		perror("send_packet() error");
		return -1;
	}
	LOG("ACK sent");

	return 0;
}



