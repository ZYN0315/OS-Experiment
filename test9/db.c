#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define APPEND 'A'
#define SELECT 'S'
#define INSERT 'I'
#define DELETE 'D'
#define QUIT 'Q'

int main() {
    char str[] = {'\n', ' ', '\0'};
    char path[] = "st.txt";
    char option;
    int flag = 1;
    int no;
    char id[25];
    char name[25];
    char score[3];
    char result[100];
    char temp1[99999];
    char temp2[99999];
    int i, offset, end;
    int fd = open(path, O_WRONLY | O_CREAT, 0644);
    close(fd);
    while (flag) {
        scanf("%c", &option);
        switch (option) {
            case APPEND:
                fd = open(path, O_APPEND | O_WRONLY);
                printf("ID : ");
                scanf("%s", id);
                for (i = 0; id[i] != '\0'; i++) write(fd, &id[i], 1);
                for (; i < 20; i++) write(fd, &str[1], 1);
                printf("Name : ");
                scanf("%s", name);
                for (i = 0; name[i] != '\0'; i++) write(fd, &name[i], 1);
                for (; i < 20; i++) write(fd, &str[1], 1);
                printf("Score : ");
                scanf("%s", score);
                for (i = 0; score[i] != '\0'; i++) {
                    write(fd, &score[i], 1);
                }
                for (; i < 20; i++) write(fd, &str[1], 1);
                write(fd, &str[0], 1);
                close(fd);
                break;
            case SELECT:
                printf("No : ");
                scanf("%d", &no);
                offset = (no - 1) * 61;
                fd = open(path, O_RDONLY);
                lseek(fd, offset, SEEK_SET);
                int x = read(fd, result, 60);
                result[60] = '\0';
                close(fd);
                printf("No.%d item : %s\n", no, result);
                break;
            case INSERT:
                printf("No : ");
                scanf("%d", &no);
                offset = (no - 1) * 61;
                fd = open(path, O_RDONLY);
                read(fd, temp1, offset);
                lseek(fd, offset, SEEK_SET);
                end = read(fd, temp2, 9999);
                close(fd);

                fd = open(path, O_TRUNC | O_WRONLY);
                write(fd, temp1, offset);
                lseek(fd, offset, SEEK_SET);
                printf("ID : ");
                scanf("%s", id);
                for (i = 0; id[i] != '\0'; i++) write(fd, &id[i], 1);
                for (; i < 20; i++) write(fd, &str[1], 1);
                printf("Name : ");
                scanf("%s", name);
                for (i = 0; name[i] != '\0'; i++) write(fd, &name[i], 1);
                for (; i < 20; i++) write(fd, &str[1], 1);
                printf("Score : ");
                scanf("%s", score);
                for (i = 0; score[i] != '\0'; i++) write(fd, &score[i], 1);
                for (; i < 20; i++) write(fd, &str[1], 1);
                write(fd, &str[0], 1);

                lseek(fd, offset + 61, SEEK_SET);
                write(fd, temp2, end);
                close(fd);
                break;
            case DELETE:
                printf("No : ");
                scanf("%d", &no);
                offset = (no - 1) * 61;
                fd = open(path, O_RDONLY);
                read(fd, temp1, offset);
                lseek(fd, offset + 61, SEEK_SET);
                end = read(fd, temp2, 9999);
                close(fd);
                fd = open(path, O_TRUNC | O_WRONLY);
                write(fd, temp1, offset);
                lseek(fd, offset, SEEK_SET);
                write(fd, temp2, end);
                close(fd);
                break;
            case QUIT:
                flag = 0;
                break;
        }
    }
    return 0;
}
