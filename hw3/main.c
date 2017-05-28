#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_SIZE 1000

long ind = 0;
long long numbers[MAX_SIZE];

int read_file(char *file_name) {
	FILE *fd = fopen(file_name, "rt");

    if (fd == NULL) {
        printf("Error! Can't open %s!\n", file_name);
        return 1;
    }

	size_t n;
	char sym;
	long long number = 0;

    while ((n = fread(&sym, 1, 1, fd)) == 1) {
        if (sym >= '0' && sym <= '9') {
			number = (number * 10) + (sym - '0');
		} else {
			if (number != 0) {
				numbers[ind++] = number;
				number = 0;
			}
	    }

		if (ind >= MAX_SIZE) {
			printf("Error! Amount of numbers is greater then expected!\n");
			return 2;
		}
	}

	if (number != 0) {
		numbers[ind++] = number;
	}

	return 0;
}

int compare(const void *a, const void *b) {
	const unsigned long long *x = a, *y = b;

	if(*x > *y) {
		return 1;
	} else {
		return (*x < *y) ? -1 : 0;
	}
}

int write_result(char *file_name) {
	FILE *fp = fopen(file_name, "w");

	if (fp == NULL) {
		printf("Error! Can't open %s!\n", file_name);
		return 1;
	}

	long i;
    for (i = 0; i < ind; ++i) {
		int n = fprintf(fp, "%llu ", numbers[i]);

        if (n <= 0) {
			printf("Error! Can't write in file!\n");
			return 3;
		}
	}

	fclose(fp);

	return 0;
}

int main(int argc, char * argv[]) {
	if (argc == 1) {
		printf("Error! You entered wrong parameters!\n");
		return 1;
	}

	int n;
	int i;
	for (i = 1; i < argc; ++i) {
		if ((n = read_file(argv[i])) != 0) {
			exit(n);
		}
	}

	qsort(numbers, (size_t)ind, sizeof(unsigned long long), compare);

    if ((n = write_result(argv[argc - 1])) != 0) {
		exit(n);
	}

	return 0;
}