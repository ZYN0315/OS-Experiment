#include <stdio.h>
/* Unix header */
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

char buf1[] = "abcdefghij";
char buf2[] = "ABCDEFGHIJ";

int main(void) {
    int fd, size;

    if ((fd = creat("file.txt", S_IRUSR | S_IWUSR)) < 0) {
        printf("creat error\n");
        return -1;
    }

    size = sizeof buf1 - 1;
    if (write(fd, buf1, size) != size) {
        printf("buf1 write error\n");
        return -1;
    }
    /* offset now = 10 */

    if (lseek(fd, 16384, SEEK_SET) == -1) {
        printf("lseek error\n");
        return -1;
    }
    /* offset now = 16384 */

    size = sizeof buf2 - 1;
    if (write(fd, buf2, size) != size) {
        printf("buf2 write error\n");
        return -1;
    }
    /* offset now = 16394 */

    return 0;
}