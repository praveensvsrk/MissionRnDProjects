#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>

typedef struct {
	char** paths_array;
	int len;
	int maxlen;
}paths;

void print_stairs(int step_count, char *path, int i) {
	if (step_count == 0) {		  
		path[i] = '\0';
		printf("%s\n", path);
		return;
	}
	path[i] = 's';
	print_stairs(step_count - 1, path, i + 1);
	if (step_count - 1 != 0) {
		path[i] = 'd';
		print_stairs(step_count - 2, path, i + 1);
	}
}


char** compute_all_paths(int step_count, int *path_count) {
	if (step_count == 0) {
		*path_count = *path_count + 1;
		return NULL;
	}
	char** single_step = compute_all_paths(step_count - 1, path_count);
	char** double_step;
	if (step_count - 1 != 0)
		double_step = compute_all_paths(step_count - 2, path_count);
	else
		double_step = NULL;

	
	char **res = (char**)malloc(sizeof(char*) * (*path_count));
	char *str = (char*)malloc(sizeof(char) * 10);
	strcpy(str, "s");
	if (single_step != NULL) {
		strcat(str, single_step[*path_count - 1]);
		res[*path_count - 1] = str;
	}
	else if (double_step != NULL) {
		strcpy(str, "d");
		strcat(str, double_step[*path_count - 1]);
		res[*path_count - 1] = str;
	}
	return res;
	

}


void print_subsets(int *array, int n, int pos, int *aux, int index) {
	if (pos == n) {
		for (int i = 0; i < index; i++) {
			printf("%d ", aux[i]);
		}
		printf("\n");
		return;

	}
	aux[index] = array[pos];
	print_subsets(array, n, pos + 1, aux, index + 1);
	print_subsets(array, n, pos + 1, aux, index);
}

void swap(int *arr, int i, int j) {
	int temp = arr[i];
	arr[i] = arr[j];
	arr[j] = temp;
}

void permutations(int *arr, int index, int len){
	if (len == index) {
		for (int i = 0; i < len; i++) {
			printf("%d ", arr[index]);
			printf("\n");
		}
	}
	for (int i = 0; i < len; i++) {
		swap(arr, i, index);
		permutations(arr, index + 1, len);
		swap(arr, i, index);
	}
}

int main() {

	int steps;
	char* path;
	int *array = (int*)malloc(sizeof(int) * 4);
	int *aux = (int*)malloc(sizeof(int) * 4);
	array[0] = 1;
	array[1] = 2;
	array[2] = 3;
	array[3] = 4;

	while (1) {
		printf("Enter steps: ");
		scanf("%d", &steps);
		//path = (char*)malloc(sizeof(char) * (steps + 1));
		//for (int i = 0; i < steps; i++) {
		//	path[i] = '\0';
		//}
		//print_stairs(steps, path, 0);
		//int path = 0;
		//char** res = compute_all_paths(steps, &path);

	}
	//print_subsets(array, 4, 0, aux, 0);

	//parantheses possibilities

}