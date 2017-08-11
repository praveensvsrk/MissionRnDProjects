#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int shrinks = 0;

typedef struct {
	int **array;
	int rows;
	int cols;
}matrix;


void display_matrix(matrix *m) {
	int j = 0;
	for (int i = shrinks; i < m->rows; i++) {
		for (int j = shrinks; j < m->cols; j++) {
			printf("%d ", m->array[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

int test_and_shrink(matrix *m) {
	int cols_count = 0;
	int rows_count = 0;
	int rows = m->rows;
	int cols = m->cols;
	int min = m->rows < m->cols ? m->rows : m->cols;
	int i;
	for (i = shrinks; i < cols; i++)
		cols_count += m->array[shrinks][i];
	for (i = shrinks; i < rows; i++)
		rows_count += m->array[i][shrinks];

	if (rows_count + cols_count == rows + cols - 2*shrinks) {
		shrinks++;
		return 1;
	}
	return 0;
}

int count_consecutive(matrix *m, int row, int col) {
	int count = 0;
	int item = m->array[row][col];
	int j;
	for (j = col + 1; j < m->cols; j++) {
		if (m->array[row][j] == item)
			count++;
		else
			break;
	}
	return count;
}


int reduce(matrix *m, int row, int col) {
	int consecutive = count_consecutive(m, row, col);
	for (int i = row + shrinks; i >= 1 + shrinks; i--) {
		for (int j = col + shrinks; j <= col + consecutive; j++) {
			m->array[i][j] = m->array[i-1][j];
		}
	}
	for (int i = col; i <= col + consecutive; i++) {
		m->array[shrinks][i] = 1;
	}
}

matrix* init_matrix() {
	matrix *m = (matrix*)malloc(sizeof(matrix));
	printf("Enter rows: ");
	scanf("%d", &m->rows);
	printf("Enter cols: ");
	scanf("%d", &m->cols);
	m->array = (int**)malloc(sizeof(int*) * m->rows);

	printf("Enter the matrix:\n");
	for (int i = 0; i < m->rows; i++) {
		m->array[i] = (int *)malloc(sizeof(int) * m->cols);
		for (int j = 0; j < m->cols; j++) {
			scanf("%d", &(m->array[i][j]));
		}
	}
	return m;
}


int main() {
	int choice;
	int index;
	int  moves = 0;
	int row, col;
	
	matrix *m = init_matrix();
	int min = m->rows < m->cols ? m->rows : m->cols;
	while (shrinks < min) {
		display_matrix(m);
		printf("1. Reduce\n");
		printf("2. Flip\n");
		scanf("%d", &choice);
		printf("Enter row: ");
		scanf("%d", &row);
		printf("Enter col: ");
		scanf("%d", &col);
		switch (choice) {
		case 1:
			if (row < m->rows && col < m->cols) {
				reduce(m, row, col);
				moves++;
				while (test_and_shrink(m)) {
				}
			}
			break;
		case 2:
			row += shrinks;
			col += shrinks;
			moves++;
			m->array[row][col] = m->array[row][col] == 1 ? 0 : 1;
			while (test_and_shrink(m)) {
			}
			break;
		default:
			break;

		}
	}
	printf("Moves : %d\n", moves);
	getchar();
	getchar();
	return 0;
}