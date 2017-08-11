#define _CRT_SECURE_NO_WARNINGS
#define set_bit(bits, pos) (bits[pos/8] |= (1 << pos%8))
#define unset_bit(bits, pos) (bits[pos/8] &= ~(1 << pos%8))
#define test_bit(bits, pos) (bits[pos/8] & (1 << pos%8))
#define BLOCK_SIZE 1024
#define SIZE 20971520

#include<stdio.h>

typedef struct {
	unsigned char bits[2560]; // bitvector for 20480 blocks
}bit_vector;

typedef struct {
	char message[128];
}msg;

typedef struct {
	char username[16];
	char password[16];
	long msg_pos;
	long image_pos;
}user;

typedef struct {
	long pos;
}user_meta;

typedef struct {
	char name[16];
	char image_name[16];
	long pos;
	long next;
	//int buff[4];
}msg_meta;

typedef struct {
	long msg_meta_pos;
}master_block;

typedef struct {
	char data[1020];
	long next;
}file_block;

typedef struct {
	char name[16];
	//long created_time;
	long pos;
	long next;
	long size;
}file_meta;

FILE *fs_ptr;

void *alloc_block() {
	return malloc(BLOCK_SIZE);
}

void *read_block(int offset) {
	void *memory = alloc_block();
	fseek(fs_ptr, offset, SEEK_SET);
	fread(memory, BLOCK_SIZE, 1, fs_ptr);
	return memory;
}

void write_block(int offset, void* memory) {
	fseek(fs_ptr, offset, SEEK_SET);
	fwrite(memory, BLOCK_SIZE, 1, fs_ptr);
}

long get_file_size(FILE* fp) {
	fseek(fp, 0, SEEK_END);
	return ftell(fp);
}

void write_users(user *users) {
	write_block(3 * BLOCK_SIZE, users);
}

void* read_users() {
	return read_block(3 * BLOCK_SIZE);
}

int write_bit_vector(bit_vector* vector) {
	int size = sizeof(vector->bits);
	int block = 0;
	while (size >= BLOCK_SIZE) {
		size -= BLOCK_SIZE;
		write_block(block * BLOCK_SIZE, vector->bits + (block * BLOCK_SIZE));
		block++;
	}
	if (size > 0)
		write_block(block * BLOCK_SIZE, vector->bits + (block * BLOCK_SIZE));
	return block;
}

void* read_bit_vector() {
	bit_vector* vector = (bit_vector*)malloc(sizeof(bit_vector));
	int size = sizeof(vector->bits);
	int block = 0;
	while (size >= BLOCK_SIZE) {
		size -= BLOCK_SIZE;
		void *ptr = (vector + block * BLOCK_SIZE);
		ptr = read_block(block * BLOCK_SIZE);
		block++;
	}
	if (size > 0) {
		void *ptr = (vector + block * BLOCK_SIZE);
		ptr = read_block(block * BLOCK_SIZE);
	}
	return vector;
}


int get_free_block_offset() {
	bit_vector *bv = (bit_vector*)malloc(sizeof(bit_vector));
	int offset = 0, i = 1;
	bv = (bit_vector*)read_block(offset);
	while (offset <= 2 * BLOCK_SIZE) {
		while (i < BLOCK_SIZE && bv->bits[i] == 0)
			i++;

		if (i != BLOCK_SIZE) {
			for (int j = 0; j < 8; j++) {
				int temp = i*8 + j;
				if (test_bit(bv->bits, temp)) {
					unset_bit(bv->bits, temp);
					write_block(offset, bv->bits);
					return temp * BLOCK_SIZE;
				}
			}
		}
		offset += BLOCK_SIZE;
		free(bv);
		bv = (bit_vector*)read_block(offset);
		i = 0;
	}
	while (i < BLOCK_SIZE / 2 && bv->bits[i] == 0)
		i++;
	if (i != BLOCK_SIZE / 2) {
		for (int j = 0; j < 8; j++) {
			int temp = i * 8 + j;
			if (test_bit(bv->bits, temp)) {
				unset_bit(bv->bits, temp);
				write_block(offset, bv->bits);
				free(bv);
				return temp * BLOCK_SIZE;
			}
		}
	}
	else return -1;
}

void upload_file(user* users, user* current_user, char *file_name) {
	int file_ptr = fopen(file_name, "rb+");
	int size = get_file_size(file_ptr);
	rewind(file_ptr);
	file_meta *file_meta_ptr = (file_meta*)alloc_block();
	file_block *f = (file_block*)alloc_block();
	file_meta *meta_ptr_list;
	memset(file_meta_ptr, 0, BLOCK_SIZE);
	int blocks = size / (BLOCK_SIZE - 4);
	file_meta_ptr->size = size;
	
	int file_meta_pos = get_free_block_offset();
	int file_pos = get_free_block_offset();
	strcpy(file_meta_ptr->name, file_name);
	file_meta_ptr->pos = file_pos;
	file_meta_ptr->next = -1;

	long file_meta_offset = current_user->image_pos;
	if (file_meta_offset == -1) {
		current_user->image_pos = file_meta_pos;
		for (int i = 0; i < 20; i++) {
			if (strcmp(users[i].username, current_user->username) == 0) {
				users[i].image_pos = current_user->image_pos;
				break;
			}
		}
		write_users(users);
	}
	else {
		while (1) {
			meta_ptr_list = (file_meta*)read_block(file_meta_offset);
			if (meta_ptr_list->next == -1) {
				meta_ptr_list->next = file_meta_pos;
				write_block(file_meta_offset, meta_ptr_list);
				break;
			}
			file_meta_offset = meta_ptr_list->next;
			free(meta_ptr_list);
		}
	}
	write_block(file_meta_pos, file_meta_ptr);
	free(file_meta_ptr);
//	write_block(file_pos, msg_ptr);

	int new_pos;
	while (blocks--) {
		for (int i = 0; i < 1020; i++) {
			f->data[i] = fgetc(file_ptr);
		}
		new_pos = get_free_block_offset();
		f->next = new_pos;
		write_block(file_pos, f);
		file_pos = new_pos;
		
		//file_pos = f->next;
	}
	blocks = size % (BLOCK_SIZE - 4);
	int i = 0;
	memset(f, 0, BLOCK_SIZE);
	while (i < blocks) {
		f->data[i] = fgetc(file_ptr);
		i++;
	}
	f->next = -1;
	write_block(file_pos, f);
	free(f);
}

void download_file(user* current_user, char* input_file,  char* filename) {
	FILE *fp = fopen(filename, "wb");
	
	int file_meta_offset = current_user->image_pos;
	file_meta* meta_ptr = read_block(file_meta_offset);
	file_block* f;
	while (file_meta_offset != -1) {
		if (strcmp(meta_ptr->name, input_file) == 0) {
			int size = meta_ptr->size;
			int blocks = size / (BLOCK_SIZE - 4);
			int offset = meta_ptr->pos;
			while (blocks--) {
				f = read_block(offset);
				fwrite(f->data, 1020, 1, fp);
				offset = f->next;
			}
			blocks = size % (BLOCK_SIZE - 4);
			f = read_block(offset);
			fwrite(f->data, blocks, 1, fp);
			break;
		}
		file_meta_offset = meta_ptr->next;
		meta_ptr = read_block(file_meta_offset);
		
	}
	fclose(fp);

}

void insert_message(user *current_user, msg_meta* meta_ptr, msg* msg_ptr) {
	msg_meta* meta_ptr_list;
	long msg_meta_offset = current_user->msg_pos;
	if (msg_meta_offset == -1) {
		msg_meta_offset = get_free_block_offset();
		current_user->msg_pos = msg_meta_offset;
		user* users = read_users();
		for (int i = 0; i < 20; i++) {
			if (strcmp(users[i].username, current_user->username) == 0) {
				users[i].msg_pos = current_user->msg_pos;
				break;
			}
		}
		write_users(users);
		free(users);
	}
	else {
		while (1) {
			meta_ptr_list = (msg_meta*)read_block(msg_meta_offset);
			if (meta_ptr_list->next == -1) {
				long prev_offset = msg_meta_offset;
				msg_meta_offset = get_free_block_offset();
				meta_ptr_list->next = msg_meta_offset;
				write_block(prev_offset, meta_ptr_list);
				break;
			}
			msg_meta_offset = meta_ptr_list->next;
		}
		
	}
	long msg_offset = get_free_block_offset();
	meta_ptr->pos = msg_offset;
	write_block(msg_meta_offset, meta_ptr);
	write_block(msg_offset, msg_ptr);

}

void create_message(user* user, char* img_name, char* message) {
	msg_meta* meta_ptr = (msg_meta*)malloc(sizeof(msg_meta));
	msg* new_msg_ptr = (msg*)malloc(sizeof(msg));
	strcpy(new_msg_ptr->message, message);
	strcpy(meta_ptr->image_name, img_name);
	strcpy(meta_ptr->name, user->username);
	meta_ptr->next = -1;
	insert_message(user, meta_ptr, new_msg_ptr);
}

msg* fetch_messages(user *current_user) {
	int msg_meta_offset = current_user->msg_pos;
	if (msg_meta_offset == -1)
		return NULL;
	int len = 0;
	int maxlen = 20;
	msg* messages = (msg*)malloc(sizeof(msg)*20);
	memset(messages, 0, (sizeof(msg) * 20));
	msg_meta* meta_ptr_list;
	msg *message;
	while (1) {
		meta_ptr_list = (msg_meta*)read_block(msg_meta_offset);
		if (len == maxlen - 1) {
			maxlen *= 2;
			messages = realloc(messages, maxlen);
		}
		message = (msg*)read_block(meta_ptr_list->pos);	
		strcpy(messages[len++].message, message->message);
		msg_meta_offset = meta_ptr_list->next;
		if (meta_ptr_list->next == -1)
			break;
		free(meta_ptr_list);
		free(message);
	}
	strcpy(messages[len].message, "");
	return messages;
}

void delete_message(bit_vector* vector, user *current_user, int msg_count) {
	int msg_meta_offset = current_user->msg_pos;
	if (msg_meta_offset == -1)
		return NULL;
	msg_meta* meta_ptr_list = NULL;
	msg_meta* prev_meta_ptr = NULL;
	msg_meta* delete_node = NULL;
	msg *message = NULL;
	while (--msg_count) {
		if (prev_meta_ptr != NULL)
			free(prev_meta_ptr);
		prev_meta_ptr = meta_ptr_list;
		meta_ptr_list = (msg_meta*)read_block(msg_meta_offset);
		msg_meta_offset = meta_ptr_list->next;
		if (meta_ptr_list->next == -1)
			break;
	}
	
	delete_node = (msg_meta*)read_block(msg_meta_offset);
	int pos = delete_node->pos;
	if (current_user->msg_pos == msg_meta_offset) {
		meta_ptr_list = delete_node;
		current_user->msg_pos = meta_ptr_list->next;
		user* users = read_users();
		for (int i = 0; i < 20; i++) {
			if (strcmp(users[i].username, current_user->username) == 0) {
				users->msg_pos = current_user->msg_pos;
				write_users(users);
				break;
			}
		}
		set_bit(vector->bits, msg_meta_offset);
		set_bit(vector->bits, pos);
	}
	else {
		if (prev_meta_ptr == NULL) {
			prev_meta_ptr = (msg_meta*)read_block(current_user->msg_pos);
			meta_ptr_list->next = delete_node->next;
			write_block(current_user->msg_pos, meta_ptr_list);
		}
		else {
			meta_ptr_list->next = delete_node->next;
			write_block(prev_meta_ptr->next, meta_ptr_list);
		}
		set_bit(vector->bits, msg_meta_offset);
		set_bit(vector->bits, pos);
	}
	write_bit_vector(vector);
}


int register_user(user *new_user) {
	user* users = read_block(3 * BLOCK_SIZE);
	for (int i = 0; i < 20; i++) {
		if (strcmp(users[i].username, new_user->username) == 0) {
			printf("User limit reached!\n");
			return 0;
		}
		else if (strcmp(users[i].username, "") == 0) {
			strcpy(users[i].username, new_user->username);
			strcpy(users[i].password, new_user->password);
			users[i].image_pos = -1;
			users[i].msg_pos = -1;
			write_users(users);
			return 1;
		}
	}
	return 5;
}

int authenticate(user *u) {
	user *users = (user*)read_block(3 * BLOCK_SIZE);
	for (int i = 0; i < 20; i++) {
		if (strcmp(users[i].username, u->username) == 0 && strcmp(users[i].password, u->password) == 0) {
			u->image_pos = users[i].image_pos;
			u->msg_pos = users[i].msg_pos;
			return 1;
		}
	}
	return 0;
}

void cleardb(bit_vector *bv, user *users) {
	write_bit_vector(bv);
	write_users(users);
}

void loop() {
	user *users = (user*)malloc(sizeof(user)*20);
	bit_vector *bv = (bit_vector*)malloc(sizeof(bit_vector));
	user* current_user = (user*)malloc(sizeof(user));
	user* new_user = (user*)malloc(sizeof(user));
	msg* messages;
	memset(users, 0, sizeof(user) * 20);
	memset(&bv->bits, 255, 2560);
	fs_ptr = fopen("myfs.dat", "rb+");
	if (fs_ptr == 0) {
		system("fsutil file createnew myfs.dat 20971520");
		fs_ptr = fopen("myfs.dat", "rb+");
		cleardb(bv, users);
	}
	else {
		users = read_users();
		bv = read_bit_vector();
	}

	char* command = (char*)malloc(sizeof(char) * 100);
	char* command2 = (char*)malloc(sizeof(char) * 100);
	char* image_name = (char*)malloc(sizeof(char) * 16);
	char* message = (char*)malloc(sizeof(char) * 16);

	while (1) {
		printf("Allowed commands\n\n");
		printf("Login\n");
		printf("Register\n\n");
		scanf("%s", command);
		if (strcmp(command, "Login") == 0) {
			printf("Enter username:\n");
			scanf("%s", current_user->username);
			printf("Enter password:\n");
			scanf("%s", current_user->password);
			if (!authenticate(current_user)) {
				printf("Authentication failed\n\n");
			}
			else {
				printf("Successfully authenticated\n\n");
				for (int i = 0; i < 20; i++) {
					if (strcmp(users[i].username, current_user->username) == 0) {
						current_user->msg_pos = users[i].msg_pos;
						current_user->image_pos = users[i].image_pos;
					}
				}
				break;
			}
		}
		else if (strcmp(command, "Register") == 0) {
			printf("Enter username:\n");
			scanf("%s", new_user->username);
			printf("Enter password:\n");
			scanf("%s", new_user->password);
			if (!register_user(new_user)) {
				printf("Registration failed\n");
			}
			else {
				printf("Successfully registered\n");
				printf("Now logging you in\n\n");
				authenticate(new_user);
				current_user = new_user;
				current_user->image_pos = -1;
				current_user->msg_pos = -1;
				break;
			}
			users = read_users();
		}
		else
			printf("Unknown command\n\n");

	}


	printf("Allowed commands:\n\n");
	printf("create (for creating messages)\n");
	printf("read\n");
	printf("delete\n");
	printf("enum\n");
	printf("cleardb\n");
	printf("upload\n");
	printf("download\n");
	printf("exit\n\n");

	while (1) {
		printf("> ");
		scanf("%s", command);

		if (strcmp(command, "create") == 0) {
			printf("Enter image name: ");
			scanf("%s", image_name);
			printf("Enter message: ");
			scanf("%s", message);
			create_message(current_user, image_name, message);
		}
		
		else if (strcmp(command, "read") == 0) {
			messages = fetch_messages(current_user);
			int i = 0;
			while (messages && strcmp(messages[i].message, "") != 0) {
				printf("%s\n", messages[i].message);
				i++;
			}
		}
		else if (strcmp(command, "delete") == 0) {
			messages = fetch_messages(current_user);
			int i = 0;
			while (strcmp(messages[i].message, "") != 0) {
				printf("%d. %s\n", i+1, messages[i].message);
				i++;
			}
			printf("Choose one message to delete: ");
			scanf("%d", &i);
			delete_message(bv, current_user, i);
		}
		else if (strcmp(command, "enum") == 0) {

		}
		else if (strcmp(command, "upload") == 0) {
			printf("Specify the file name: ");
			scanf("%s", command);
			int fp = fopen(command, "rb+");
			if (fp == 0) {
				printf("File not found/unable to open\n");
				fclose(fp);
				break;
			}
			fclose(fp);
			upload_file(users, current_user, command);
		}
		else if (strcmp(command, "download") == 0) {
			printf("Specify the file to download: ");
			scanf("%s", command);
			printf("Set the name of the downloaded file: ");
			scanf("%s", command2); 
			int fp = fopen(command2, "rb");
			if (fp != 0) {
				fclose(fp);
				printf("File already exists\n");
				break;
			}
			download_file(current_user, command, command2);
		}
		else if (strcmp(command, "delete") == 0) {
			printf("Specify the file to delete: ");
			scanf("%s", command);
		}
		else if (strcmp(command, "cleardb") == 0) {
			memset(&users, 0, sizeof(user) * 20);
			memset(&bv->bits, 255, 2560);
			cleardb(bv, users);
		}
		else if (strcmp(command, "exit") == 0) {
			break;
		}
		else
			printf("Unknown command\n");
	}
	fclose(fs_ptr);
	free(users);
	free(bv);
}

int main() {
	loop();

}
