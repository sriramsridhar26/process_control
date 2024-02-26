#define _GNU_SOURCE
#define _OPEN_SYS_ITOA_EXT
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>

struct status
{
    int pid;
    int ppid;
    int rootpid;
    int state;
    int *children;
    int childsize;
};
int fs_procid = 0;

int checkdirexists(char *filename)
{
    struct stat sb;

    if (stat(filename, &sb) == 0 && S_ISDIR(sb.st_mode))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// DEPRECATED
struct status *parseStatus(char *filename)
{
    struct status *ptr = malloc(sizeof(struct status));
    int fd = open(filename, O_RDONLY);
    // printf("\nfd stat: %d\n", fd);
    char buff[50];

    int d = read(fd, &buff, 50);
    int count = 0;
    char *token = strtok(buff, " ");
    while (token != NULL)
    {
        if (isdigit(token[0]))
        {
            if (count == 0)
            {
                ptr->pid = atoi(token);
                count++;
            }
            else if (count == 1)
            {
                ptr->ppid = atoi(token);
                count++;
            }
            else if (count == 2)
            {
                ptr->rootpid = atoi(token);
                count++;
            }
        }
        if (isalpha(token[0]))
        {

            if (strcmp(token, "R") == 0)
            {
                ptr->state = 1;
            }
            if (strcmp(token, "Z") == 0)
            {
                ptr->state = 2;
            }
        }
        token = strtok(NULL, " ");
    }

    // printf("\n%s\n",buff);

    // printf("\ncount: %d\n", count);

    return ptr;

    // free(buff);
}

struct status *parseStat(char *procid)
{
    char *statpath = malloc((6 + strlen(procid)) * sizeof(char));

    strcpy(statpath, "/proc/");
    strcat(statpath, procid);
    if (!checkdirexists(statpath))
    {
        printf("\nProcess process does not exist\n");
        return 0;
    }
    char *childpath = malloc((20 + (2 * strlen(procid)) * sizeof(char)));
    strcpy(childpath, statpath);
    strcat(childpath, "/task/");
    strcat(childpath, procid);
    strcat(childpath, "/children");
    int rlcstat = realloc(statpath, ((11 + strlen(procid)) * sizeof(char)));
    strcat(statpath, "/stat");

    struct status *ptr = malloc(sizeof(struct status));
    int fd = open(statpath, O_RDONLY);
    // printf("\nfd stat: %d\n", fd);
    char buff[100];

    int d = read(fd, &buff, 50);
    int count = 0;
    char *token = strtok(buff, " ");
    while (token != NULL)
    {
        if (isdigit(token[0]))
        {
            if (count == 0)
            {
                ptr->pid = atoi(token);
                count++;
            }
            else if (count == 1)
            {
                ptr->ppid = atoi(token);
                count++;
            }
            else if (count == 2)
            {
                ptr->rootpid = atoi(token);
                count++;
            }
        }
        if (isalpha(token[0]))
        {

            if (strcmp(token, "R") == 0)
            {
                ptr->state = 1;
            }
            if (strcmp(token, "Z") == 0)
            {
                ptr->state = 2;
            }
        }
        token = strtok(NULL, " ");
    }

    close(fd);
    fd = open(childpath, O_RDONLY);
    // printf("\nchildpath fd: %d",)
    d = read(fd, &buff, 100);
    int countchil = 0;
    token = strtok(buff, " ");
    // printf("\nchildpath read bytes: %d",d);
    if (d > 0)
    {
        ptr->children = malloc(((countchil + 1) * sizeof(int)));
        while (token != NULL)
        {
            if (isdigit(token[0]))
            {
                // printf("\nproblemo here\n");
                // fflush(stdout);
                int statin = realloc(ptr->children, ((countchil + 1) * sizeof(int)));
                *(ptr->children + countchil) = atoi(token);
                countchil++;
            }
            token = strtok(NULL, " ");
        }
    }
    ptr->childsize = countchil;
    close(fd);
    free(childpath);
    free(statpath);

    // printf("\n%s\n",buff);

    // printf("\ncount: %d\n", count);

    return ptr;

    // free(buff);
}

// DEPRECATED
struct status *statRetrieve(char *procid)
{
    char *procpath = malloc((6 + strlen(procid)) * sizeof(char));
    strcpy(procpath, "/proc/");
    strcat(procpath, procid);
    if (!checkdirexists(procpath))
    {
        printf("\nProcess process does not exist\n");
        return 0;
    }
    int rlcstat = realloc(procpath, ((11 + strlen(procid)) * sizeof(char)));
    strcat(procpath, "/stat");
    struct status *procid_stat = parseStatus(procpath);
    return procid_stat;
}

int procLevelOp(struct status *proc, struct status *rootproc, int type)
{
    if (proc->rootpid == rootproc->pid)
    {
        if (type == 0)
        {
            printf("\n%d %d\n", proc->pid, proc->ppid);
            return 1;
        }
        else if (type == 1)
        {
            if (kill(proc->pid, SIGKILL) < 0)
            {
                printf("\nKilling process failed\n");
                return 0;
            }
            printf("\nProcess killed successfully\n");
            return 1;
        }
        else if (type == 2)
        {
            printf("\nThis doesn't guarantee that any defunct process from root process will be killed\n");
            if (kill(rootproc->pid, SIGKILL) < 0)
            {
                printf("\nKilling root process failed\n");
                return 0;
            }
            printf("\nRoot process killed successfully\n");
            return 1;
        }
        else if (type == 3)
        {
            if (kill(proc->pid, SIGSTOP) < 0)
            {
                printf("\nSuspending process failed\n");
                return 0;
            }
            printf("\nProcess suspended successfully\n");
            return 1;
        }
        else if (type == 4)
        {
            if (kill(proc->pid, SIGCONT) < 0)
            {
                printf("\nSIGCONT process failed\n");
                return 0;
            }
            printf("\nProcess resumed successfully\n");
            return 1;
        }
        else if (type == 5)
        {
            if (proc->state > 1)
            {
                printf("\nProcess is defunct\n");
                return 1;
            }
            printf("\nProcess is still running");
        }
    }
    else
    {
        printf("\nProcess does not exist in hierarchy or error\n");
        return 0;
    }
}

int listprocessop(struct status *proc, struct status *rootproc, int type)
{
    // printf("\nproc->childsize: %d\n",proc->childsize);
    // if (proc->childsize == 0 || proc->pid==rootproc->pid)
    if (type == 1 || type==4 )
    {
        if (proc->childsize == 0)
        {
            return 0;
        }
        for (int i = 0; i < proc->childsize; i++)
        {

            if (*(proc->children + i) == 0)
            {
                return 0;
            }
            char child[10];
            sprintf(child, "%d", *(proc->children + i));
            struct status *childst = parseStat(child);

            if ( type == 1 && (!(childst->ppid == fs_procid)))
            {
                printf("%d ", childst->pid);
                fflush(stdout);
                // char child[10];
            }
            if(type == 4 && (childst->state==2)){
                printf("%d ", childst->pid);
                fflush(stdout);
            }
            // if(type==5){
            //     if(childst->ppid==fs_procid){
            //         fs_procid = childst->pid;
            //     }
            // }


            listprocessop(childst, rootproc, type);
            // free(childst);
        }
        // free(rootproc);
        // printf("\n");
        return 1;
    }
    else if (type == 2)
    {
        if (proc->childsize == 0)
        {
            printf("\nNo children in this process\n");
            return 0;
        }
        for (int i = 0; i < proc->childsize; i++)
        {
            if (*(proc->children + i) == 0)
            {
                continue;
            }
            printf("%d ", *(proc->children + i));
        }
       printf("\n");
    }
    else if (type == 3)
    {
        char ppidstr[10];
        sprintf(ppidstr, "%d", proc->ppid);
        struct status *ptproc = malloc(sizeof(struct status));
        ptproc = parseStat(ppidstr);
        for (int i = 0; i < ptproc->childsize; i++)
        {
            if ((*(ptproc->children + i) == 0) || *(ptproc->children+i)==proc->pid)
            {
                continue;
            }
            printf("%d ", *(ptproc->children + i));
        }
        free(ptproc);
        printf("\n");
        return 1;
    }
    else if (type == 5){
        if (proc->childsize == 0)
        {
            return 0;
        }
        for (int i = 0; i < proc->childsize; i++)
        {

            if (*(proc->children + i) == 0)
            {
                continue;
            }
            char child[10];
            sprintf(child, "%d", *(proc->children + i));
            struct status *childst = parseStat(child);
            if(childst->childsize==0){
                continue;
            }
            for(int j =0 ;j<childst->childsize;j++){
                printf("%d ",*(childst->children+j));
            }
            free(childst);
        }
        return 1;
    }
    // fflush(stdout);
    return 0;
}

// DEPRECATED
//  int checkprocesstree(char *procid, char *rootprocid)
//  {
//      char *rootpath = malloc((6 + strlen(rootprocid)) * sizeof(char));
//      char *procpath = malloc((6 + strlen(procid)) * sizeof(char));
//      strcpy(procpath, "/proc/");
//      strcpy(rootpath, "/proc/");
//      strcat(rootpath, rootprocid);
//      strcat(procpath, procid);
//      if (!checkdirexists(rootpath))
//      {
//          printf("\nRoot process does not exist\n");
//          return 0;
//      }
//      if (!checkdirexists(procpath))
//      {
//          printf("\nProcess process does not exist\n");
//          return 0;
//      }
//      int rlcstat = realloc(procpath, ((11 + strlen(procid)) * sizeof(char)));
//      strcat(procpath, "/stat");
//      // printf("\n%s\n", procpath);
//      // fflush(stdout);
//      struct status *procid_stat = parseStatus(procpath);
//      // printf("\nproc id: %d\n", procid_stat->pid);
//      // printf("\npproc id: %d\n", procid_stat->ppid);
//      // printf("\nroot id: %d\n", procid_stat->rootpid);
//      // printf("\nstat: %d\n", procid_stat->state);
//      if (procid_stat->rootpid == atoi(rootprocid))
//      {
//          return 1;
//      }
//      free(procpath);
//      free(rootpath);
//      free(procid_stat);
//      return 0;
//  }

void main(int argc, char *argv[])
{
    // struct status *procid_stat = parseStat(argv[1]);
    // printf("\nproc id: %d\n", procid_stat->pid);
    // printf("\npproc id: %d\n", procid_stat->ppid);
    // printf("\nroot id: %d\n", procid_stat->rootpid);
    // printf("\nstat: %d\n", procid_stat->state);
    // printf("\nchildsize: %d\n",procid_stat->childsize);
    // for(int i = 0 ;i<procid_stat->childsize;i++){
    //     printf("%d ",*(procid_stat->children+i));
    // }

    if (argc == 4)
    {
        if (strcmp(argv[3], "-rp") == 0)
        {
            printf("\nEntering rp op\n");
            int stat = procLevelOp(parseStat(argv[1]), parseStat(argv[2]), 1);
        }
        else if (strcmp(argv[3], "-pr") == 0)
        {
            printf("\nEntering pr op\n");
            int stat = procLevelOp(parseStat(argv[1]), parseStat(argv[2]), 2);
        }
        else if (strcmp(argv[3], "-xt") == 0)
        {
            printf("\nEntering xt op\n");
            int stat = procLevelOp(parseStat(argv[1]), parseStat(argv[2]), 3);
        }
        else if (strcmp(argv[3], "-xc") == 0)
        {
            printf("\nEntering xc op\n");
            int stat = procLevelOp(parseStat(argv[1]), parseStat(argv[2]), 4);
        }
        else if (strcmp(argv[3], "-zs") == 0)
        {
            printf("\nEntering zs op\n");
            // printf("\n Entering zs");
            int stat = procLevelOp(parseStat(argv[1]), parseStat(argv[2]), 5);
        }
        else if (strcmp(argv[3], "-xn") == 0)
        {
            printf("\nEntering xn op\n");
            fs_procid = atoi(argv[1]);
            int stat = listprocessop(parseStat(argv[1]), parseStat(argv[2]), 1);
            printf("\n");
        }
        else if (strcmp(argv[3], "-xd") == 0)
        {
            printf("\nEntering xd op\n");
            int stat = listprocessop(parseStat(argv[1]), parseStat(argv[2]), 2);
        }
        else if (strcmp(argv[3], "-xs") == 0)
        {
            printf("\nEntering xs op\n");
            int stat = listprocessop(parseStat(argv[1]), parseStat(argv[2]), 3);
        }
        else if (strcmp(argv[3], "-xz") == 0)
        {
            printf("\nEntering xz op\n");
            int stat = listprocessop(parseStat(argv[1]), parseStat(argv[2]), 4);
            printf("\n");
        }
        else if (strcmp(argv[3], "-xg") == 0)
        {
            printf("\nEntering xg op\n");
            int stat = listprocessop(parseStat(argv[1]), parseStat(argv[2]), 5);
            printf("\n");
        }
    }
}

// void main()
// {
//     // parseStatus("/proc/95485/stat");
//     int checkprocstat = checkprocesstree(119824,119818 );
// }