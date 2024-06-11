#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

// struct str{
//     int *ptr;
// }

// performop(struct str *st){
//     st->ptr=malloc(3*sizeof(int));
//     for(int i=0;i<3;i++){
//         scanf("%d",st->ptr+i);
//     }

// }

void main(){

    char *tt = "29720";
    printf("\n%d\n",atoi(tt));

    // char arr[] = "laksnd aslkdnd slkdn";
    // char *stt;
    // char *res = arr;
    // stt = strsep(&res, " ");
    // char*rr;
    // rr = strsep(&res, " ");
    // printf("%s\n", rr);
    // for(int i=0;i<strlen(stt);i++){
    //     printf("%s\n",stt+i);
        
    // }
    // char *tmp = "R";
    // tmp = NULL;
    // if(tmp == NULL){
    //     printf("\nexit\n");
    // }
    // switch(*tmp){
    //     case 'R':
    //     printf("R\n");
    //     break;
    //     default:
    //     printf("\nWrror\n");
    // }
    // struct str *st = malloc(sizeof(struct str));
    // performop(st);
    // for(int i=0;i<3;i++){
    //     printf("%d ",*(st->ptr+i));
    // }
}
