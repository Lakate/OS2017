#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/file.h>

/*
после прочтения и после перезаписи ставится sleep на 5 секунд
пока спит запускаем вторую копию программы
вторая копия ждет разблокировки и затем также выполняется
*/

typedef struct user {
    char username[50];
    char pass[50];
};

struct user users[50];

void write_blocktype(char * file, char * blocktype) {
    char * lckfile = malloc(strlen(file)+5);

    strcpy(lckfile, file);
    strcat(lckfile, ".lck");

    FILE * f = fopen(lckfile, "w");
    fprintf(f, "%ld %s", (long)getpid(), blocktype);
    fclose(f);
}

void change_password(char *file,char *username,	char *pass) {
    FILE * f = fopen(file, "r+");
    if (f == NULL) {
        printf("Can't open file %s!\n", file);
        exit(1);
    }

    int sleeptime = 5;
    size_t len;
    char * line = NULL;
    int i = 0;
    int user_flag = 1;
    ssize_t n;
    char * part;

    flock(fileno(f), LOCK_EX);
    write_blocktype(file, "read");
    printf("Reading...\n");

    while ((n = getline(&line, &len, f)) != -1) {
        part = strtok(line, " \n");
        strcpy(users[i].username, part);

        if (strcmp(part, username) == 0) {
            strcpy(users[i].pass, pass);
            user_flag--;
        } else {
            part = strtok(NULL, " \n");
            strcpy(users[i].pass, part);
        }

        i++;
    }

    if (user_flag != 0) {
        strcpy(users[i].username, username);
        strcpy(users[i].pass, pass);
        i++;
    }

    fclose(f);

    sleep(sleeptime);
    write_blocktype(file, "write");
    printf("Writing...\n");

    f = fopen(file, "w+");
    if (f == NULL) {
        printf("Can't open file %s!\n", file);

        exit(1);
    }

    int j;
    for (j = 0; j < i; j++) {
        fprintf(f,"%s %s\n", users[j].username, users[j].pass);
    }

    sleep(sleeptime);
    printf("Finished\n");
    flock(fileno(f), LOCK_UN);
    fclose(f);
}

void add_to_file(char *file, char *username, char *pass) {
    char * lckfile = malloc(strlen(file)+5);
    strcpy(lckfile, file);
    strcat(lckfile, ".lck");

    while (access(lckfile, 0) != -1) {
        FILE * f = fopen(lckfile, "r");
        long pid;
        char operation[5];
        fscanf(f, "%ld %s", &pid, operation);
        fclose(f);
        printf("File blocked. Operation: %s. pid: %ld\n", operation, pid);
        sleep(1);
    }

    FILE * f = fopen(lckfile, "w");
    fclose(f);
    change_password(file, username, pass);
    remove(lckfile);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Wrong number of arguments\n");

        exit(1);
    }

    add_to_file(argv[1], argv[2], argv[3]);

    return 0;
}