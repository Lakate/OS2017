#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_SIZE 50
#define MAX_NUMBER 30

/*
Параллельно вычисляется факториал для каждого числа из файла
Они должны быть неотрицательными целыми числами
Для наглядности обработчики дополнительно усыпляю на 5 секунд
*/

long long numbers[MAX_SIZE];
int pipes[MAX_SIZE];
int n_numbers = 0;

int Factorial(long long number) {
    if (number < 0) {
        return -1;
    }else if (number < 2) {
        return 1;
    }
    return (number*Factorial(number-1));
}

void parallel(int input, int output, int index) {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(input, &fds);

    struct timeval t;
    t.tv_sec = 5;
    t.tv_usec = 0;

    int retval;
    retval = select(pipes[index] + 1, &fds, NULL, NULL, &t);
    if (retval == -1) {
        printf("Select error!\n");

        exit(1);
    }

    FILE * f;
    long long number;

    f = fdopen(input, "r");
    fscanf(f, "%lld", &number);
    fclose(f);

    FD_ZERO(&fds);
    FD_SET(output, &fds);
    retval = select(pipes[index] + 1, NULL, &fds, NULL, &t);

    if (retval == -1) {
        printf("Select error!\n");

        exit(1);
    }

    f = fdopen(output, "w");
    fprintf(f, "%d\n", Factorial(number));
    fclose(f);

    sleep(5);
}

void fork_handler(long long number, int index) {
    int fd[2];
    pipe(fd);
    pid_t pid;
    pid = fork();
    FILE *f;

    switch(pid) {
        case -1:
            printf("Fork error\n");
            exit(1);
        case 0:
            parallel(fd[0], fd[1], index);
            exit(0);
        default:
            f = fdopen(fd[1], "w");
            fprintf(f, "%lld\n", number);
            fclose(f);
            pipes[index] = fd[0];
    }
}

void get_numbers(char * file) {
    FILE * f = fopen(file, "r");

    if (f == NULL) {
        printf("Can't open file %s\n", file);

        exit(1);
    }

    while (fscanf(f, "%lld", &numbers[n_numbers]) == 1) {
        if ((numbers[n_numbers])> MAX_NUMBER){
            printf("Too big number: %lld", numbers[n_numbers]);

            exit(1);
        }

        n_numbers++;
    }

    fclose(f);
}


int main(int argc, char * argv[]) {
    if (argc != 2) {
        printf("Wrong number of arguments\n");

        return 1;
    }
    get_numbers(argv[1]);

    int i;
    for (i = 0; i < n_numbers; ++i) {
        fork_handler(numbers[i], i);
    }

    int active = n_numbers;
    while (active) {
        wait(NULL);
        active--;
    }

    int fact;
    long long result;

    for (i = 0; i < n_numbers; ++i) {
        FILE * f = fdopen(pipes[i], "r");
        fscanf(f, "%d", &result);
        fclose(f);

        printf ("Fact(%lld) = %lld\n", numbers[i], result);
    }

    return 0;
}