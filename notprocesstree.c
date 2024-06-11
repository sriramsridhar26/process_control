#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>

void alrmhandler(){
    kill(getpid(),SIGKILL);

}

void main(){
    
    signal(SIGALRM,alrmhandler);
    printf("\npid %d ppid %d",getpid(),getppid());
    int a =fork();
    int b =fork();
    int c =fork();

    if(a==0 && b==0 && c==0){
        sleep(5);
        exit(1);
    }
    if(a==0 && b>0 && c==0){
        sleep(5);
        exit(1);
    }
    if(a>0 && b==0 && c==0){
        sleep(5);
        exit(1);
        
    }
    alarm(1000);
    while(1){

    }
    
}
 

