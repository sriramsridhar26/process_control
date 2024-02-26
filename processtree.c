#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

void main()
{
    int a = fork();
    int b = fork();
    // killing second children altogether
    if (a > 0 && b == 0 )
    {
        wait(NULL);
        exit(1);
    }
    int c = fork();
    int d = fork();
    // killing third children altogether
    if (a > 0 && b > 0 && c > 0 && d == 0)
    {
        wait(NULL);
        exit(1);
    }
    int e = fork();


    

    

    // first child processes will enter here
    if (a == 0)
    {
        if (b > 0)
        {
            if (c == 0)
            {
                // killing childrens of second child
                if (d == 0 || e == 0)
                {
                    exit(1);
                }
                waitpid(d);
                waitpid(e);


                // Second child of first child going to sleep for 5s. After sleep it will be defunct
                printf("\nI am second child of first child\n PID:%d PPID:%d\n", getpid(), getppid());
                fflush(stdout);
                wait(NULL);
                sleep(5);
                exit(1);
            }

            if (c > 0)
            {

                if (d == 0)
                {

                    // killing childrens of third child of first child
                    if (e == 0)
                    {
                        exit(1);
                    }
                    waitpid(e);
                    // Third child of first child going to sleep for 5s. After sleep it will be defunct
                    printf("\nI am third child of first child who will become defunct\n PID:%d PPID:%d\n", getpid(), getppid());
                    fflush(stdout);
                    wait(NULL);
                    sleep(5);
                    exit(1);
                }

                // killing fourth child of first child
                if (d > 0 && e == 0)
                {
                    exit(1);
                }
            }
        }

        if (b == 0)
        {

            if (c == 0 && d == 0 && e == 0)
            {

                // killing super-last-grand child of first child
                exit(1);
            }
            if (c == 0 && d > 0 && e == 0)
            {
                waitpid(d);
                // Second child of first chilf of first child of first child going to sleep for 5s. After sleep it will be defunct
                printf("\nI am second child of first chilf of first child of first child who will become defunct\nPID:%d PPID:%d", getpid(), getppid());
                fflush(stdout);
                wait(NULL);
                sleep(5);
                exit(1);
            }
        }
        while (1)
        {
            // Rest all processes will enter infinite loop
        }
    }

    if (a > 0 && b > 0 && c == 0)
    {
        // killing second child of third child
        if (d > 0 && e == 0)
        {
            exit(1);
        }
        // Rest all processes from third child will enter into infinite loop
        while (1)
        {
        }
    }

    if (a > 0 && b > 0 && c > 0 && d > 0 && e == 0)
    {
        // last child(second child) will exit after 5s but will become defunct because parent is in infinite loop
        printf("\nI am last(second) child who will become defunct\nPID:%d PPID:%d", getpid(), getppid());
        fflush(stdout);
        wait(NULL);
        sleep(5);
        exit(1);
    }
    if(a>0 && b > 0 && c > 0 && d > 0 && e > 0){
        sleep(2);
        printf("\nParent came out of sleep\n");
        waitpid(b);
        waitpid(d);
        // Parent goes into infinite loop
        while(1){

        }
    }
}