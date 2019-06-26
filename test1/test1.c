#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <wait.h> 

int main(){
    int i;
    int pid;
    int status;
    char *args[] = {"/bin/ls","-a",NULL};
    while (1)
    {
        pid = fork();
        if(pid<0){
            printf("fail to create process\n");
            exit(EXIT_FAILURE);
        }
        else if (pid==0)
        {
            status = execve(args[0],args,NULL);
        }
        sleep(3);
    }
    

    return 0;
}