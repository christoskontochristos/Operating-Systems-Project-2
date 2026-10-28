//coordinator.c

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h> 
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#include "record.h"
#include "coordinator.h"
#include "splitter.h"
#include "utility.h"
#include "macros.h"
#include "Data.h"




static int USR1_Signal_Counter; //global variable to count the USR1 signals
static int USR2_Signal_Counter; //global variable to count the USR2 signals


//function to hasndle the signals the root receives
//updates the global counters that count the total number of each type of signal
void signal_handler(int signal){

    if(signal == SIGUSR1) USR1_Signal_Counter++;

    else if (signal == SIGUSR2) USR2_Signal_Counter++;

    return;
}



//function to read the data from a splitter
//returns a Data struct object that contains the array read and its length
Data Read_Data_From_Splitter(int fd[][2], int i, struct UtilityList *list){
    Data data;
    int len;

    close(fd[i][1]);

    if(read_full(fd[i][0], &len, sizeof(len)) == -1 || len < 0 || len > 100000000){
        fprintf(stderr, "Invalid record count from splitter %d\n", i);
        close(fd[i][0]);
        exit(EXIT_FAILURE);
    }

    Record *array = NULL;

    if(len > 0){

        array = malloc(sizeof(*array) * (size_t)len);

        if(array == NULL){
            perror("malloc");
            close(fd[i][0]);
            exit(EXIT_FAILURE);
        }

        if(read_full(fd[i][0], array, sizeof(*array) * (size_t)len) == -1) {
            fprintf(stderr, "Incomplete array from splitter %d\n", i);
            free(array);
            close(fd[i][0]);
            exit(EXIT_FAILURE);
        }
    }

    data.array = array;
    data.len = len;

    close(fd[i][0]);
    return data;
}


//function to iteratively read the sorted data from all the splitters
//returns a linked list that contains all the arrays read from the splitters
struct UtilityList * Read_1(int fd[][2],int n){

    struct UtilityList *list = NULL;
    for(int i=0;i<n;i++){
        //read data from a splitter
        Data data = Read_Data_From_Splitter(fd,i,list);
        //insert the array to the list of arrays
        InsertItem(data.array, data.len, &list);
    }

    return list;
}






//function to print the final data in the tty  ALSO PRINT THE TIME OF EACH SORTER
void Print_Final_Data(Record *array,int size){

    printf("The contents of the file sorted :\n\n");

    for(int i=0;i<size;i++){
        printf("%d  ",i);
        printf("%-15s %-15s %-6s %d\n",array[i].lastname,array[i].name,array[i].postcode,array[i].id);
    }

    printf("\n\n");
    printf("USR_1 Signals : %d\n",USR1_Signal_Counter); //amount of USR1 signals 
    printf("USR_2 Signals : %d\n",USR2_Signal_Counter); //amount of USR1 signals 
    printf("\n\n");
    //also print the time taken for each sorter to finish
    return;
}


//function to create N processes using fork()
void Create_Splitters(int N,char *file,int amount,char *exe1,char *exe2){ 

    int i;
    //n file descreptors for n pipes
    int fd[N][2]; // fd[i][0] -> read , fd[i][1] -> write // parent to child communication
    int fd_1[N][2]; // fd_1[i][0] -> read , fd_1[i][1] -> write // child to parent communication
    //struct UtilityList * list = NULL;

    //set up the signal handler for USR1 and USR2 signals 
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);

    //create N processes
    for(i=0;i<N;i++){
        //check if pipe() fails
        if(pipe(fd[i]) == -1){
            perror("pipe");
            exit(1);
        }
        //check if pipe() fails
        if(pipe(fd_1[i]) == -1){
            perror("pipe");
            exit(1);
        }

        //fflush(stdout);
        pid_t fork_value = fork(); // get the fork value 
        if(fork_value < 0){ //check for fork failure
            perror("fork");
            exit(1);
        }

        //child process - splitter
        if(fork_value == 0){
            //read the data from the pipe and initialize the info struct 
            Process_Info splitter_info = InitSplitter(fd,i);
            //get the amount of sorters to create (sorters_amount = N - splitter_index)
            int n = splitter_info.N - splitter_info.index;
            //calculate the amount of records that each sorter has to sort 
            int sorting_amount = splitter_info.amount / (splitter_info.N - splitter_info.index);
            splitter_info.amount = sorting_amount;
            //Create the n sorters
            Create_Sorters(n,splitter_info, fd_1);
            //send a USR1 signal to the root
            Send_Signal_To_Root(splitter_info.root_pid, SIGUSR1);

            exit(0);
        }
        //parent process - root
        else{ 
            //write all the data to the write end of the pipe
            //so the children can read it
            int val = i;
            pid_t root_pid = getpid(); //get the root pid
            int file_len = strlen(file);
            
            //printf("parent_pid : %d\n",root_pid);

            close(fd[i][0]);

            write(fd[i][1], &val, sizeof(int)); //the i is the index of the splitter
            write(fd[i][1], &N, sizeof(int)); //the amount N of splitters N given in the command line
            write(fd[i][1], &amount, sizeof(int)); //the amount of records for each splitter
            write(fd[i][1], &root_pid, sizeof(pid_t)); //the root pid
            write(fd[i][1], &file_len, sizeof(int)); //the length of the file (the string)
            write(fd[i][1], file, sizeof(char) * (file_len+1)); //the file string
            write(fd[i][1], exe1, sizeof(char) * EXE_LENGTH); //the sorting executable 1 string
            write(fd[i][1], exe2, sizeof(char) * EXE_LENGTH); //the sorting executable 2 string
            
            close(fd[i][1]);

            //wait(NULL); //wait for the child process to finish 
        }
    }
    // for(i=0;i<N;i++) wait(NULL); //wait N times (for every child process)

    //Read the sorted data from the splitters
    struct UtilityList *l = Read_1(fd_1,N);
    for(i=0;i<N;i++) wait(NULL); //wait N times (for every child process)
    //Merge the sorted data 
    Data data = Merge(l);
    //print everything to tty
    Print_Final_Data(data.array,data.len);

}







