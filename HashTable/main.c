#define BLOCK_SIZE 1048576
#define BLOCKS 100
#include<stdio.h>

typedef struct {
	int key_len;		
	int value_len;
	/* As a part of collision resolution, 
	 * offset stores the pointer to the next node with the same hash */
	int offset;
	char *key;
	void *value;
}node;

typedef struct {
	int *table;
	/* max_size and size determine when to perform a rehash(not yet implemented) */
	unsigned long size;
	unsigned long max_size;
}hash_table;

typedef struct {
	unsigned char bit_vector[8];
	/* offset points to the in-memory location where this slab resides */
	int offset;
	//prev pointers not yet implemented
	struct slab *prev;
	struct slab *next;
}slab;

typedef struct {
	int sub_block_count;
	slab *s;
}slab_meta;

typedef struct {
	slab_meta *slab_128;
	slab_meta *slab_256;
	slab_meta *slab_512;
	slab_meta *slab_1024;
}slab_list;


int allocated_list[] = { 0 };
unsigned char* memory;
slab_list *list;


unsigned char *allocate_memory() {
	return calloc(1, BLOCK_SIZE * BLOCKS);
}


int compute_node_size(node* n) {
	int size = 0;
	size = n->key_len + n->value_len + sizeof(n);
	if (size <= 128) {
		return 128;
	}
	else if (size <= 256) {
		return 256;
	}
	else if (size <= 512) {
		return 512;
	}
	else if (size <= 1024) {
		return 1024;
	}
	else {
		return -1;
	}
}


int get_free_block() {
	for (int i = 0; i < BLOCKS; i++) {
		if (allocated_list[i] == 0) {
			allocated_list[i] = 1;
			return i;
		}
	}
}


unsigned long hash(char *str)
{
	unsigned long hash = 5381;
	int c;

	while (c = *str++)
		hash = ((hash << 5) + hash) + c; 

	return hash;
}


void init_hash_table(hash_table *h) {
	h->max_size = 127;
	h->size = 0;
	h->table = (node*)malloc(sizeof(node)*h->max_size);

	for (unsigned long i = 0; i < h->max_size; i++) {
		h->table[i] = -1;
	}
}

node* create_node(char *key, char *value) {
	node* new_node = (node*)malloc(sizeof(node));
	new_node->key = (char*)malloc(sizeof(char)*strlen(key));

	strcpy(new_node->key, key);
	new_node->value = value;
	new_node->offset = -1;
	new_node->key_len = strlen(key);
	new_node->value_len = strlen(value);

	return new_node;
}


slab_meta* get_slab_meta_data(int size) {
	if (size == 128) {
		return list->slab_128;
	}
	else if (size == 256) {
		return list->slab_256;
	}
	else if (size == 512) {
		return list->slab_512;
	}
	else if (size == 1024) {
		return list->slab_1024;
	}
}


int get_free_sub_block_offset(node* n) {
	slab_meta *slab_meta_data;
	int size = compute_node_size(n);
	slab *s, *prev;
	slab * new_slab;

	slab_meta_data = get_slab_meta_data(size);

	if (slab_meta_data->s == NULL) {
		slab_meta_data->s = (slab*)malloc(sizeof(slab));
		s = slab_meta_data->s;
		s->offset = get_free_block() * 1024;
		s->next = NULL;
		memset(&(s->bit_vector), 0, sizeof(s->bit_vector));
		s->bit_vector[0] = 1;
		return s->offset;
	}
	else{
		s = slab_meta_data->s;
		prev = NULL;
		while (s != NULL) {
			for (int i = 0; i < slab_meta_data->sub_block_count; i++) {
				if (!s->bit_vector[i]) {
					s->bit_vector[i] = 1;
					s->next = NULL;
					return s->offset + i * 1024 / slab_meta_data->sub_block_count;
				}
			}
			prev = s;
			s = s->next;
		}
		new_slab = (slab*)malloc(sizeof(slab));
		prev->next = new_slab;
		memset(&(new_slab->bit_vector), 0, sizeof(new_slab->bit_vector));
		new_slab->bit_vector[0] = 1;
		new_slab->offset = get_free_block() * 1024;
		new_slab->prev = prev->offset;
		new_slab->next = NULL;
		return new_slab->offset;
	}
}


void put(hash_table *h, char *key, void *value) {
	
	node *node_ptr;
	int sub_block_index;
	int i, j;
	unsigned long hash_index = hash(key) % h->max_size;
	int hashed_location = h->table[hash_index];

	node* new_node = create_node(key, value);
	int slab_size = compute_node_size(new_node);

	if (hashed_location == -1) {
		sub_block_index = get_free_sub_block_offset(new_node);
		node_ptr = (node*)(memory + sub_block_index);
		h->table[hash_index] = sub_block_index;
	}
	else {
		while (1) {
			sub_block_index = hashed_location;
			node_ptr = (node*)(memory + sub_block_index);
			while (strlen(key) != node_ptr->key_len && node_ptr->offset != -1) {
				node_ptr = (node*)(memory + node_ptr->offset);
			}
			char* loc_key = (char*)malloc(sizeof(char)*node_ptr->key_len);
			unsigned char* pos = (unsigned char*)(node_ptr);

			for (i = 12, j = 0; i < 12 + node_ptr->key_len; i++, j++) {
				loc_key[j] = pos[i];
			}
			loc_key[i - 12] = '\0';
			if (strcmp(key, loc_key) == 0) {
				break;
			}
			else if(node_ptr->offset == -1){
				sub_block_index = get_free_sub_block_offset(new_node);
				node_ptr->offset = sub_block_index;
				node_ptr = (node*)(memory + sub_block_index);
				break;
			}
		}
	}
	node_ptr->key_len = new_node->key_len;
	node_ptr->value_len = new_node->value_len;
	node_ptr->offset = new_node->offset;

	/* Serialization of data to bytestream */
	unsigned char* serialized_data = (unsigned char*)(node_ptr);
	char* value_ptr = (char*)new_node->value;
	
	for (i = 12, j = 0; new_node->key[j]!='\0'; i++, j++) {
		serialized_data[i] = new_node->key[j];
	}
	
	for (j = 0; value_ptr[j] != '\0'; i++, j++) {
		serialized_data[i] = value_ptr[j];
	}
	/* Serialization ends */

	h->size++;
}

void display_hash_table(hash_table *h) {
	for (int i = 0; i < h->max_size; i++) {
		if (h->table[i] != -1)
			printf("%d\n", h->table[i]);
		else
			printf("empty\n");
	}
}


unsigned char* get(hash_table *h, char* key) {
	unsigned long hash_index = hash(key) % h->max_size;
	int hashed_location = (h->table[hash_index]);
	node* node_ptr;
	int len = strlen(key);
	int i, j = 0;

	if (hashed_location != -1) {
		while (1) {
			node_ptr = (node*)(memory + hashed_location);

			while (node_ptr && len != node_ptr->key_len) {
				node_ptr = (node*)node_ptr->offset;
			}

			char* loc_key = (char*)malloc(sizeof(char)*node_ptr->key_len);
			unsigned char* pos = (unsigned char*)(node_ptr);

			for (i = 12; i < 12 + node_ptr->key_len; i++, j++) {
				loc_key[j] = pos[i];
			}
			loc_key[i - 12] = '\0';

			if (strcmp(key, loc_key) == 0) {
				char* loc_value = (char*)malloc(sizeof(char)*node_ptr->value_len);
				for (j = 0; i < 12 + node_ptr->key_len + node_ptr->value_len; i++, j++) {
					loc_value[j] = pos[i];
				}
				loc_value[j] = '\0';
				return loc_value;
			}

			hashed_location = node_ptr->offset;
			if (hashed_location == -1)
				break;
		}
	}
	return NULL;
}

void init_slab_list() {
	list = (slab_list*)malloc(sizeof(slab_list));

	list->slab_128 = (slab_meta*)malloc(sizeof(slab_meta));
	memset(list->slab_128, 0, sizeof(slab_meta));
	list->slab_128->sub_block_count = 8;
	list->slab_128->s = NULL;

	list->slab_256 = (slab_meta*)malloc(sizeof(slab_meta));
	memset(list->slab_256, 0, sizeof(slab_meta));
	list->slab_256->sub_block_count = 4;
	list->slab_128->s = NULL;

	list->slab_512 = (slab_meta*)malloc(sizeof(slab_meta));
	memset(list->slab_512, 0, sizeof(slab_meta));
	list->slab_512->sub_block_count = 2;
	list->slab_128->s = NULL;

	list->slab_1024 = (slab_meta*)malloc(sizeof(slab_meta));
	memset(list->slab_1024, 0, sizeof(slab_meta));
	list->slab_1024->sub_block_count = 1;
	list->slab_128->s = NULL;
}


void display_slabs() {
	slab_list *ptr = list;

	slab* slab_128_ptr = ptr->slab_128->s;
	node* n;
	printf("Slabs of size 128:\n");
	while (slab_128_ptr != NULL) {
		for (int i = 0; i < ptr->slab_128->sub_block_count; i++) {
			if (slab_128_ptr->bit_vector[i]) {
				n = (node*)(memory + slab_128_ptr->offset + i * 128);
				printf("Address: %d, key len: %d\n", slab_128_ptr->offset + i * 128, n->key_len);
			}
			else
				printf("Address: Empty\n");
		}
		printf("\n");
		slab_128_ptr = slab_128_ptr->next;
	}
	printf("\n");
	slab* slab_256_ptr = ptr->slab_256->s;
	printf("Slabs of size 256:\n");
	while (slab_256_ptr != NULL) {
		for (int i = 0; i < ptr->slab_256->sub_block_count; i++) {
			if (slab_256_ptr->bit_vector[i]) {
				n = (node*)(memory + slab_256_ptr->offset + i * 256);
				printf("Address: %d, key len: %d\n", slab_256_ptr->offset + i * 256, n->key_len);
			}
			else
				printf("Address: Empty\n");
		}
		printf("\n");
		slab_256_ptr = slab_256_ptr->next;
	}
	printf("\n");
	slab* slab_512_ptr = ptr->slab_512->s;
	printf("Slabs of size 512:\n");
	while (slab_512_ptr != NULL) {
		for (int i = 0; i < ptr->slab_512->sub_block_count; i++) {
			if (slab_512_ptr->bit_vector[i]) {
				n = (node*)(memory + slab_512_ptr->offset + i * 512);
				printf("Address: %d, key len: %d\n", slab_512_ptr->offset + i * 512, n->key_len);
			}
			else
				printf("Address: Empty\n");
		}
		printf("\n");
		slab_512_ptr = slab_512_ptr->next;
	}
	printf("\n");
	slab* slab_1024_ptr = ptr->slab_1024->s;
	printf("Slabs of size 1024:\n");
	while (slab_1024_ptr != NULL) {
		for (int i = 0; i < ptr->slab_1024->sub_block_count; i++) {
			if (slab_1024_ptr->bit_vector[i]) {
				n = (node*)(memory + slab_1024_ptr->offset + i * 1024);
				printf("Address: %d, key len: %d\n", slab_1024_ptr->offset + i * 1024, n->key_len);
			}
			else
				printf("Address: Empty\n");
		}
		printf("\n");
		slab_1024_ptr = slab_1024_ptr->next;
	}
}

int main() {

	hash_table* h = (hash_table*)malloc(sizeof(hash_table));
	init_hash_table(h);

	memory = allocate_memory();
	init_slab_list();

	put(h, "abcde", "first try");
	put(h, "band1", "one direction");
	put(h, "band2", "one republic");
	put(h, "best band", "beatles");
	put(h, "cute singer", "taylor swift");
	put(h, "best alternative band", "imagine dragons");
	put(h, "black", "men in black");
	put(h, "greet", "hello hey there");
	put(h, "natural voice", "Evanescence");
	put(h, "natural voice", "katy perry"); //test rewriting value to existing key
	put(h, "high pitch", "adam levine");
	put(h, "big text", "testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
						"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest");
	put(h, "very big text", "testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest");
	put(h, "extra large text", "testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest");
	put(h, "another extra large text", "testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"
		"testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest");
	printf("%s\n", get(h, "band1"));
	printf("%s\n", get(h, "best band"));
	printf("%s\n", get(h, "natural voice"));
	printf("%s\n", get(h, "high pitch"));
	printf("%s\n\n\n", get(h, "ror")); //test non existant key

	display_slabs();
	getchar();
	return 0;
}
