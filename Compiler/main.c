#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>

typedef enum {declaration, execution} state;

state current_state = declaration;

int ins_count = 0;
typedef struct {
	int in_no;
	int op_code;
	int param[4];
	int param_count;
}lang_table_entry;

typedef struct {
	lang_table_entry **entries;
	int len;
	int max_len;
}lang_table;

typedef struct {
	char* name;
	int address;
	int size;
}symbol_table_entry;

typedef struct {
	symbol_table_entry **entries;
	int len;
	int max_len;
}symbol_table;

typedef struct {
	char *name;
	int address;
}block_table_entry;

typedef struct {
	block_table_entry **entries;
	int len;
	int max_len;
}block_table;

typedef struct {
	int array[100];
	int top;
}stack;

typedef struct {
	int *blocks;
	int len;
	int max_len;
}memory;


stack *stk;
memory *mem_blocks;
lang_table *l_table;
symbol_table *sym_table;
block_table *blk_table;

int strlen(char* str) {
	int i = 0;
	while (str[i] != '\0')
		i++;
	return i;
}

void init_l_table() {
	l_table = (lang_table*)malloc(sizeof(lang_table));
	l_table->entries = (lang_table_entry**)malloc(sizeof(lang_table_entry*) * 20);
	l_table->max_len = 20;
	l_table->len = 0;
}

void init_sym_table() {
	sym_table = (symbol_table*)malloc(sizeof(symbol_table));
	sym_table->entries = (symbol_table_entry**)malloc(sizeof(symbol_table_entry*) * 20);
	sym_table->max_len = 20;
	sym_table->len = 0;
}

void int_blk_table() {
	blk_table = (block_table*)malloc(sizeof(block_table));
	blk_table->entries = (block_table_entry**)malloc(sizeof(block_table_entry*) * 20);
	blk_table->max_len = 20;
	blk_table->len = 0;
}

void init_stack() {
	stk = (stack*)malloc(sizeof(stack));
	stk->top = -1;
}

void init_memory() {
	mem_blocks = (memory*)malloc(sizeof(memory));
	mem_blocks->blocks = (int*)malloc(sizeof(int) * 20);
	mem_blocks->max_len = 20;
	mem_blocks->len = 0;
}

void add_l_table_entry(lang_table_entry *e) {
	if (l_table->len == l_table->max_len - 1) {
		l_table->max_len *= 2;
		l_table->entries = (lang_table_entry**)realloc(l_table->entries, l_table->max_len * sizeof(lang_table_entry*));
	}
	l_table->entries[l_table->len] = e;
	l_table->len++;
}

void add_blk_table_entry(block_table_entry *e) {
	if (blk_table->len == blk_table->max_len - 1) {
		blk_table->max_len *= 2;
		blk_table->entries = (block_table_entry**)realloc(blk_table->entries, blk_table->max_len * sizeof(block_table_entry*));
	}
	blk_table->entries[blk_table->len] = e;
	blk_table->len++;
}

void add_sym_table_entry(symbol_table_entry *e) {
	if (sym_table->len == sym_table->max_len - 1) {
		sym_table->max_len *= 2;
		sym_table->entries = (symbol_table_entry**)realloc(sym_table->entries, sym_table->max_len * sizeof(symbol_table_entry*));
	}
	sym_table->entries[sym_table->len] = e;
	sym_table->len++;
}

void add_to_memory(int value) {
	if (mem_blocks->len == mem_blocks->max_len - 1) {
		mem_blocks->max_len *= 2;
		mem_blocks->blocks = (int*)realloc(mem_blocks->blocks, mem_blocks->max_len);
	}
	mem_blocks->blocks[mem_blocks->len] = value;
	mem_blocks++;
}

int fetch_from_memory(char* name) {
	for (int i = 0; i < sym_table->len; i++) {
		if (strcmp(name, sym_table->entries[i]->name) == 0)
			return mem_blocks->blocks[sym_table->entries[i]->address];
	}
	return -1;
}


lang_table_entry* create_l_table_entry(int in_no, int op_code, int *param, int param_count) {
	lang_table_entry* new_entry = (lang_table_entry*)malloc(sizeof(lang_table_entry));
	new_entry->in_no = in_no;
	new_entry->op_code = op_code;
	new_entry->param_count = param_count;
	for (int i = 0; i < param_count; i++) {
		new_entry->param[i] = param[i];
	}
	return new_entry;
}

block_table_entry* create_blk_table_entry(char *name, int address) {
	block_table_entry* new_entry = (block_table_entry*)malloc(sizeof(block_table_entry));
	new_entry->name = (char*)malloc((strlen(name) + 1) * sizeof(char));
	strcpy(new_entry->name, name);
	new_entry->address = address;
	return new_entry;
}

symbol_table_entry* create_sym_table_entry(char *name, int address, int size) {
	symbol_table_entry* new_entry = (symbol_table_entry*)malloc(sizeof(symbol_table_entry));
	new_entry->name = (char*)malloc((strlen(name) + 1) * sizeof(char));
	strcpy(new_entry->name, name);
	new_entry->address = address;
	new_entry->size = size;
	return new_entry;
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

int get_dec_code(char* str) {
	if (strcmp(str, "DATA") == 0)
		return 1;
	if (strcmp(str, "CONST") == 0)
		return 2;
}

int get_register_code(char *reg) {
	switch (reg[0]) {
	case 'A':
		return 0;
	case 'B':
		return 1;
	case 'C':
		return 2;
	case 'D':
		return 3;
	case 'E':
		return 4;
	case 'F':
		return 5;
	case 'G':
		return 6;
	case 'H':
		return 7;
	default:
		return -1;
	}
}

int get_op_code(char *str) {
	int len = strlen(str);
	if (strcmp(str, "MOV") == 0)
		return 1;
	if (strcmp(str, "ADD") == 0)
		return 3;
	if (strcmp(str, "SUB") == 0)
		return 4;
	if (strcmp(str, "MUL") == 0)
		return 5;
	if (strcmp(str, "JUMP") == 0 || strcmp(str, "ELSE") == 0)
		return 6;
	if (strcmp(str, "IF") == 0)
		return 7;
	if (strcmp(str, "EQ") == 0)
		return 8;
	if (strcmp(str, "LT") == 0)
		return 9;
	if (strcmp(str, "GT") == 0)
		return 10;
	if (strcmp(str, "LTEQ") == 0)
		return 11;
	if (strcmp(str, "GTEQ") == 0)
		return 12;
	if (strcmp(str, "PRINT") == 0)
		return 13;
	if (strcmp(str, "READ") == 0)
		return 14;
	if (strcmp(str, "ENDIF") == 0)
		return 15;
}

int get_block_address(char *name) {
	int i = 0;
	int len = strlen(name) + 2;
	char *new_name = (char*)malloc(sizeof(char)*len);
	strcpy(new_name, name);
	len = strlen(new_name);
	new_name[len] = ':';
	new_name[len + 1] = '\0';
	for (i = 0; i < blk_table->len; i++) {
		if (strcmp(new_name, blk_table->entries[i]->name) == 0)
			return blk_table->entries[i]->address;
	}
	return -1;
}

int get_symbol_address(char* name) {
	int i = 0;
	int array_pos = strpos(name, '[');
	int value = 0;
	if (array_pos != -1) {
		value = parse_value(name + array_pos + 1);
	}
	for (i = 0; i < sym_table->len; i++) {
		if (strcmp(name, sym_table->entries[i]->name) == 0)
			return sym_table->entries[i]->address + value;
	}
	return -1;
}

int strpos(char* str, char ch) {
	int len = strlen(str);
	for (int i = 0; i < len; i++) {
		if (ch == str[i])
			return i;
	}
	return -1;
}

int parse_value(char *str) {
	int len = strlen(str);
	int result = 0;
	for (int i = 0; str[i] != ']' && str[i] != '\0'; i++) {
		result *= 10;
		result += str[i] - '0';
	}
	str[-1] = '\0';
	return result;
}

void execute_instruction(char* ins) {
	int len, inst;
	if (strcmp(ins, "START:") == 0) {
		current_state = execution;
		return;
	}
	if (strcmp(ins, "END") == 0) {
		return;
	}
	char **strings = split(ins, ' ', &len);
	symbol_table_entry* entry = NULL;
	if (current_state == declaration) {
		int dec_code = get_dec_code(strings[0]);
		int size = 1;
		int array_pos = strpos(strings[1], '[');
		if (array_pos != -1) {
			size += parse_value(strings[1] + array_pos + 1);
		}
		switch (dec_code) {
		case 1:
			if (sym_table->len == 0)
				entry = create_sym_table_entry(strings[1], 0, size);
			else {
				symbol_table_entry *last_entry = sym_table->entries[sym_table->len - 1];
				entry = create_sym_table_entry(
					strings[1], last_entry->address + last_entry->size, size);
			}
			break;
		case 2:
			if (sym_table->len == 0)
				entry = create_sym_table_entry(strings[1], 0, 0);
			else {
				symbol_table_entry *last_entry = sym_table->entries[sym_table->len - 1];
				entry = create_sym_table_entry(
					strings[1], last_entry->address + last_entry->size, 0);
			}
			break;
		}
		add_sym_table_entry(entry);
	}
	else {
		
		if (len == 1 && ins[strlen(ins) - 1] == ':') { //label
			block_table_entry* blk_table_entry = create_blk_table_entry(strings[0], ins_count + 1);
			add_blk_table_entry(blk_table_entry);
		}
		else {
			ins_count++;
			int op_code = get_op_code(strings[0]);
			lang_table_entry* l_table_entry;
			int param[4];
			int param_count = 0;
			switch (op_code) {
			case 1:
				strings = split(ins + strlen(strings[0]) + 1, ',', &len);
				if (strings[1][strlen(strings[1])-1] == 'X') {
					param[0] = get_symbol_address(strings[0]);
					param[1] = get_register_code(strings[1]);
				}
				else {
					param[0] = get_register_code(strings[0]);
					param[1] = get_symbol_address(strings[1]);
					op_code = 2;
				}
				param_count = 2;
				break;
			case 3:
			case 4:
			case 5:
				strings = split(ins + strlen(strings[0]) + 1, ',', &len);
				param[0] = get_register_code(strings[0]);
				param[1] = get_register_code(strings[1]);
				param[2] = get_register_code(strings[2]);
				param_count = 3;
				break;
			case 6:
				if(strings[0][0] == 'J')
					param[0] = get_block_address(strings[1]);
				else {
					inst = stk->array[stk->top--];
					l_table->entries[inst - 1]->param[3] = ins_count + 1;
					stk->array[++stk->top] = ins_count;
				}
					
				param_count = 1;
				break;
			case 7:
				strings = split(ins + strlen(strings[0]) + 1, ' ', &len);
				param[0] = get_register_code(strings[0]);
				param[1] = get_op_code(strings[1]);
				param[2] = get_register_code(strings[2]);
				param[3] = -1;
				param_count = 4;
				stk->array[++stk->top] = ins_count;
				break;
			case 13:
			case 14:
				param[0] = get_register_code(strings[1]);
				param_count = 1;
				break;
			case 15:
				inst = stk->array[stk->top--];
				ins_count--;
				if(l_table->entries[inst]->op_code == get_op_code("IF"))
					l_table->entries[inst]->param[3] = ins_count;
				else
					l_table->entries[inst - 1]->param[0] = ins_count;
				return;
			default:
				break;
			}
			l_table_entry = create_l_table_entry(ins_count, op_code, param, param_count);
			add_l_table_entry(l_table_entry);
		}
	}
		
	
}

void display_l_table() {
	for (int i = 0; i < l_table->len; i++) {
		printf("%d, %d", l_table->entries[i]->in_no, l_table->entries[i]->op_code);
		for (int j = 0; j < l_table->entries[i]->param_count; j++) {
			printf(", %d", l_table->entries[i]->param[j]);
		}
		printf("\n");
	}
}

void display_sym_table() {
	for (int i = 0; i < sym_table->len; i++) {
		printf("%s, %d, %d", sym_table->entries[i]->name, sym_table->entries[i]->address, sym_table->entries[i]->size);
		printf("\n");
	}
}

void display_blk_table() {
	for (int i = 0; i < blk_table->len; i++) {
		printf("%s, %d", blk_table->entries[i]->name, blk_table->entries[i]->address);
		printf("\n");
	}
}


char *fetch_instruction(FILE *fp) {
	char *instruction = (char*)malloc(sizeof(char) * 100);
	char *retval = fgets(instruction, 100, fp);
	if (retval == NULL) {
		free(instruction);
		return NULL;
	}
	else {
		instruction[strlen(instruction) - 1] = '\0';
		return instruction;
	}
		
}

int main() {
	char *instruction = NULL;
	FILE *fp = fopen("sample.txt", "r");
	int i = 0;
	init_l_table();
	init_sym_table();
	int_blk_table();
	init_stack();
	while ((instruction = fetch_instruction(fp)) != NULL) {
		execute_instruction(instruction);
		i++;
	}
	display_sym_table();
	printf("\n");
	display_l_table();
	printf("\n");
	fclose(fp);
	getchar();
	getchar();

	return 0;

}