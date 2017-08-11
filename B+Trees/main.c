#define _CRT_SECURE_NO_WARNINGS
#define PAGE_SIZE 64
#define COURSE_COUNT 12
#define POOL_LEN 4

#include <time.h>
#include <stdio.h>

//#define INT_SIZE PAGE_SIZE/sizeof(int)
/*
HASH JOIN - used when the cardinaality is low
*/


typedef struct {
	int id;
	int course_id;
	char name[19];
	unsigned char marks;
}student;

typedef struct {
	int id;
	char name[16];
}course;

typedef struct {
	 course courses[3];
}course_leaf_page;

typedef struct {
	unsigned char page_type;
	unsigned char table_id;
	short level;
	unsigned char data[0];
}page_info;

typedef struct {
	int table_id;
	int root_page;
	char table_name[8];
}table_info;

typedef struct {
	table_info tables[4];
}header;

typedef struct {
	student s[2];
	int pad;
}student_leaf_page;

typedef struct {
	int keys[7];
	int children[8];
}non_leaf_page;

typedef struct {
	int page_id;
	int last_access_time;
	page_info* page;
	unsigned char free;
}page_pool_entry;

typedef struct {
	page_pool_entry entries[POOL_LEN];
}page_pool;

page_pool *pool;


int get_free_or_lru_page() {
	for (int i = 0; i < POOL_LEN; i++) {
		if (pool->entries[i].free == 1)
			return i;
	}
	int min = 0;
	for (int i = 1; i < POOL_LEN; i++) {
		if (pool->entries[min].last_access_time < pool->entries[i].last_access_time)
			min = i;
	}
	return min;
}

void update_pool(int free_page, int page_id) {
	pool->entries[free_page].free = 0;
	pool->entries[free_page].page_id = page_id;
	pool->entries[free_page].last_access_time = clock(); //current time
}

int get_page(int page_id) {
	for (int i = 0; i < POOL_LEN; i++) {
		if (!(pool->entries[i].free) && pool->entries[i].page_id == page_id) {
			pool->entries[i].last_access_time = clock();
			return i;
		}
	}
	return -1;
}

void* alloc_page() {
	return calloc(PAGE_SIZE, 1);
}

void write_page(FILE *fp, int offset, void *memory) {
	fseek(fp, offset, SEEK_SET);
	fwrite(memory, PAGE_SIZE, 1, fp);
}


void init_page_pool() {
	pool = (page_pool*)malloc(sizeof(page_pool));
	for (int i = 0; i < POOL_LEN; i++) {
		pool->entries[i].free = 1;
		pool->entries[i].page_id = -1;
		pool->entries[i].last_access_time = 0;
		pool->entries[i].page = (page_info*)alloc_page();
	}
}

void* read_page(FILE *fp, int offset) {
	int page_loc = get_page(offset);
	if (page_loc != -1) {
		pool->entries[page_loc].last_access_time = clock();
		return pool->entries[page_loc].page;
	}
	int free_page = get_free_or_lru_page();
	page_pool_entry* page = &(pool->entries[free_page]);
	fseek(fp, offset, SEEK_SET);
	fread(page->page, PAGE_SIZE, 1, fp);
	update_pool(free_page, offset);
	return page->page;
}

page_info* create_non_leaf_page() {
	page_info* p_info = (page_info*)alloc_page();
	p_info->page_type = 3;
	non_leaf_page* p = (non_leaf_page*)(p_info->data);
	int i;
	for (i = 0; i < 7; i++) {
		p->keys[i] = 0;
		p->children[i] = -1;
	}
	p->children[i] = -1;
	return p_info;
}

page_info* create_header() {
	page_info *page = (page_info*)alloc_page();
	header* header_block = (header*)page->data;
	header_block->tables[0].table_id = 1;
	strcpy(header_block->tables[0].table_name, "student");
	header_block->tables[0].table_id = 2;
	strcpy(header_block->tables[0].table_name, "course");
	return header_block;
}

page_info* create_course_leaf_page(course **c) {
	page_info *memory = (page_info*)alloc_page();
	memory->level = 0;
	memory->page_type = 2;
	memory->table_id = 2;

	course_leaf_page *clp = (course_leaf_page*)(memory->data);
	for (int i = 0; i < 3; i++) {
		clp->courses[i].id = c[i]->id;
		strcpy(clp->courses[i].name, c[i]->name);
	}
	return memory;
}

page_info* create_student_leaf_page(student **s) {
	page_info *memory = (page_info*)alloc_page();
	memory->level = 0;
	memory->page_type = 1;
	memory->table_id = 1;
	student_leaf_page *sp = (student_leaf_page*)(memory->data);
	sp->s[0].id = s[0]->id;
	sp->s[1].id = s[1]->id;
	strcpy(sp->s[0].name, s[0]->name);
	strcpy(sp->s[1].name, s[1]->name);
	sp->s[0].course_id = s[0]->course_id;
	sp->s[1].course_id = s[1]->course_id;
	sp->s[0].marks = s[0]->marks;
	sp->s[1].marks = s[1]->marks;

	return memory;
}

void display_non_leaf_page(non_leaf_page *p) {
	int i;
	printf("Node data: \n");
	for (i = 0; i < 7; i++) {
		printf("%3d ", p->keys[i]);
	}
	printf("\n");
	printf("Child pointers: \n");
	for (i = 0; i < 8; i++) {
		printf("%3d ", p->children[i]);
	}
	printf("\n\n");
}

void update_parent_of_student_leaf(FILE* fp, non_leaf_page* p) {
	page_info* p_info;
	student_leaf_page* slp;
	for (int i = 1; i <= 7; i++) {
		p_info = (page_info*)read_page(fp, p->children[i]);
		slp = (student_leaf_page*)(p_info->data);
		p->keys[i - 1] = slp->s[0].id;
	}
}

void update_parent_of_course_leaf(FILE* fp, non_leaf_page* p) {
	page_info* p_info;
	course_leaf_page* clp;
	for (int i = 1; i <= 7; i++) {
		p_info = (page_info*)read_page(fp, p->children[i]);
		clp = (course_leaf_page*)(p_info->data);
		p->keys[i - 1] = clp->courses[0].id;
	}
}

void update_parent_of_non_leaf(FILE* fp, non_leaf_page* p) {
	page_info* p_info;
	non_leaf_page* nlp;
	for (int i = 1; i <= 7; i++) {
		if (p->children[i] != -1) {
			p_info = (page_info*)read_page(fp, p->children[i]);
			nlp = (non_leaf_page*)(p_info->data);
			p->keys[i - 1] = nlp->keys[i];
		}
	}
}

void display_student_leaf_page(student_leaf_page *p) {
	printf("Leaf Data: \n");
	printf("%d %d %s %d\n", p->s[0].id, p->s[0].course_id, p->s[0].name, p->s[0].marks);
	printf("%d %d %s %d\n", p->s[1].id, p->s[1].course_id, p->s[1].name, p->s[1].marks);
	printf("\n\n");
}

void add_leaf(non_leaf_page* p, int pos, int offset) {
	p->children[pos] = offset;
}

int search_record(FILE* fp, int id, int offset) {
	page_info* page = (page_info*)read_page(fp, offset);
	student_leaf_page* slp;
	course_leaf_page* clp;
	non_leaf_page* nlp;
	int i;
	switch (page->page_type) {
	case 1:
		slp = (student_leaf_page*)page->data;
		for (i = 0; i < 2; i++) {
			if (slp->s[i].id == id) {
				return ftell(fp) - PAGE_SIZE;
			}
		}
		if (i == 2)
			return -1;
		break;
	case 2:
		clp = (course_leaf_page*)page->data;
		for (i = 0; i < 3; i++) {
			if (clp->courses[i].id == id) {
				return ftell(fp) - PAGE_SIZE;
			}
		}
		if (i == 2)
			return -1;
		break;
	case 3:

		nlp = (non_leaf_page*)page->data;
			for (i = 0; i < 7; i++) {
				
			if (i == 0 && id < nlp->keys[i]) {
				return search_record(fp, id, nlp->children[0]);
			}
			else if (nlp->keys[i] == 0 || id < nlp->keys[i]) {
				return search_record(fp, id, nlp->children[i]);
			}
			else if (id >= nlp->keys[i]) {
				continue;
			}
		}
		if(i == 7)
			return search_record(fp, id, nlp->children[i]);

	}
}

int find_max_enrollments(FILE* fp) {
	header *header_table = (header*)(((page_info*)read_page(fp, 0))->data);
	int root_offset = header_table->tables[0].root_page;
	page_info* page = (page_info*)read_page(fp, root_offset);
	int courses_hash[COURSE_COUNT];
	memset(&courses_hash, 0, sizeof(courses_hash));

	while (page->page_type != 1) { // iterate until we get the leaf
		non_leaf_page* nlp = (non_leaf_page*)page->data;
		page = read_page(fp, nlp->children[0]);
	}
	int leaf_offset = ftell(fp);
	while (page->page_type == 1) {
		student_leaf_page* slp = (student_leaf_page*)(page->data);
		leaf_offset += PAGE_SIZE;
		courses_hash[slp->s[0].course_id - 1]++;
		courses_hash[slp->s[1].course_id - 1]++;
		page = read_page(fp, leaf_offset);
	}

	int max = -1;
	for (int i = 0; i < 12; i++) {
		printf("%d  ", courses_hash[i]);
		if (courses_hash[max] < courses_hash[i])
			max = i;
	}
	printf("\n");

	printf("Maximum enrollments is in course %d with %d enrollments\n\n", max + 1, courses_hash[max]);

}


int join_students_courses(FILE* fp) {
	header *header_table = (header*)(((page_info*)read_page(fp, 0))->data);
	int student_root_offset = header_table->tables[0].root_page;
	int course_root_offset = header_table->tables[1].root_page;
	page_info* page = (page_info*)read_page(fp, student_root_offset);
	student_leaf_page slp_obj;
	student_leaf_page slp_obj2;

	int course_offset, i;
	course_leaf_page* clp;

	while (page->page_type != 1) { // iterate until we get the leaf
		non_leaf_page* nlp = (non_leaf_page*)page->data;
		page = read_page(fp, nlp->children[0]);
	}
	int leaf_offset = ftell(fp);
	printf("%6s %15s %6s %6s %15s\n\n", "ID", "NAME", "MARKS", "COURSE", "CNAME");
	while (page->page_type == 1) {
		student_leaf_page* slp = (student_leaf_page*)(page->data);
		slp_obj.s[0].course_id = slp->s[0].course_id;
		slp_obj.s[0].id = slp->s[0].id;
		strcpy(slp_obj.s[0].name, slp->s[0].name);
		slp_obj.s[0].marks = slp->s[0].marks;
		slp_obj.s[1].course_id = slp->s[1].course_id;
		slp_obj.s[1].id = slp->s[1].id;
		strcpy(slp_obj.s[1].name, slp->s[1].name);
		slp_obj.s[1].marks = slp->s[1].marks;
		leaf_offset += PAGE_SIZE;
		course_offset = search_record(fp, slp_obj.s[0].course_id, course_root_offset);
		if (course_offset >= 0) {
			clp = (course_leaf_page*)((page_info*)read_page(fp, course_offset))->data;
			for (i = 0; i < 3; i++) {
				if (clp->courses[i].id == slp_obj.s[0].course_id) {
					printf("%6d %15s %6d %6d %15s\n", slp_obj.s[0].id, slp_obj.s[0].name, slp_obj.s[0].marks, slp_obj.s[0].course_id, clp->courses[i].name);
					break;
				}
			}
			
		}
		course_offset = search_record(fp, slp_obj.s[1].course_id, course_root_offset);
		if (course_offset >= 0) {
			
			clp = (course_leaf_page*)((page_info*)read_page(fp, course_offset))->data;
			for (i = 0; i < 3; i++) {
				if (clp->courses[i].id == slp_obj.s[1].course_id) {
					printf("%6d %15s %6d %6d %15s\n", slp_obj.s[1].id, slp_obj.s[1].name, slp_obj.s[1].marks, slp_obj.s[1].course_id, clp->courses[i].name);
					break;
				}
			}

		}
		page = read_page(fp, leaf_offset);
	}
}

/*
int group_by_courses(FILE* fp) {
	header *header_table = (header*)(((page_info*)read_page(fp, 0))->data);
	int student_root_offset = header_table->tables[0].root_page;
	int course_root_offset = header_table->tables[1].root_page;
	page_info* page = (page_info*)read_page(fp, student_root_offset);
	course_leaf_page clp_obj;

	int student_offset, i;
	student_leaf_page* slp;

	while (page->page_type != 1) { // iterate until we get the leaf
		non_leaf_page* nlp = (non_leaf_page*)page->data;
		page = read_page(fp, nlp->children[0]);
	}
	int leaf_offset = ftell(fp);
	printf("%15s %15s\n\n", "COURSE", "STUDENTS");
	while (page->page_type == 1) {
		course_leaf_page* clp = (course_leaf_page*)(page->data);
		leaf_offset += PAGE_SIZE;

		
		clp_obj.courses[0].id = clp->courses[0].id;
		strcpy(clp_obj.courses[0].name, clp->courses[0].name);
		clp_obj.courses[1].id = clp->courses[1].id;
		strcpy(clp_obj.courses[1].name, clp->courses[1].name);
		clp_obj.courses[2].id = clp->courses[2].id;
		strcpy(clp_obj.courses[2].name, clp->courses[2].name);

		for (int j = 0; j < 3; j++) {
			student_offset = search_record(fp, clp_obj.courses[j].id, student_root_offset);
			if (student_offset >= 0) {
				slp = (student_leaf_page*)((page_info*)read_page(fp, student_offset))->data;
				for (i = 0; i < 2; i++) {
					if (slp->s[i].course_id == clp_obj.courses[j].id) {
						printf("%15s %15s\n", clp_obj.courses[j].name, slp->s[i]);
						break;
					}
				}
			}
		}
		
		
		
		if (course_offset >= 0) {

			clp = (course_leaf_page*)((page_info*)read_page(fp, course_offset))->data;
			for (i = 0; i < 3; i++) {
				if (clp->courses[i].id == slp_obj.s[1].course_id) {
					printf("%6d %15s %6d %6d %15s\n", slp_obj.s[1].id, slp_obj.s[1].name, slp_obj.s[1].marks, slp_obj.s[1].course_id, clp->courses[i].name);
					break;
				}
			}

		}
		page = read_page(fp, leaf_offset);
	}
}
*/

int compute_avg_by_id_range(FILE* fp, int low, int high) {
	header *header_table = (header*)(((page_info*)read_page(fp, 0))->data);
	int root_offset = header_table->tables[0].root_page;
	page_info* page = (page_info*)read_page(fp, root_offset);
	float sum = 0;
	int count = 0;
	while (page->page_type != 1) { // iterate until we get the leaf
		non_leaf_page* nlp = (non_leaf_page*)page->data;
		page = read_page(fp, nlp->children[0]);
	}
	int leaf_offset = ftell(fp);
	int range_complete = 0;
	while (page->page_type == 1) {
		student_leaf_page* slp = (student_leaf_page*)(page->data);
		leaf_offset += PAGE_SIZE;

		if (slp->s[1].id > low) {
			if (slp->s[0].id > low) {
				sum += slp->s[0].marks;
				count++;
			}
			sum += slp->s[1].marks;
			count++;
			page = read_page(fp, leaf_offset);
			break;
		}
		page = read_page(fp, leaf_offset);
	}

	while (page->page_type == 1) {
		student_leaf_page* slp = (student_leaf_page*)(page->data);
		leaf_offset += PAGE_SIZE;

		if (slp->s[1].id >= high) {
			if (slp->s[0].id < high) {
				sum += slp->s[0].marks;
				count++;
			}
			break;
		}
		else {
			sum += slp->s[0].marks + slp->s[1].marks;
			count+=2;
		}
		page = read_page(fp, leaf_offset);
	}

	printf("Average: %f\n", sum / count);
}


int select_records_by_id_range(FILE* fp, int low, int high) {
	header *header_table = (header*)(((page_info*)read_page(fp, 0))->data);
	int root_offset = header_table->tables[0].root_page;
	page_info* page = (page_info*)read_page(fp, root_offset);

	while (page->page_type != 1) { // iterate until we get the leaf
		non_leaf_page* nlp = (non_leaf_page*)page->data;
		page = read_page(fp, nlp->children[0]);
	}
	int leaf_offset = ftell(fp);
	int range_complete = 0;
	while (page->page_type == 1) {
		student_leaf_page* slp = (student_leaf_page*)(page->data);
		leaf_offset += PAGE_SIZE;
		
		if (slp->s[1].id > low) {
			if (slp->s[0].id > low) {
				printf("%3d %3d %10s %3d\n", slp->s[0].id, slp->s[0].course_id, slp->s[0].name, slp->s[0].marks);
			}
			printf("%3d %3d %10s %3d\n", slp->s[1].id, slp->s[1].course_id, slp->s[1].name, slp->s[1].marks);
			page = read_page(fp, leaf_offset);
			break;
		}
		page = read_page(fp, leaf_offset);
	}

	while (page->page_type == 1) {
		student_leaf_page* slp = (student_leaf_page*)(page->data);
		leaf_offset += PAGE_SIZE;

		if (slp->s[1].id >= high) {
			if (slp->s[0].id < high) {
				printf("%3d %3d %10s %3d\n", slp->s[0].id, slp->s[0].course_id, slp->s[0].name, slp->s[0].marks);
			}
			break;
		}
		else {
			printf("%3d %3d %10s %3d\n", slp->s[0].id, slp->s[0].course_id, slp->s[0].name, slp->s[0].marks);
			printf("%3d %3d %10s %3d\n", slp->s[1].id, slp->s[1].course_id, slp->s[1].name, slp->s[1].marks);
		}
		page = read_page(fp, leaf_offset);
	}
}


int count_records_by_id_range(FILE* fp, int low, int high) {
	header *header_table = (header*)(((page_info*)read_page(fp, 0))->data);
	int root_offset = header_table->tables[0].root_page;
	page_info* page = (page_info*)read_page(fp, root_offset);
	int count = 0;

	while (page->page_type != 1) { // iterate until we get the leaf
		non_leaf_page* nlp = (non_leaf_page*)page->data;
		page = read_page(fp, nlp->children[0]);
	}
	int leaf_offset = ftell(fp);
	int range_complete = 0;
	while (page->page_type == 1) {
		student_leaf_page* slp = (student_leaf_page*)(page->data);
		leaf_offset += PAGE_SIZE;

		if (slp->s[1].id > low) {
			if (slp->s[0].id > low) {
				count++;
			}
			count++;
			page = read_page(fp, leaf_offset);
			break;
		}
		page = read_page(fp, leaf_offset);
	}

	while (page->page_type == 1) {
		student_leaf_page* slp = (student_leaf_page*)(page->data);
		leaf_offset += PAGE_SIZE;

		if (slp->s[1].id >= high) {
			if (slp->s[0].id < high) {
				count++;
			}
			break;
		}
		else {
			count += 2;
		}
		page = read_page(fp, leaf_offset);
	}
	printf("Count  = %d\n", count);
}


int main() {
	init_page_pool();
	student **s = (student**)malloc(sizeof(student*)*2);
	s[0] = (student*)malloc(sizeof(student));
	s[1] = (student*)malloc(sizeof(student));

	course **c = (course**)malloc(sizeof(course*)*3);
	c[0] = (course*)malloc(sizeof(course));
	c[1] = (course*)malloc(sizeof(course));
	c[2] = (course*)malloc(sizeof(course));

	page_info *l;
	page_info *cl;
	int ptr = PAGE_SIZE;
	page_info* header_table_page = create_header();
	header* header_table = (header*)header_table_page->data;
	page_info* nlp_student[3];
	non_leaf_page* nlp_student_ptr;
	nlp_student[0] = create_non_leaf_page();
	nlp_student[1] = create_non_leaf_page();
	nlp_student[2] = create_non_leaf_page();

	page_info* nlp_course = create_non_leaf_page();
	//page_info *metadata = (page_info*)alloc_page();

	FILE *student_fp = fopen("records.txt", "rb");
	FILE *course_fp = fopen("courses.csv", "rb");
	FILE *bplus_tree = fopen("tree.dat", "rb+");

	//1. WRITE HEADER TO DB
	write_page(bplus_tree, 0, header_table_page);

	//2. WRITE STUDENT RECORDS TO DB
	int j = 0;
	int i;
	for (i = 0; i < 8; i++) {
		memset(s[0], 0, sizeof(student));
		memset(s[1], 0, sizeof(student));
		fscanf(student_fp, "%d %s %d %d", &(s[0]->id), s[0]->name, &(s[0]->course_id), &(s[0]->marks));
		fscanf(student_fp, "%d %s %d %d", &(s[1]->id), s[1]->name, &(s[1]->course_id), &(s[1]->marks));
		
		l = create_student_leaf_page(s);
		//display_student_leaf_page(l->data);
		add_leaf(nlp_student[0]->data, i, (i + 1) * PAGE_SIZE);
		write_page(bplus_tree, (i + 1) * PAGE_SIZE, l);
	}
	j = i + 1;

	int data = 0;
	for (i = 0; i < 8; i++) {
		memset(s[0], 0, sizeof(student));
		memset(s[1], 0, sizeof(student));
		fscanf(student_fp, "%d %s %d %d", &(s[0]->id), s[0]->name, &(s[0]->course_id), &(s[0]->marks));
		if (data == 0)
			data = s[0]->id;
		fscanf(student_fp, "%d %s %d %d", &(s[1]->id), s[1]->name, &(s[1]->course_id), &(s[1]->marks));
		l = create_student_leaf_page(s);
		//display_leaf_page(l->data);
		add_leaf(nlp_student[1]->data, i, (j + i) * PAGE_SIZE);
		write_page(bplus_tree, (j + i) * PAGE_SIZE, l);
	}

	update_parent_of_student_leaf(bplus_tree, nlp_student[0]->data);
	update_parent_of_student_leaf(bplus_tree, nlp_student[1]->data);
	display_non_leaf_page(nlp_student[0]->data);
	write_page(bplus_tree, (i + j) * PAGE_SIZE, nlp_student[0]);

	add_leaf(nlp_student[2]->data, 0, (i + j) * PAGE_SIZE);
	j++;
	display_non_leaf_page(nlp_student[1]->data);
	
	write_page(bplus_tree, (i + j) * PAGE_SIZE, nlp_student[1]);

	add_leaf(nlp_student[2]->data, 1, (i + j) * PAGE_SIZE);
	j++;

	header_table->tables[0].root_page = (i + j) * PAGE_SIZE;
	header_table->tables[0].table_id = 1;
	strcpy(header_table->tables[0].table_name, "Student");
	non_leaf_page *root_page = (non_leaf_page*)nlp_student[2]->data;
	root_page->keys[0] = data;
	write_page(bplus_tree, (i + j++) * PAGE_SIZE, nlp_student[2]);

	rewind(bplus_tree);

	ptr = (i + j);
	
	for (i = 0; i < 4; i++) {
		memset(c[0], 0, sizeof(course));
		memset(c[1], 0, sizeof(course));
		memset(c[2], 0, sizeof(course));
		fscanf(course_fp, "%d,%s\n", &(c[0]->id), c[0]->name);
		fscanf(course_fp, "%d,%s\n", &(c[1]->id), c[1]->name);
		fscanf(course_fp, "%d,%s\n", &(c[2]->id), c[2]->name);
		cl = create_course_leaf_page(c);
		write_page(bplus_tree, (ptr + i) * PAGE_SIZE, cl);
		add_leaf(nlp_course->data, i, (ptr + i) * PAGE_SIZE);
	}
	update_parent_of_course_leaf(bplus_tree, nlp_course->data);
	write_page(bplus_tree, (ptr + i) * PAGE_SIZE, nlp_course);
	
	header_table->tables[1].root_page = (ptr + i) * PAGE_SIZE;
	header_table->tables[1].table_id = 2;
	strcpy(header_table->tables[1].table_name, "Course");
	write_page(bplus_tree, 0, header_table_page);
	
	rewind(bplus_tree);
	
	header* memory = (header*)((page_info*)read_page(bplus_tree, 0))->data;

	//printf("root %d %d %s\n", memory->tables[0].root_page, memory->tables[0].table_id, memory->tables[0].table_name);
	//printf("root %d %d %s\n", memory->tables[1].root_page, memory->tables[1].table_id, memory->tables[1].table_name);
	
	printf("hello\n\n");
	/*
	nlp_student_ptr = (non_leaf_page*)((page_info*)read_page(bplus_tree, memory->tables[0].root_page))->data;
	display_non_leaf_page(nlp_student_ptr);
	non_leaf_page* child = nlp_student_ptr->children[0];
	nlp_student_ptr = (non_leaf_page*)((page_info*)read_page(bplus_tree, nlp_student_ptr->children[0]))->data;
	display_non_leaf_page(nlp_student_ptr);
	nlp_student_ptr = (non_leaf_page*)((page_info*)read_page(bplus_tree, child))->data;
	display_non_leaf_page(nlp_student_ptr);
	student* sptr = (student_leaf_page*)((page_info*)read_page(bplus_tree, nlp_student_ptr->children[0]))->data;
	display_student_leaf_page(sptr);*/
	//while (1) {
	//	printf("Enter record to search\n");
	//	scanf("%d", &data);
	//	if (data < 0)
	//		break;
	//	search_record(bplus_tree, data, header_table->tables[0].root_page);
	//}

	find_max_enrollments(bplus_tree);
	select_records_by_id_range(bplus_tree, 41, 400);
	compute_avg_by_id_range(bplus_tree, 41, 500);
	count_records_by_id_range(bplus_tree, 41, 400);
	join_students_courses(bplus_tree);
	fclose(bplus_tree);

	getchar();
	getchar();
	return 0;

}

