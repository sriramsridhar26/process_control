//
//  maybeas2part2.c
//  ASP
//
//  Created by Anup Suguru Veeriah on 22/02/2024
//
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

struct procDetails
{
    int procId;
    int parentId;
    int groupId;
    int currStatus;
    int *childPids;
    int childCount;
};
int gblprocid = 0;
int defuncCount = 0;

// This function returns stat path constructed with procid
char *scaffoldstatpath(char *procid)
{
    char *temp = malloc((11 + strlen(procid)) * sizeof(char));
    memset(temp, " ", 11 + strlen(procid));
    strcpy(temp, "/proc/");
    strcat(temp, procid);
    strcat(temp, "/stat");

    return temp;
}

// This function returns children path constructed with procid
char *scaffoldchildpath(char *procid)
{
    char *temp = malloc((20 + (2 * strlen(procid)) * sizeof(char)));
    memset(temp, " ", 20 + (2 * strlen(procid)));
    strcpy(temp, "/proc/");
    strcat(temp, procid);
    strcat(temp, "/task/");
    strcat(temp, procid);
    strcat(temp, "/children");
    return temp;
}

// This function returns pointer to a struct with process details
// Process details include: pid, ppid, pgid, status, childpids, childcount
int populateProcDetails(char *statPath, struct procDetails *ptr)
{

    // Open stat file and read 100 characters. Within 100 characters, we'll get whatever we need
    int statFd = open(statPath, O_RDONLY);
    char *buff = malloc(100 * sizeof(char));
    memset(buff, ' ', 100);
    int bytesRead = read(statFd, buff, 100);

    // Exit if read failed
    if (bytesRead <= 0)
    {
        return 0;
    }

    int count = 0;

    // Allocate memory for a temporary char pointer variable and copy the content of buff to this variable
    char *strspTemp = malloc(strlen(buff) * sizeof(char));
    memset(strspTemp, " ", strlen(buff));
    strcpy(strspTemp, buff);
    char *tmp;

    // In an infinite loop separate numeric values(pid, ppid, pgid) from the strspTemp delimited by space and store it into appropriate
    // variables based on the iteration count
    for (;;)
    {
        tmp = strsep(&strspTemp, " ");
        if (tmp == NULL)
        {
            break;
        }
        if (isdigit(*tmp))
        {
            switch (count)
            {
            case 0:
                ptr->procId = atoi(tmp);
                count += 1;
                break;
            case 1:
                ptr->parentId = atoi(tmp);
                count += 1;
                break;
            case 2:
                ptr->groupId = atoi(tmp);
                count += 1;
                break;
            default:
                continue;
            }
        }
    }
    // Clear the variable and copy the values again
    free(strspTemp);

    char *strsTemp = malloc(strlen(buff) * sizeof(char));
    memset(strsTemp, " ", strlen(buff));

    // *strsTemp = *buff;
    strcpy(strsTemp, buff);

    // In an infinite loop separate character values(state) from the strspTemp delimited by space and assign a value for the state
    for (;;)
    {

        tmp = strsep(&strsTemp, " ");

        if (tmp == NULL)
        {
            break;
        }
        if (isalpha(*tmp))
        {
            switch (*tmp)
            {
            case 'R':
                ptr->currStatus = 1;
                break;
            case 'Z':
                ptr->currStatus = 2;
                break;
            case 'S':
                ptr->currStatus = 3;
                break;
            default:
                continue;
            }
        }
    }

    // Free the memory occupied by variables
    free(strsTemp);
    free(buff);
    free(statPath);

    // Close the stat file
    close(statFd);
    return 1;
}

int populateChildDetails(char *childPath, struct procDetails *ptr)
{
    // Open the child file and read 100 bytes
    int childFd = open(childPath, O_RDONLY);
    char *buff = malloc(100 * sizeof(char));
    memset(buff, " ", 100);
    int bytesRead = read(childFd, buff, 100);

    // If read failed return 1, if file empty then set childcount as 0 and return 1
    if (bytesRead == 0)
    {
        ptr->childCount = 0;
        return 1;
    }
    if (bytesRead < 0)
    {
        return 0;
    }

    ptr->childCount = 0;

    ptr->childPids = malloc(1 * sizeof(int));

    // In an infinite loop, iterate through every child pids retrieved from children file and insert into childpid array in struct
    for (;;)
    {
        char *tmp = strsep(&buff, " ");

        if (tmp == NULL)
        {
            break;
        }

        if (isdigit(*tmp))
        {
            void *realcStat = realloc(ptr->childPids, ((ptr->childCount + 1) * sizeof(int)));
            if (realcStat == NULL)
            {
                printf("\nInserting child pids failed\n");
                return 0;
            }
            *(ptr->childPids + ptr->childCount) = atoi(tmp);
            ptr->childCount += 1;
        }
    }
    // Close the file and free the memory
    close(childFd);
    free(buff);
    free(childPath);
    return 1;
}
// Returns pointer to a struct variable that contains pid, parentId, groupId, childrens and number of childrens
int parseStat(char *procid, struct procDetails *ptr)
{
    // Check if process exist
    if (kill(atoi(procid), 0) == -1)
    {
        printf("\nInvalid process id %s\n", procid);
        return 0;
    }
    // Populate status information by accessing stat file
    if (populateProcDetails(scaffoldstatpath(procid), ptr) != 1)
    {
        printf("\nPopulating proc details failed\n");
        return 0;
    }
    // Populate children information by accessing children file
    if (populateChildDetails(scaffoldchildpath(procid), ptr) != 1)
    {
        // printf("\n%s\n",procid);
        printf("\nPopulating child details failed\n");
        return 0;
    }

    return 1;
}

// Prints appropriate output and returns 1 if process accessing and state modification operation succeeds or returns 0 after printing error message
int procLevelOp(struct procDetails *currproc, struct procDetails *pproc, int type)
{
    // If process group id does not match with root process' and process group id does not match with process' id
    // then that process does not exist in hierarchy
    if ((currproc->groupId != pproc->groupId) && (currproc->groupId != currproc->procId))
    {
        printf("\nProcess does not exist in hierarchy or error\n");
        return 0;
    }

    if (type == 0)
    {
        // Print process id and parent process id
        printf("\n%d %d\n", currproc->procId, currproc->parentId);
        return 1;
    }
    else if (type == 1)
    {
        // Kill the process with SIGKILL
        kill(currproc->procId, SIGKILL);
        printf("\nProcess: %d killed using SIGKILL\n", currproc->procId);
        return 1;
    }
    else if (type == 2)
    {
        // Kill the root process with SIGKILL
        kill(currproc->groupId, SIGKILL);
        printf("\nRoot process: %d killed using SIGKILL\n", pproc->procId);
        return 1;
    }
    else if (type == 3)
    {
        // STOP the process with SIGSTOP
        kill(currproc->procId, SIGSTOP);
        printf("\nProcess: %d stopped using SIGSTOP\n", currproc->procId);
        return 1;
    }
    else if (type == 4)
    {
        // Resume the process SIGCONT
        kill(currproc->procId, SIGCONT);
        printf("\nProcess: %d resumed successfully\n", currproc->procId);
        return 1;
    }
    else if (type == 5)
    {
        // Display current status of process
        if (currproc->currStatus == 2)
        {
            printf("\nProcess id: %d is defunct\n", currproc->procId);
            return 1;
        }
        else if (currproc->currStatus == 3)
        {
            printf("\nProcess id: %d is sleeping\n", currproc->procId);
            return 1;
        }
        printf("\nProcess id: %d is running", currproc->procId);
        return 1;
    }
    if (type == 6 || type == 9)
    {
        // If there's no childPids then exit by return 0
        if (currproc->childCount == 0)
        {
            return 0;
        }
        int k = 0;
        // Iterate through the childrens
        while (k < currproc->childCount)
        {
            // Construct child pid string and populate struct for that
            struct procDetails *childTemp = malloc(sizeof(struct procDetails));
            char childidstr[10];
            sprintf(childidstr, "%d", *(currproc->childPids + k));
            int t = parseStat(childidstr, childTemp);
            if (t <= 0)
            {
                printf("\nError creating child proc\n");
            }
            // Ignore the proccess id if parent id is same as the process id given by user
            if (type == 6)
            {
                if (!(childTemp->parentId == gblprocid))
                {
                    printf("%d ", childTemp->procId);
                    fflush(stdout);
                }
            }
            // Print the process id if its defunct
            if (type == 9)
            {
                if (childTemp->currStatus == 2)
                {
                    printf("%d ", childTemp->procId);
                    fflush(stdout);
                    defuncCount++;
                }
            }
            procLevelOp(childTemp, currproc, type);
            // Clear memory
            free(childTemp);
            k++;
        }
        if(type==9 && defuncCount==0){
            printf("\nNo defunct processes found");
        }

        return 1;
    }
    else if (type == 7)
    {
        // Print child pids by iterating through the childPids array, exit if childcount is zero
        if (currproc->childCount == 0)
        {
            printf("\nNo direct descendants\n");
            return 0;
        }
        int temp = 0;
        while (temp < currproc->childCount)
        {
            printf("%d ", *(currproc->childPids + temp));
            temp++;
        }

        return 1;
    }
    else if (type == 8)
    {
        // If root process is not same as parent process
        if (pproc->procId != currproc->parentId)
        {
            // Populate struct for parent id
            struct procDetails *parentDts = malloc(sizeof(struct procDetails));
            char parentidstr[10];
            sprintf(parentidstr, "%d", currproc->parentId);
            int t = parseStat(parentidstr, parentDts);

            if (t <= 0)
            {
                printf("\nPopulating parent struct failed\n");
                return 0;
            }

            // Check childcount, iterate through the list and display the children which ppid is not same as process id
            // given by user
            if (parentDts->childCount == 1)
            {
                printf("\nNo siblings\n");
                return 1;
            }
            int k = 0;
            while (k < parentDts->childCount)
            {
                if (currproc->procId == *(parentDts->childPids + k))
                {
                    k++;
                    continue;
                }
                if (*(parentDts->childPids + k) == 0)
                {
                    k++;
                    continue;
                }
                printf("%d ", *(parentDts->childPids + k));
                k++;
            }

            // Clear the memory
            free(parentDts);
            return 1;
        }

        // If rootprocess is same as parent process then the same operation will be performed but with rootproc struct

        // Check childcount, iterate through the list and display the children which ppid is not same as process id
        // given by user
        if (pproc->childCount == 1)
        {
            printf("\nNo siblings\n");
            return 1;
        }
        // Traverse through the list and ignore the pid which we gave
        int k = 0;
        while (k < pproc->childCount)
        {
            if (currproc->procId == *(pproc->childPids + k))
            {
                k++;
                continue;
            }
            if ((*(pproc->childPids + k) == 0))
            {
                k++;
                continue;
            }
            printf("%d ", *(pproc->childPids + k));
            k++;
        }

        return 1;
    }
    else if (type == 10)
    {

        if (currproc->childCount == 0)
        {
            return 0;
        }

        int i = 0;
        while (i < currproc->childCount)
        {
            // Iterate through child pids and populate struct for child pid
            struct procDetails *childTemp = malloc(sizeof(struct procDetails));
            char childidstr[10];
            sprintf(childidstr, "%d", *(currproc->childPids + i));
            int t = parseStat(childidstr, childTemp);
            if (t <= 0)
            {
                printf("\nError creating child proc\n");
            }

            // print childpids of child if exist
            if (childTemp->childCount == 0)
            {
                i++;
                continue;
            }
            int k = 0;

            while (k < childTemp->childCount)
            {
                printf("%d ", *(childTemp->childPids + k));
                k++;
            }
            free(childTemp);
            i++;
        }
        return 1;
    }
}

// Returns an appropriate integer value for the argument
int typecode(char *type)
{

    if (strcmp(type, "-rp") == 0)
    {
        return 1;
    }
    else if (strcmp(type, "-pr") == 0)
    {
        return 2;
    }
    else if (strcmp(type, "-xt") == 0)
    {
        return 3;
    }
    else if (strcmp(type, "-xc") == 0)
    {
        return 4;
    }
    else if (strcmp(type, "-zs") == 0)
    {
        return 5;
    }
    else if (strcmp(type, "-xn") == 0)
    {
        return 6;
    }
    else if (strcmp(type, "-xd") == 0)
    {
        return 7;
    }
    else if (strcmp(type, "-xs") == 0)
    {
        return 8;
    }
    else if (strcmp(type, "-xz") == 0)
    {
        return 9;
    }
    else if (strcmp(type, "-xg") == 0)
    {
        return 10;
    }
    return 0;
}

void main(int argc, char *argv[])
{
    int opstat = 0;
    // Populate struct for process id and root/parent id
    struct procDetails *proc = malloc(sizeof(struct procDetails));

    // int a = parseStat(argv[1], proc);
    // printf("\nproc id: %d\n", proc->procId);
    // fflush(stdout);
    // printf("\npproc id: %d\n", proc->parentId);
    // fflush(stdout);
    // printf("\nroot id: %d\n", proc->groupId);
    // fflush(stdout);
    // printf("\nstat: %d\n", proc->currStatus);
    // fflush(stdout);
    // printf("\nchildsize: %d\n", proc->childCount);
    // fflush(stdout);
    // for (int i = 0; i < proc->childCount; i++)
    // {
    //     printf("%d ", *(proc->childPids + i));
    // }
    // exit(0);
    if (parseStat(argv[1], proc) == 0)
    {
        exit(0);
    }
    struct procDetails *pproc = malloc(sizeof(struct procDetails));
    if (parseStat(argv[2], pproc) == 0)
    {
        exit(0);
    }

    // For the appropriate option(-rp, pr, etc.), procLevelOp function will be invoked with appropriate typecode
    if (argc == 3)
    {
        opstat = procLevelOp(proc, pproc, 0);
    }
    if (argc == 4)
    {
        if (typecode(argv[3]) == 6)
        {
            gblprocid = proc->procId;
        }
        opstat = procLevelOp(proc, pproc, typecode(argv[3]));
        printf("\n");
    }

    // CLear memory
    free(proc);
    free(pproc);

    // Print unsuccessful if operation failed
    if (opstat == 0)
    {
        printf("\nOperation Unsuccessful\n");
        fflush(stdout);
    }
}