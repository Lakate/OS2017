#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>

#define PORT 6000

//для проверки пересчета за секунду можно раскоментировать sleep в next_generation
char field[15][15] = {
        "               ",
        "               ",
        "               ",
        "   111111111   ",
        "    1     1    ",
        "     1   1     ",
        "      1 1      ",
        "       1       ",
        "      1 1      ",
        "     1   1     ",
        "    1     1    ",
        "   111111111   ",
        "               ",
        "               ",
        "               ",
};

void start_server() {
    int sock, accepted_sock;
    int portno = PORT;
    struct sockaddr_in server_addr, client_addr;
    int pid;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        printf("Socket error\n");
        exit(1);
    }
    bzero((char *) &server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(portno);

    if (bind(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        printf("Socket error2\n");
        exit(1);
    }
    listen(sock, 2);

    while (1) {
        int cl = sizeof(client_addr);
        accepted_sock = accept(sock, (struct sockaddr *) &client_addr, &cl);

        if (accepted_sock < 0) {
            printf("Socket error1\n");
            exit(1);
        }

        printf("Connection established\n");
        pid = fork();

        switch (pid){
            case -1:
                printf("Fork error\n");
                exit(1);
            case 0:
                close(sock);
                int n = write(accepted_sock, field, 15 * 15);

                if (n == -1) {
                    printf("Socket error3\n");
                    exit(1);
                }

                exit(0);
            default:
                close(accepted_sock);
        }
    }
}

void next_generation() {
    int x, y, neighbours;
    char new_field[15][15];

    for (x = 0; x < 15; x++) {
        for (y = 0; y < 15; y++) {
            neighbours = count_neighbours(x,y);

            if (neighbours == 3){
                new_field[x][y] = '1';
            } else if (neighbours == 2) {
                new_field[x][y] = field[x][y];
            } else {
                new_field[x][y] = ' ';
            }
        }
    }

//    sleep(5);

    int i,j;

    for (i = 0; i < 15; i++) {
        for (j = 0; j < 15; j++) {
            field[i][j] = new_field[i][j];
        }
    }
}

int recount_generation(){
    pthread_t generation_thread;

    if ((pthread_create(&generation_thread, NULL, &next_generation, NULL)) != 0) {
        printf("Thread error!\n");
        exit(1);
    }

    sleep(1);

    if(pthread_kill(generation_thread, NULL) == 0) {
        return 1;
    }

    return 0;
}

int count_neighbours(int x, int y) {
    int i, j, neighbours = 0;
    int xcoord, ycoord;

    for (i = -1; i < 2; i++) {
        for (j = -1; j < 2; j++) {
            xcoord = x + i;
            ycoord = y + j;
            //printf("%d : %d : %d\n",xcoord, ycoord, field[xcoord][ycoord]);

            if ((i != 0 || j != 0) &&
                (xcoord >= 0) && (xcoord < 15) &&
                (ycoord >= 0) && (ycoord < 15) &&
                (field[xcoord][ycoord] == '1')) {
                neighbours++;
            }
        }
    }

    //printf ("%d\n", neighbours);
    return neighbours;

}



int main(int argc, char * argv[]) {
    pthread_t serv_thread;

    if ((pthread_create(&serv_thread, NULL, &start_server, NULL)) != 0) {
        printf("Thread error!\n");
        exit(1);
    }

    //printf("%d", count_neighbours(4, 4));

    while (1){
        if (recount_generation() == 1){
            printf("New generation has not been found in one second\n");
        }
    }
    return 0;
}