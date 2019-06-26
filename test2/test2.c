#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int f1(int x) {
    if (x == 1) {
        return 1;
    }
    return f1(x - 1) * x;
}

int f2(int y) {
    if (y == 1 || y == 2) {
        return 1;
    }
    return f2(y - 1) + f2(y - 2);
}

int main() {
    int x, y;
    printf("please input x and y:");
    scanf("%d %d", &x, &y);
    int pid1, pid2;
    int pipe1[2];
    int pipe2[2];
    int pipe3[2];
    int pipe4[2];

    if (pipe(pipe1) < 0) {
        printf("create pipe1 fail\n");
    }
    if (pipe(pipe2) < 0) {
        printf("create pipe2 fail\n");
    }
    if (pipe(pipe3) < 0) {
        printf("create pipe3 fail\n");
    }
    if (pipe(pipe4) < 0) {
        printf("create pipe4 fail\n");
    }

    if ((pid1 = fork()) < 0) {
        perror("fail to create pid1");
    } else if (pid1 == 0) {
        close(pipe1[1]);
        close(pipe2[0]);
        int x1;
        read(pipe1[0], &x1, sizeof(int));
        int z = f1(x1);
        write(pipe2[1], &z, sizeof(int));
        close(pipe1[0]);
        close(pipe2[1]);
        printf("x=%d\n", z);
        exit(EXIT_SUCCESS);
    } else {
        if ((pid2 = fork()) < 0) {
            perror("fail to create pid2");
        } else if (pid2 == 0) {
            close(pipe3[1]);
            close(pipe4[0]);
            int y1;
            read(pipe3[0], &y1, sizeof(int));
            int z = f2(y1);
            write(pipe4[1], &z, sizeof(int));
            close(pipe3[0]);
            close(pipe4[1]);
            printf("y=%d\n", z);
            exit(EXIT_SUCCESS);
        }
        close(pipe1[0]);
        close(pipe2[1]);
        close(pipe3[0]);
        close(pipe4[1]);
        int z;
        write(pipe1[1], &x, sizeof(int));
        write(pipe3[1], &y, sizeof(int));
        read(pipe2[0], &x, sizeof(int));
        read(pipe4[0], &y, sizeof(int));
        z = x + y;
        printf("the result is %d\n", z);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe3[1]);
        close(pipe4[0]);
    }

    return 0;
}