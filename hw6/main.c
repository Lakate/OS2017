#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/file.h>

#define MAX_USERS 100

typedef struct user_info {
    char username[100];
    char password[100];
};

struct user_info users_list[MAX_USERS];

char * concat(char * s1, char * s2) {
    char * result = malloc(strlen(s1)+strlen(s2)+1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

void print_status(char * filename, char * status) {
    char *filelck = concat(filename, ".lck");
    FILE * fd = fopen(filelck, "w");
    fprintf(fd, "%ld %s", (long)getpid(), status);
    fclose(fd);
}

void change_password(char *filename,char *username,
                     char *password) {
    FILE * fd = fopen(filename, "r+");
    if (fd == NULL) {
        printf("Error! File don't exist!\n");
        exit(1);
    }
    flock(fileno(fd), LOCK_EX);

    print_status(filename, "READ");
    int new_user = 0;
    int users_count = 0;
    while(!feof(fd)) {
        char tmp_str[100];
        if(fgets(tmp_str, sizeof(tmp_str), fd)) {
            char *istr;
            char *separators = " \n";
            istr = strtok(tmp_str, separators);
            strcpy(users_list[users_count].username, istr);
            if (strcmp(istr, username) == 0) {
                strcpy(users_list[users_count].password, password);
                new_user = 1;
            } else {
                istr = strtok(NULL, separators);
                strcpy(users_list[users_count].password, istr);
            }
            ++users_count;
        }
    }

    if (new_user == 0) {
        strcpy(users_list[users_count].username, username);
        strcpy(users_list[users_count].password, password);
        ++users_count;
    }

    print_status(filename, "WRITE");
    fd = freopen(NULL, "w+", fd);
    int i;
    for (i = 0; i < users_count; ++i) {
        fprintf(fd, "%s %s\n", users_list[i].username, users_list[i].password);
    }

    flock(fileno(fd), LOCK_UN);
    fclose(fd);
}

void update_user_info(char *filename,
                      char *username, char *password) {
    char *filelck = concat(filename, ".lck");
    while (1) {
        if(access(filelck, F_OK) == -1) {
            break;
        }

        FILE * fp = fopen(filelck, "r");
        long long_pid;
        char operation[10];
        fscanf(fp, "%ld %s", &long_pid, operation);
        fclose(fp);

        printf("%ld, %s. Wait for file unlock...\n", long_pid, operation);
        sleep(1);
    }

    FILE * fd = fopen(filelck, "ab+");
    chmod(filelck, S_IRUSR | S_IWUSR | S_IROTH);
    fclose(fd);

    change_password(filename, username, password);

    remove(filelck);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Error! Wrong params!\n");
        exit(1);
    }

    char * filename = argv[1];
    char * username = argv[2];
    char * password = argv[3];

    update_user_info(filename, username, password);

    return 0;
}