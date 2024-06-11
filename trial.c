#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int count = 0;

void sighandler(int sig)
{
    if(sig == SIGALRM){
        count = 0;
    }
    else if(sig == SIGINT){
        if(count == 0){
            count = 1;
            alarm(5);
            printf("\nFirst CTRL+C pressed. Press again within 5 seconds to stop the program.\n");
        }
        else{
            printf("\nSecond CTRL+C pressed within 5 seconds. Program will stop now.\n");
            raise(SIGSTOP);
        }
    }
}

int main()
{
    signal(SIGINT, sighandler);
    signal(SIGALRM, sighandler);

    while(1){
        printf("Program is running...\n");
        sleep(1);
    }

    return 0;
}