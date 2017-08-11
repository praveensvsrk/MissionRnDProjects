#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include <stdlib.h>
#include <string.h>
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
	for (int j = 0; j < COLS; j++) {
		printf("______");
	}
	printf("__\n");
	for (int i = 0; i < ROWS; i++) {
		printf("| ");
		
		for (int j = 0; j < COLS; j++) {
			if (strcmp(sheet[i][j].expression, "") == 0)
				printf("     |");
			else
				printf("%4d |", eval(&sheet[i][j]));

		}
		printf("\n");
		for (int j = 0; j < COLS; j++) {
			printf("______");
		}
		printf("__\n");
		
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

char* get_range(int i, int j, int end_row, int end_col) {
	for (; i <= end_row; i++) {
		for (; j < end_col; j++) {

		}
	}
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

char** split(char* str, char delim, int *final_len) {
	char** strings = (char**)malloc(sizeof(char*) * 5);
	int len = strlen(str);
	int i = 0, j;
	int count = 0;
	while (str[i] == '\t' || str[i] == ' ')
		i++;
	int start = i;
	for (i = 0; i < len; i++) {
		if (str[i] == delim || str[i + 1] == '\0') {
			if (str[i + 1] == '\0')
				i++;
			strings[count] = (char*)malloc(sizeof(char)*(i + 1 - start));
			for (j = start; j < i && str[j] != '\n'; j++) {
				strings[count][j - start] = str[j];
			}
			strings[count][j - start] = '\0';
			start = i + 1;
			while (str[i + 1] == ' ') {
				i++;
				start++;
			}
			count++;
		}
	}
	*final_len = count;
	return strings;
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
	else if (strstr(n->expression, "SUM") || strstr(n->expression, "AVG")) {
		printf("cool");
	}
	else if (strstr(n->expression, "MAX")) {
		printf("cool");
	}
	else if (strstr(n->expression, "MIN")) {
		printf("cool");
	}
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
	return 0;
}


void execute_expr(char** strings) {
	int pos, *dimensions;
	if (strcmp(strings[0], "SET") == 0 || strcmp(strings[0], "SETF") == 0) {
		dimensions = get_dimensions(strings[1], 0, strlen(strings[1]) - 1);
		strcpy(sheet[dimensions[0]][dimensions[1]].expression, strings[2]);
		is_accessed[dimensions[0]][dimensions[1]] = 1;
	}

}

void evaluate_expression(char *filename) {
	FILE *fp = fopen(filename, "r");
	int len;
	int pos, *dimensions;
	char **strings;
	char op;
	char* expr = (char*)malloc(sizeof(char) * 100);
	char *retval;
	while ((retval = fgets(expr, 100, fp)) != NULL) {
		expr[strlen(expr)-1] = '\0';
		strings = split(expr, ' ', &len);
		execute_expr(strings);
		if (strcmp(strings[0], "IF") == 0) {
			dimensions = get_dimensions(strings[1], 0, strlen(strings[1]) - 1);
			int cell_val = eval(&sheet[dimensions[0]][dimensions[1]]);
			int val = parse_value(strings[3]);

			if ((strcmp(strings[2], "EQ") == 0 && cell_val == val)
				|| (strcmp(strings[2], "GT") == 0 && cell_val > val)
				|| (strcmp(strings[2], "LT") == 0 && cell_val < val)
				|| (strcmp(strings[2], "LTEQ") == 0 && cell_val <= val)
				|| (strcmp(strings[2], "GTEQ") == 0 && cell_val >= val)) {

				while ((retval = fgets(expr, 100, fp)) != NULL) {
					expr[strlen(expr) - 1] = '\0';
					if (strcmp(expr, "ELSE") == 0 || strcmp(expr, "ENDIF") == 0)
						break;
					strings = split(expr, ' ', &len);
					execute_expr(strings);
				}

			}
			else {
				while ((retval = fgets(expr, 100, fp)) != NULL) {
					expr[strlen(expr) - 1] = '\0';
					if (strcmp(expr, "ELSE") == 0 || strcmp(expr, "ENDIF") == 0)
						break;
				}
				if (strcmp(expr, "ELSE") == 0) {
					while ((retval = fgets(expr, 100, fp)) != NULL) {
						expr[strlen(expr) - 1] = '\0';
						if (strcmp(expr, "ENDIF") == 0)
							break;
						strings = split(expr, ' ', &len);
						execute_expr(strings);
					}
				}
				
			}	
			if (strcmp(expr, "ELSE") == 0) {
				while ((retval = fgets(expr, 100, fp)) != NULL) {
					expr[strlen(expr) - 1] = '\0';
					if (strcmp(expr, "ENDIF") == 0)
						break;
				}
			}
		}
	}
	fclose(fp);
}

	

int main() {
	memset(is_accessed, 0, sizeof(is_accessed[0][0]) * ROWS * COLS);
	char* expr = (char*)malloc(sizeof(char) * 100);
	evaluate_expression("excel.txt");
	display();
	getchar();
	return 0;
}
