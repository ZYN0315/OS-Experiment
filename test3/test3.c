#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

struct sched_param p[2];

void int_handler() {
    setpriority(PRIO_PROCESS, getpid(),
                getpriority(PRIO_PROCESS, getpid()) + 1);
}

void cstp_handler() {
    setpriority(PRIO_PROCESS, getpid(),
                getpriority(PRIO_PROCESS, getpid()) - 1);
}

int main(int argc, char *argv[]) {
    int i, n, pid;
    n = atoi(argv[1]);
    struct sched_param p[2];
    for (i = 0; i < 2; i++) p[i].__sched_priority = atoi(argv[i + 2]);
    pid = fork();
   if (pid == 0) {
        signal(SIGINT, (__sighandler_t)SIG_IGN);
        signal(SIGTSTP, (__sighandler_t)cstp_handler);
        sched_setscheduler(getpid(), atoi(argv[5]), &p[1]);
        setpriority(PRIO_PROCESS, getpid(), p[1].__sched_priority);
        for (i = 0; i < n; i++) {
            printf(
                "\nChild process:\n\tpid = %d,\n\tpriority = %d,\n\tscheduler "
                "= "
                "%d\n",
                getpid(), getpriority(PRIO_PROCESS, 0), atoi(argv[5]));
            printf("Press ctrl+z to let priority--.\n");
            pause();
        }
        return EXIT_SUCCESS;
    } else {
        signal(SIGINT, (__sighandler_t)int_handler);
        signal(SIGTSTP, (__sighandler_t)SIG_IGN);
        sched_setscheduler(getpid(), atoi(argv[4]), &p[0]);
        setpriority(PRIO_PROCESS, getpid(), p[0].__sched_priority);
        for (i = 0; i < n; i++) {
            printf(
                "\nParent process:\n\tpid = %d,\n\tpriority = %d,\n\tscheduler "
                "= "
                "%d\n",
                getpid(), getpriority(PRIO_PROCESS, 0), atoi(argv[4]));
            printf("Press ctrl+c to let priority++.\n");
            pause();
        }
    }

    return 0;
}