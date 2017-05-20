#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

const int END_BYTE = -1;

const unsigned int BUFFER_SIZE = 0x800;

int unzip_sparse(char * f_name) {
    int file_des = open(f_name,  O_WRONLY | O_TRUNC | O_CREAT , S_IRWXU);
    int n;
    char buffer[BUFFER_SIZE];

    if (file_des == -1) {
        printf("Error! Can't open or create file: %s", f_name);
        return 1;
    }

    while ((n = read(0, buffer, BUFFER_SIZE)) != 0) {
        int i;
        int skip = 0;

        for (i = 0; i < n; ++i) {
            if (buffer[i] == 0) {
                ++skip;
            } else {
                if (skip != 0) {
                    lseek (file_des, skip, SEEK_CUR);
                }

                write(file_des, &buffer[i], 1);
                skip = 0;
            }
        }

        if (skip != 0) {
            lseek (file_des, skip, SEEK_CUR);
        }
    }

    write(file_des, &END_BYTE, 1);
    close(file_des);

    return 0;
}

int main(int argc, char * argv[]) {
    if (argc == 1) {
        printf("Error! You have to entered file name!");
    } else {
        return unzip_sparse(argv[2]);
    }
}