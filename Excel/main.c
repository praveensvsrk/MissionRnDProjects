#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include <stdlib.h>
#define ROWS 13
#define COLS 13
typedef struct {
	char expression[20];
}node;

typedef struct {
	node *nodes[2];
	char op;
}operation;

node sheet[ROWS][COLS];
int is_accessed[ROWS][COLS];

char operators[] = { '+', '-', '*', '/' };

node* create_node(char* name) {
	node* new_node = (node*)malloc(sizeof(node));
	strcpy(new_node->expression, name);
}

void display() {
	for (int i = 0; i < ROWS; i++) {
		printf("| ");
		for (int j = 0; j < COLS; j++) {
			if (strcmp(sheet[i][j].expression) == "")
				printf("     |");
			else
				printf("%4d |", eval(&sheet[i][j]));
		}
		printf("\n");
	}
}

int strlen(char *str) {
	int i = 0;
	while (str[i] != '\0') {
		i++;
	}
	return i;
}

int is_value(char* str) {
	int len = strlen(str);
	int i = 0;
	if (str[0] == '-')
		i++;
	for (; i < len; i++) {
		if (!(str[i] >= '0' && str[i] <= '9')) {
			return 0;
		}
	}
	return 1;
}

int parse_value(char* str) {
	int result = 0;
	int len = strlen(str);
	for (int i = 0; i < len; i++) {
		result *= 10;
		result += str[i] - '0';
	}
	return result;
}

int strpos(char* str, char ch) {
	int len = strlen(str);
	for (int i = 0; i < len; i++) {
		if (ch == str[i])
			return i;
	}
	return -1;

}

int get_op(char* str, char *op, char *result) {
	int len = strlen(str);
	for (int i = 0; i < len; i++) {
		if (op[0] == str[i]){
			*result = op[0];
			return i;
		}
		if (op[1] == str[i]) {
			*result = op[1];
			return i;
		}
		if (op[2] == str[i]) {
			*result = op[2];
			return i;
		}
		if (op[3] == str[i]) {
			*result = op[3];
			return i;
		}

	}
	return -1;

}

char* get_cell(int row, int col) {
	char* cell = (char*)malloc(sizeof(char) * 4);
	cell[0] = 'A' + col;
	
	if (row / 10 != 0) {
		cell[1] = '0' + row / 10;
		cell[2] = '0' + row % 10;
		cell[3] = '\0';
	}
	else {
		cell[1] = '0' + row % 10;
		cell[2] = '\0';
	}
		
	return cell;
}

int *get_dimensions(char *str, int start, int end) {
	
	int *dimensions = (int*)calloc(2, sizeof(int));
	dimensions[1] = str[start] - 65;
	char a[5];
	int i = 0;
	start++;
	while (start <= end) {
		a[i] = str[start];
		i++;
		start++;
	}
	a[i] = '\0';
	dimensions[0] = atoi(a) - 1;
	return dimensions;
}

operation* parse_expression(char* expression) {
	operation *oper = (operation*)malloc(sizeof(operation));
	char op;
	int pos = get_op(expression, operators, &op);
	if (pos == -1) {
		printf("Invalid expression\n");
	}
	else {
		oper->op = op;
		int i;
		int *dimensions1 = NULL;
		int *dimensions2 = NULL;
		char *temp = (char*)malloc((pos) * sizeof(char));
		for (i = 0; i < pos; i++) {
			temp[i] = expression[i];
		}
		temp[i] = '\0';
		if (is_value(temp)) {
			oper->nodes[0] = (node*)malloc(sizeof(node));
			strcpy(oper->nodes[0]->expression, temp);
		}
		else {
			if (get_op(temp, operators, &op) != -1) {
				oper->nodes[0] = create_node(temp);
			}
			else {
				dimensions1 = get_dimensions(expression, 0, pos - 1);
				oper->nodes[0] = &(sheet[dimensions1[0]][dimensions1[1]]);
			}
		}
		for (i = pos+1; i < strlen(expression); i++) {
			temp[i - pos - 1] = expression[i];
		}
		temp[i - (pos + 1)] = '\0';
		if (is_value(temp)) {
			oper->nodes[1] = (node*)malloc(sizeof(node));
			strcpy(oper->nodes[1]->expression, temp);
		}
		else {
			if (get_op(temp, operators, &op) != -1) {
				oper->nodes[1] = create_node(temp);
			}
			else {
				dimensions2 = get_dimensions(expression, pos + 1, strlen(expression) - 1);
				oper->nodes[1] = &(sheet[dimensions2[0]][dimensions2[1]]);
			}
			
		}
	}
	return oper;
}


int eval(node *n) {
	if (is_value(n->expression))
		return parse_value(n->expression);
	else {
		
		operation *oper = parse_expression(n->expression);
		switch (oper->op) {
		case '+':
			return eval(oper->nodes[0]) + eval(oper->nodes[1]);
			break;
		case '-':
			return eval(oper->nodes[0]) - eval(oper->nodes[1]);
			break;
		case '*':
			return eval(oper->nodes[0]) * eval(oper->nodes[1]);
			break;
		case '/':
			return eval(oper->nodes[0]) / eval(oper->nodes[1]);
			break;
		default:
			break;
		}
	}
}

void load_from_file(char* filename) {
	FILE *fp = fopen(filename, "r");
	char* expr = (char*)malloc(sizeof(char) * 100);
	int pos, *dimensions;
	while (fscanf(fp, "%s\n", expr) != -1) {
		pos = strpos(expr, '=');
		dimensions = get_dimensions(expr, 0, pos - 1);
		strcpy(sheet[dimensions[0]][dimensions[1]].expression, expr + pos + 1);
		is_accessed[dimensions[0]][dimensions[1]] = 1;
	}
	fclose(fp);
}

void save_to_file(char* filename) {
	FILE *fp = fopen(filename, "w");
	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLS; j++) {
			if (is_accessed[i][j]) {
				fprintf(fp, "%s=%s\n", get_cell(i + 1, j), sheet[i][j].expression);
			}
		}
	}
	fclose(fp);
}


int main() {
	memset(is_accessed, 0, sizeof(is_accessed[0][0]) * ROWS * COLS);
	char* expr = (char*)malloc(sizeof(char) * 100);
	while (1) {
		display();
		printf("\nEnter 'quit' to stop\n\n> ");
		gets(expr);
		fflush(stdin);
		if (strcmp(expr, "quit") == 0)
			break;
		else if (strcmp(expr, "save") == 0) {
			save_to_file("excel.txt");
			continue;
		}
		else if (strcmp(expr, "load") == 0) {
			load_from_file("excel.txt");
			continue;
		}
		int pos = strpos(expr, '=');
		int *dimensions = get_dimensions(expr, 0, pos - 1);
		strcpy(sheet[dimensions[0]][dimensions[1]].expression, expr + pos + 1);
		is_accessed[dimensions[0]][dimensions[1]] = 1;
	}
	getchar();
	return 0;
}
