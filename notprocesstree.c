#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

void main(){
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


    while(1){

    }
}