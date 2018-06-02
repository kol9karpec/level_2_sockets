#include "gamelib.h"

void field_draw(field_t * field, FILE * out_dest) {
	size_t i = 0;
	size_t j = 0;
	unsigned size = field->size;
	char *** matr = &field->matr;

	for(; i < size; i++) {
		for(j = 0; j < size; j++) {
			fprintf(out_dest, "%c\t", (*matr)[i][j]);
		}
		fprintf(out_dest, "\n");
	}
}

void field_init(field_t * field) {
	size_t i = 0;
	unsigned size = field->size;
	char *** matr = &field->matr;

	*matr = malloc(sizeof(char*) * size);
	for(; i < field->size; i++) {
		(*matr)[i] = malloc(sizeof(char) * size);
	}
}

void fill_char(field_t * field, char symbol) {
	size_t i = 0;
	size_t j = 0;
	unsigned size = field->size;
	char *** matr = &field->matr;

	for(; i < size; i++) {
		for(j = 0; j < size; j++) {
			(*matr)[i][j] = symbol;
		}
	}
}

void field_free(field_t * field) {
	size_t i = 0;
	unsigned size = field->size;
	char *** matr = &field->matr;

	for(; i < size; i++) {
		 free((*matr)[i]);
	}
	free(*matr);
}

int char_set(field_t * field, unsigned i, unsigned j, char c) {
	char *** matr = &field->matr;

	unsigned size = field->size;
	if(i >= size || j >= size) {
		return -1;
	}

	(*matr)[i][j] = c;

	return 0;
}

void start_game(player_type_t type) {


}
