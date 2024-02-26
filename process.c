#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


void main(){
    printf("\nprocess id of parent: %d\n",getpid());
    int t = fork();

    if(t==0){
        printf("\n direct child process id: %d\n",getpid());
        int g = fork();
        if(g==0){
            printf("going to sleep\n");
            sleep(90);
        }
    }
    if(t>0){
        
        int e = fork();
        if(e==0){
            printf("\n direct child process id: %d\n",getpid());
            sleep(90);

        }

    }
    int a = waitpid(t);
}