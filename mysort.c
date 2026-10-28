#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h> 
#include <string.h>

#include "record.h"
#include "coordinator.h"
#include "macros.h"
#include "utility.h"


//main function of the program
int main(int argc,char **argv){

    char *file;
    char *exe1;
    char *exe2;
    int N;

    if(argc < 9){
        printf("ERROR: Too few command line arguments given!\n");
        exit(1);
    }

    //parse the command line arguments
    for(int i=0;i<argc;i++){
        if(strcmp(argv[i],I_FLAG)==0){ //check for the -i flag
            printf("file : %s\n",argv[i+1]);
            file = argv[i+1];
        }
        if(strcmp(argv[i],K_FLAG)==0){ //check for the -k flag
            if(IsInt(argv[i+1]) == false){  //check if the argument of the -k flag is an integer
                printf("ERROR: The argument of the -k flag  be an integer!\n");
                exit(1);
            }
            else{ 
                N = atoi(argv[i+1]);
                printf("Children num : %d\n",N); 
            }
        }
        //pass those arguments down to the children processes
        if(strcmp(argv[i],E1_FLAG)==0){ //check for the -e1 flag
            exe1 = argv[i+1];
            printf("Sorting executable 1 : %s\n",exe1); 
        }
        if(strcmp(argv[i],E2_FLAG)==0){ //check for the -e2 flag
            exe2 = argv[i+1];
            printf("Sorting executable 2 : %s\n",exe2); 
        }
    }

    

    printf("\n\n\n");

    //get the amount to split and create the splitters
    int amount_to_sort = Split_File(file,N);

    //if the amount to sort is 0 means that the number of records is smaller than the N 
    //N is the amnount of sorters to create
    if(amount_to_sort == 0){
        printf("Number of Records is smaller than the N!\n");
        exit(1);
    }

    Create_Splitters(N,file,amount_to_sort,exe1,exe2);
    return 0;

}
