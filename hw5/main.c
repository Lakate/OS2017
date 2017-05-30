#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>

/*
В действующих конфигурациях запускаем 3 процесса:
	Первый печатает аргументы по разу.
	Второй печатает свои аргументы с возрастающими интервалами и перезапускается.
	Третий печатает свой аргумент спустя секунду и перезапускается.
*/

#define MAXPROC 3
#define MAX_PROG_NAME 50
#define MAX_ARG_LEN 50
#define MAX_ARG_NUM 50

#define CONF_FILE "test.conf"

typedef struct proc {
    char prog[MAX_PROG_NAME];
    char * args[MAX_ARG_NUM];
    int type;
};

struct proc proc_list[MAXPROC];
int proc_count = 0;

pid_t pid_list[MAXPROC];
int pid_count = 0;

int read_config(){
    FILE * f;
    f = fopen(CONF_FILE, "r");

    if (f == NULL) {
        printf ("Unable to open configuration file\n");
        return -1;
    }

    size_t len;
    char * line = NULL;
    char * progarg;
    char * progtype;
    char * part;
    ssize_t n;

    while ((n = getline(&line, &len, f)) != -1) {
        progarg = strtok(line, ",\n");
        progtype = strtok(NULL,", \n");

        if (progtype==NULL) return 1;

        if ((strcmp(progtype,"wait")) == 0) {
            proc_list[proc_count].type = 0;
        } else if ((strcmp(progtype,"restart")) == 0) {
            proc_list[proc_count].type = 1;
        } else {
            printf ("Wrong type at string %d", proc_count);
            fclose(f);

            return 1;
        }


        part = strtok(progarg, " ");
        strcpy(proc_list[proc_count].prog, part);

        int arg_number = 0;
        part = strtok (NULL, " ");

        while (part != NULL) {
            proc_list[proc_count].args[arg_number] = (char*) malloc(MAX_ARG_LEN);
            strcpy(proc_list[proc_count].args[arg_number], part);
            part = strtok (NULL, " ");

            arg_number++;
        }

        proc_count++;
    }


    fclose(f);
    return 0;
}


int launch(int i) {
//    printf("launched\n");
    pid_t pid = fork();
//    printf("%d for %d\n",pid, i);

    switch(pid) {
        case -1:
            printf("Fork error at %d\n", i);
            return -1;
        case 0:
            pid=getpid();

            printf(proc_list[i].prog);
            execv(proc_list[i].prog, proc_list[i].args);

            exit(0);
        default:
            add_to_logfile(proc_list[i].prog, i, pid);
            pid_list[i] = pid;
            pid_count++;

            return 0;
    }
}

int init() {
    int i;

    for (i = 0; i < proc_count; i++) {
        launch(i);
    }

    return 0;
}


char* get_name(char *name) {
    char *substr = strchr(name, '/');

    while (substr != NULL) {
        name = substr + 1;
        substr = strchr(name, '/');
    }

    return name;
}

int add_to_logfile(char *name, int i, int pid) {
    char path[MAX_PROG_NAME];
    name = get_name(name);

    sprintf(path, "/tmp/%s_%d.pid", name, i);
    FILE *f = fopen(path, "w");

    if (f != NULL) {
        fwrite(&pid, sizeof(pid_t), 1, f);
        fwrite("\n", 1, 1, f);
        fclose(f);
    }
}

int delete_logfile(char *name, int i, int pid) {
    char path[MAX_PROG_NAME];
    name = get_name(name);

    sprintf(path, "/tmp/%s_%d.pid", name, i);
    remove(path);
}

int main() {
    pid_t pid;
    int i;

    if ((read_config()) == -1) {
        return 0;
    }

    init();

    while (pid_count) {
        pid = waitpid(-1, NULL, 0);

        for (i = 0; i < proc_count; i++) {
            if (pid_list[i] == pid) {
                pid_list[i] = 0;
                delete_logfile(proc_list[i].prog, i, pid);
                pid_count--;

                if (proc_list[i].type == 1) {
                    launch(i);
                }
            }
        }
    }

    return 0;
}