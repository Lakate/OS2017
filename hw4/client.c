#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>

#define PORT 6000
#define HOST "localhost"

//запускается без аргументов при запущенном сервере
char field[225];

int main(int argc, char * argv[]) {
    int sock;
    struct sockaddr_in server_addr;
    struct hostent *server;
    int portno = PORT;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        printf("Socket error\n");
        exit(1);
    }

    server = gethostbyname(HOST);

    if (server == NULL) {
        printf("Connection error\n");
        exit(0);
    }

    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server -> h_addr, (char *)&server_addr.sin_addr.s_addr, server -> h_length);
    server_addr.sin_port = htons(portno);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connection error\n");
        exit(1);
    }
    bzero(field, 225);

    int n;
    if ((n = read(sock, field, 225)) < 0) {
        printf("Socket error\n");
        exit(1);
    }

    int i, j;
    for (i = 0; i < 15; ++i) {
        for (j = 0; j < 15; j++) {
            printf("%c", field[i * 15 + j]);
        }
        printf("\n");
    }

    return 0;
}