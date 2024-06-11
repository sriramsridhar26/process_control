/*
  @author: Sriram Sridhar
  @StudentId: 110126034
  filename: a2prc_sriram_sridhar_110126034.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
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
    int pgid;
    int state;
    int *children;
    int childsize;
};
int dcount = 0;
int fs_procid = 0;

// Return bytesize if file exist
int filebytesize(char *filename)
{
    struct stat sb;

    if (stat(filename, &sb) == 0)
    {
        // printf("\nsb.stz: %ld",sb.st_size);
        return sb.st_size;
    }
    else
    {
        return 0;
    }
}

// Copies or deletes a file based on parameter
int cprmfile(char *path, int type)
{
    // Perform fork() and execute copy or remove operation using execlp()
    int child = fork();
    if (child == 0 && type == 1)
    {
        execlp("cp", "cp", path, ".", NULL);
    }
    if (child == 0 && type == 2)
    {
        // Performing chmod because proc files are write protected
        chmod(path, S_IWUSR);
        execlp("rm", "rm", path, NULL);
    }
    // wait for the process to complete and display appropriate error handling messages
    int status;
    waitpid(child, &status, 0);
    if (WIFSIGNALED(status))
    {
        if (type == 1)
        {
            printf("\nCopy operation failed \nfilename: %s status:%d", path, WTERMSIG(status));
            fflush(stdout);
        }
        else
        {
            printf("\nRemove operation failed \nfilename: %s status:%d", path, WTERMSIG(status));
            fflush(stdout);
        }
        exit(0);
    }
    else if (WIFEXITED(status))
    {
        return 1;
    }
    return 0;
}

// Returns pointer to a struct variable that contains pid, ppid, pgid, childrens and number of childrens
struct status *parseStat(char *procid)
{
    // Allocate memory for the stat file path string
    char *statpath = malloc((11 + strlen(procid)) * sizeof(char));

    // scaffold the stat file path
    sprintf(statpath, "/proc/%s/stat", procid);

    // Copy the stat file from proc folder to current folder and continue if the operation is sucesss, if not exit
    if (cprmfile(statpath, 1) != 1)
    {
        printf("\nProcess process does not exist\n");
        return 0;
    }

    // Allocate memory for the children file path string
    char *childpath = malloc((20 + (2 * strlen(procid)) * sizeof(char)));

    // scaffold the children file path
    sprintf(childpath, "/proc/%s/task/%s/children", procid, procid);

    // Allocate memory for the status struct variable
    struct status *ptr = malloc(sizeof(struct status));

    // Allocate memory for file descriptor and open the copied stat file in read-only mode
    int *fd = malloc(1 * sizeof(int));
    *fd = open("stat", O_RDONLY);

    // Allocate memory for buffer variable. This byte length can be changed in the future by accessing stat of the file
    char *buff = malloc(100 * sizeof(char));

    // Set spaces in every position of the character array to prevent garbage values
    memset(buff, ' ', 100);
    
    // Allocate size for read() return variable and perform read operation 
    int *d = malloc(1*sizeof(int));
    *d = read(*fd, buff, 99);

    int *count = malloc(1*sizeof(int));
    *count=0;

    // Break the string into series of tokens delimited by the space
    char *token = strtok(buff, " ");

    //traverse through the tokens 
    while (token != NULL)
    {
        if (isdigit(token[0]))
        {
            // If its a number and if its first, then its the pid, second is ppid and third is pgid
            if (*count == 0)
            {
                ptr->pid = atoi(token);
                *count+=1;
            }
            else if (*count == 1)
            {
                ptr->ppid = atoi(token);
                *count+=1;
            }
            else if (*count == 2)
            {
                ptr->pgid = atoi(token);
                *count+=1;
                break;
            }
        }
        if (isalpha(token[0]))
        {
            // If its an alphabet and if its "R"(Running) then set process state to 1, "Z"(defunct) then set process state to 2,
            // "S"(Sleepting) then set process state to 3

            if (strcmp(token, "R") == 0)
            {
                ptr->state = 1;
            }
            else if (strcmp(token, "T") == 0)
            {
                ptr->state = 4;
            }
            else if (strcmp(token, "Z") == 0)
            {
                ptr->state = 2;
            }
            else if (strcmp(token, "S") == 0)
            {
                ptr->state = 3;
            }
        }
        token = strtok(NULL, " ");
    }

    // Close the stat file
    close(*fd);

    // Copy the children file from proc folder to current folder and continue if the operation is sucesss, if not exit
    if (cprmfile(childpath, 1) != 1)
    {
        printf("\nAccessing child file failed");
        return 0;
    }

    // Allocate memory for children file size variable and get the child file size from filebytesize()
    int *childfz = malloc(1 * sizeof(int));
    *childfz = filebytesize("children");

    // Open the children file in read-only mode
    *fd = open("children", O_RDONLY);

    // Clear the buffer by changing every array location to spaces
    memset(buff, ' ', *childfz);

    // Read the children file till children file size
    *d = read(*fd, buff, *childfz);

    ptr->childsize = 0;

    // Break the string into series of tokens delimited by the space
    token = strtok(buff, " ");

    // If number of bytes read is greater than zero then traverse through the tokens
    if (*d > 0)
    {
        // Allocate size for first children before handed
        ptr->children = malloc(((ptr->childsize + 1) * sizeof(int)));
        // Traverse through the tokens, increase the memory allocation by using realloc() whenever you need to insert a value
        while (token != NULL)
        {
            if (isdigit(token[0]))
            {
                int *statin = malloc(1 * sizeof(int));
                *statin = realloc(ptr->children, ((ptr->childsize + 1) * sizeof(int)));
                *(ptr->children + ptr->childsize) = atoi(token);
                ptr->childsize += 1;
                free(statin);
            }
            token = strtok(NULL, " ");
        }
    }

    // Close the file, delete stat and children files and free the memory occupied by variables
    close(*fd);
    cprmfile("stat", 2);
    cprmfile("children", 2);

    free(fd);
    free(d);
    free(count);
    free(childpath);
    free(statpath);
    free(buff);
    free(childfz);

    // Return the pointer
    return ptr;
}

// Perform SIGKILL, SIGSTOP or SIGCONT operation based on parameter supplied by calling function
int procLevelOp(struct status *proc, struct status *rootproc, int type)
{
    // If process' pgid and root/parent pgid does not match and process' pgid and process' pid does not match then
    // the process does not exist in hierarchy, display appropriate message and exit
    if ((proc->pgid != rootproc->pgid) && (proc->pgid != proc->pid))
    {
        printf("\nProcess not found in hierarchy\n");
        return 0;
    }

    // Diplay the process id and parent id if no argument was given by user
    if (type == 0)
    {
        printf("\n%d %d\n", proc->pid, proc->ppid);
        return 1;
    }
    else if (type == 1)
    {
        // Kill the process using SIGKILL if option -rp was given by user
        if (kill(proc->pid, SIGKILL) < 0)
        {
            printf("\nSIGKILL failed killing process\n");
            return 0;
        }
        printf("\nProcess killed successfully\n");
        return 1;
    }
    else if (type == 2)
    {
        // Kill the root/parent process if option -pr was given by user
        if (kill(rootproc->pid, SIGKILL) < 0)
        {
            printf("\nSIGKILL failed killing root process\n");
            return 0;
        }
        printf("\nRoot process killed successfully\n");
        return 1;
    }
    else if (type == 3)
    {
        // STOP/Pause the process if option -xt was given by user
        if (kill(proc->pid, SIGSTOP) < 0)
        {
            printf("\nSIGTOP failed suspending the process\n");
            return 0;
        }
        printf("\nProcess suspended successfully\n");
        return 1;
    }
    else if (type == 4)
    {
        // Send SIGCONT to process if option -xc was given by user
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
        // Print the status of process if option -zs was given by user
        if (proc->state == 2)
        {
            printf("\nProcess is defunct\n");
            return 1;
        }
        else if (proc->state == 3)
        {
            printf("\nProcess is sleeping\nNot defunct\n");
            return 1;
        }
        else if (proc->state == 4)
        {
            printf("\nProcess is paused\nNot defunct\n");
            return 1;
        }
        printf("\nProcess is still running\nNot defunct\n");
    }
}

// List PIDs of non-direct descendants or immediate descendants or sibling processes or 
// descendants that are defunct or grandchildren
int listprocessop(struct status *proc, struct status *rootproc, int type)
{
    // If process' pgid and root/parent pgid does not match and process' pgid and process' pid does not match then
    // the process does not exist in hierarchy, display appropriate message and exit
    if ((proc->pgid != rootproc->pgid) && (proc->pgid != proc->pid))
    {
        printf("\nProcess not found in hierarchy\n");
        return 0;
    }
    // List non-direct descendants or defunct descendants if option -xn or -xz is supplied
    if (type == 1 || type == 4)
    {
        // If there's no children then exit by return 0
        if (proc->childsize == 0)
        {
            return 0;
        }
        // Iterate through the childrens
        for (int i = 0; i < proc->childsize; i++)
        {
            // if Child pid is zero then exit
            if (*(proc->children + i) == 0)
            {
                return 0;
            }
            // Allocate memory for child status struct and retrieve the status
            char *child=malloc(10*sizeof(char));
            memset(child,' ',10);
            sprintf(child, "%d", *(proc->children + i));
            struct status *childst = parseStat(child);
            free(child);

            // If the option is -xn and child ppid is not same as initial pid, then print
            if (type == 1 && (!(childst->ppid == fs_procid)))
            {
                printf("%d ", childst->pid);
                fflush(stdout);
                dcount++;
            }
            // If the option is -xz and child state is defunct then print it
            if (type == 4 && (childst->state == 2))
            {
                printf("%d ", childst->pid);
                fflush(stdout);
                dcount++;
            }

            // Send child proc and current pid proc into a recursive function
            listprocessop(childst, proc, type);

            // Free space allocated for child struct
            free(childst);
        }

        return 1;
    }
    else if (type == 2)
    {
        // List all the child pids if option -xd was supplied
        // If there's no children then exit
        if (proc->childsize == 0)
        {
            printf("\nNo direct descendants \n");
            return 1;
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
        return 1;
    }
    else if (type == 3)
    {
        // If option -xs was given and root process id given is not same as ppid of process
        if (proc->ppid != rootproc->pid)
        {
            // Get ppid from stat struct of proc id and populate struct for ppid
            char ppidstr[10];
            sprintf(ppidstr, "%d", proc->ppid);
            struct status *ptproc = malloc(sizeof(struct status));
            ptproc = parseStat(ppidstr);
            
            // Check number of children and traverse in a loop 
            if(ptproc->childsize ==1){
                printf("\nProcess does not have any siblings\n");
                return 1;
            }
            for (int i = 0; i < ptproc->childsize; i++)
            {
                // If pid and user supplied pid matches then skip else print the pid
                if ((*(ptproc->children + i) == 0) || *(ptproc->children + i) == proc->pid)
                {
                    continue;
                }
                printf("%d ", *(ptproc->children + i));
            }
            free(ptproc);
            
        }
        else{
            // If rootproc is sme ppid's proc then continue or exit
            if(rootproc->childsize ==1){
                printf("\nProcess does not have any siblings\n");
                return 1;
            }
            // Traverse through the list and ignore the pid which we gave
            for (int i = 0; i < rootproc->childsize; i++)
            {
                if ((*(rootproc->children + i) == 0) || *(rootproc->children + i) == proc->pid)
                {
                    continue;
                }
                printf("%d ", *(rootproc->children + i));
            }

        }
        printf("\n");

        return 1;
    }
    else if (type == 5)
    {
        // Get childsize and if it's 0 then exit 
        if (proc->childsize == 0)
        {
            return 0;
        }
        // Traverse through the childrens and check if they match pgid or ppid
        for (int i = 0; i < proc->childsize; i++)
        {

            if (*(proc->children + i) == 0)
            {
                continue;
            }
            char *child=malloc(10*sizeof(char));
            memset(child,' ',10);
            sprintf(child, "%d", *(proc->children + i));
            
            struct status *childst = parseStat(child);
            free(child);
            if (childst->childsize == 0)
            {
                continue;
            }
            for (int j = 0; j < childst->childsize; j++)
            {
                printf("%d ", *(childst->children + j));
                dcount++;
            }
            free(childst);
        }
        return 1;
    }
    return 0;
}

void main(int argc, char *argv[])
{
    int stat = 0;
    // Debug code
    // struct status *procid_stat = parseStat(argv[1]);
    // printf("\nproc id: %d\n", procid_stat->pid);
    // printf("\npproc id: %d\n", procid_stat->ppid);
    // printf("\nroot id: %d\n", procid_stat->pgid);
    // printf("\nstat: %d\n", procid_stat->state);
    // printf("\nchildsize: %d\n", procid_stat->childsize);
    // for (int i = 0; i < procid_stat->childsize; i++)
    // {
    //     printf("%d ", *(procid_stat->children + i));
    // }
    
    // Populate the status struct variable for process id and root/parent process id
    struct status *proc = parseStat(argv[1]);
    struct status *rootproc = parseStat(argv[2]);

    // For the appropriate option(-rp, pr, etc.), procLevelOp function or Listprocessop function will be invoked
    if (argc == 3)
    {
        stat = procLevelOp(proc, rootproc, 0);
    }
    if (argc == 4)
    {
        if (strcmp(argv[3], "-rp") == 0)
        {
            stat = procLevelOp(proc, rootproc, 1);
        }
        else if (strcmp(argv[3], "-pr") == 0)
        {
            stat = procLevelOp(proc, rootproc, 2);
        }
        else if (strcmp(argv[3], "-xt") == 0)
        {
            stat = procLevelOp(proc, rootproc, 3);
        }
        else if (strcmp(argv[3], "-xc") == 0)
        {
            stat = procLevelOp(proc, rootproc, 4);
        }
        else if (strcmp(argv[3], "-zs") == 0)
        {
            stat = procLevelOp(proc, rootproc, 5);
        }
        else if (strcmp(argv[3], "-xn") == 0)
        {
            fs_procid = atoi(argv[1]);
            stat = listprocessop(proc, rootproc, 1);
            (dcount==0) && printf("\nNo non-direct descendants\n");
            printf("\n");
        }
        else if (strcmp(argv[3], "-xd") == 0)
        {
            stat = listprocessop(proc, rootproc, 2);
        }
        else if (strcmp(argv[3], "-xs") == 0)
        {
            stat = listprocessop(proc, rootproc, 3);
        }
        else if (strcmp(argv[3], "-xz") == 0)
        {
            stat = listprocessop(proc, rootproc, 4);
            (dcount==0) && printf("\nNo descendant zombie process/es\n");
        }
        else if (strcmp(argv[3], "-xg") == 0)
        {
            stat = listprocessop(proc, rootproc, 5);
            (dcount==0) && printf("\nNo grandchildren\n");
            printf("\n");
        }
    }

    // freeing memory occupied by the struct of process id and root/parent process id
    free(proc);
    free(rootproc);

    // Print operation unsuccessful if the operation failed
    if (stat == 0)
    {
        printf("\nOperation Unsuccessful\n");
        fflush(stdout);
    }
}