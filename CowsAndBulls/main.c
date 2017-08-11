#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int countlines(FILE *fp) {
	char* word = (char*)malloc(sizeof(char) * 100);
	int count = 0;
	while (fscanf(fp, "%s\n", word) != -1) {
		count++;
	}
	return count;
}

int strlen(char* str) {
	int i = 0;
	while (str[i] != '\0')
		i++;
	return i;
}

void lower(char* str) {
	int len = strlen(str);
	int i;
	for (i = 0; i < len; i++) {
		if (str[i] >= 'A' && str[i] <= 'Z') {
			str[i] = str[i] + 32;
		}
		else
			str[i] = str[i];
	}
}

int is_valid(char* str) {
	int len = strlen(str);
	for (int i = 0; i < len; i++) {
		if (!((str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z')))
			return 0;
	}
	return 1;
}

char* get_random_word() {
	FILE *fp;
	fp = fopen("words.txt", "r");

	int lines = countlines(fp);
	fseek(fp, 0, SEEK_SET);
	char* word = (char*)malloc(sizeof(char) * 100);
	int iterations = rand() % lines;

	while (iterations--) {
		fscanf(fp, "%s\n", word);
	}
	if (!is_valid(word)) {
		printf("Dictionary has invalid words\n");
		return NULL;
	}
	fclose(fp);
	lower(word);
	return word;
}


int strpos(char* str, char ch) {
	int len = strlen(str);
	for (int i = 0; i < len; i++) {
		if (ch == str[i])
			return i;
	}
	return -1;

}

void populate_hash(int* hash, char* word) {
	int len = strlen(word);
	for (int i = 0; i < len; i++) {
		hash[word[i] - 'a'] = i;
	}
}

int* count_cows_and_bulls(char* user_word, char* dict_word) {
	int len = strlen(user_word);
	int *cows_and_bulls = (int*)malloc(2 * sizeof(int));
	int letter_hash[26];
	for (int i = 0; i < 26; i++) {
		letter_hash[i] = -1;
	}
	populate_hash(letter_hash, dict_word);
	cows_and_bulls[0] = 0;
	cows_and_bulls[1] = 0;
	for (int i = 0; i < len; i++) {
		if (letter_hash[user] != -1)
			cows_and_bulls[1] ++;
		else if (letter_hash[i] == strpos(user_word, dict_word[letter_hash[i]]))
			cows_and_bulls[0] ++;
	}
	return cows_and_bulls;
}


int main() {
	char* word = get_random_word();
	char* user_word = (char*)malloc(sizeof(char) * 100);
	int len = strlen(word);
	printf("The word you need to guess is of length %d\n", len);
	while (1) {
		printf("Enter a guess:");
		scanf("%s", user_word);
		lower(user_word);
		int* cows_and_bulls = count_cows_and_bulls(user_word, word);
		if (cows_and_bulls[0] == strlen(user_word)) {
			printf("You guessed it right\n");
			break;
		}		

		else {
			printf("Cows : %d\nBulls: %d\n", cows_and_bulls[0], cows_and_bulls[1]);
			printf("Try again\n");
		}
	}

	getchar();
	getchar();
	return 0;
}