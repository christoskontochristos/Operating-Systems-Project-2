//splitter.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#include "splitter.h"
#include "record.h"
#include "Info.h"
#include "sorter.h"
#include "utility.h"
#include "macros.h"
#include "Data.h"

static int length = 0;


#define FILE_BUFFER 20





//function to read the data from a sorter
//returns a Data struct object that contains the array read and its length
Data Read_Data_From_Sorter(int fd[][2], int i, struct UtilityList *list){
    Data data;
    int len;

    close(fd[i][1]);

    //read the record count completely.
    if(read_full(fd[i][0], &len, sizeof(len)) == -1 || len < 0 || len > 100000000) {
        fprintf(stderr, "Invalid record count from sorter %d\n", i);
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

        //read the entire array, not just one portion.
        if(read_full(fd[i][0], array, sizeof(*array) * (size_t)len) == -1){
            fprintf(stderr, "Incomplete array from sorter %d\n", i);
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


//function to iteratively read the sorted data from all the sorters
//returns a linked list that contains all the arrays read from the sorters
struct UtilityList * Read(int fd[][2],int n){

    struct UtilityList *list = NULL;
    for(int i=0;i<n;i++){
        //read data from a sorter
        Data data = Read_Data_From_Sorter(fd,i,list);
        //insert the array to the list of arrays
        InsertItem(data.array, data.len, &list);
    }

    return list;
}



//function to create n processes/sorters using fork()
void Create_Sorters(int n, Process_Info info, int pipe_fd[][2]){

    //n file descreptors for n pipes 
    int fd[n][2]; // fd[i][0] -> read , fd[i][1] -> write - parent to child communication
    int fd_1[n][2]; // fd_1[i][0] -> read , fd_1[i][1] -> write - child to parent communication

    //create num processes/sorters
    int i;
    for(i=0;i<n;i++){
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

        pid_t fork_value = fork();//get the fork value
        if(fork_value < 0){ //check for fork failure
            perror("fork");
            exit(1);
        }
        if(fork_value == 0){ //child process - sorter

            printf("sorter %d from splitter %d\n",i,info.index);
            //read the data from the read end of the pipe
            Process_Info sorter_info = Init_Sorter(fd,i);
            //send a USR2 signal to the root
            Send_Signal_To_Root(info.root_pid,SIGUSR2);
            //pick a sorting program and load it with the execvp function
            Pick_Sorting_Program(sorter_info,info.index,fd_1,i,n);
            //** this part is not going to execute **
            exit(0);

        }
        //parent process - splitter
        else{ 
            
            //write all the data to the write end of the pipe
            int val = i;

            close(fd[i][0]);

            write(fd[i][1], &val, sizeof(int)); //the i is the index of the sorter
            write(fd[i][1], &info.N, sizeof(int)); //the amount of splitters N given in the command line
            write(fd[i][1], &info.amount, sizeof(int)); //the amount of records for each sorter
            write(fd[i][1], &info.root_pid, sizeof(pid_t)); //the root pid
            write(fd[i][1], &info.file_len, sizeof(int)); //the length of the file (the string)
            write(fd[i][1], info.file, sizeof(char) * (info.file_len+1)); //the file string
            write(fd[i][1], info.exe1, sizeof(char) * EXE_LENGTH); //the sorting executable 1 string
            write(fd[i][1], info.exe2, sizeof(char) * EXE_LENGTH); //the sorting executable 2 string

            close(fd[i][1]);

            //wait(NULL); //wait for the children process to finish
        }
    }
    // for(i=0;i<n;i++) wait(NULL); //wait N times (for every child process created) 

    struct UtilityList *l = Read(fd_1,n); // Read the data from the splitter using the second pipe fd_1

    for(i=0;i<n;i++) wait(NULL); //wait N times (for every child process created) 


    Data data = Merge(l); //merge the ararys

    //write the sorted data at the write end of the pipe so the root can read it
    close(pipe_fd[info.index][0]);
    write(pipe_fd[info.index][1], &data.len, sizeof(int));
    write(pipe_fd[info.index][1], data.array, sizeof(Record) * data.len);
    close(pipe_fd[info.index][1]);
}

//function to Initialize the data/info of the current process/splitter
Process_Info InitSplitter(int fd[][2],int i){
    //receive all the data from the coordinator and place it into a struct
    //that contains the info of the current process
    Process_Info splitter_info;
    int splitter_index, N, amount, file_len;
    pid_t root_pid;
    char *file;

    //allocate memory for the sorting executables 1 and 2
    char *exe1 = malloc(sizeof(char)*EXE_LENGTH), *exe2 = malloc(sizeof(char)*EXE_LENGTH);
    if(exe1 == NULL || exe2 == NULL){
        perror("malloc");
        exit(0);
    }

    close(fd[i][1]); //close the write end of the pipe

    //read all the info the root passed down to the splitter through the pipe
    read(fd[i][0], &splitter_index, sizeof(int));
    read(fd[i][0], &N, sizeof(int));
    read(fd[i][0], &amount, sizeof(int));
    read(fd[i][0], &root_pid, sizeof(pid_t));
    read(fd[i][0], &file_len, sizeof(int));

    //allocate memory for the file
    file = malloc(sizeof(char)*file_len);
    if(file == NULL){
        perror("malloc");
        exit(0);
    }

    read(fd[i][0], file, sizeof(char) * (file_len+1));
    read(fd[i][0], exe1, sizeof(char) * EXE_LENGTH);
    read(fd[i][0], exe2, sizeof(char) * EXE_LENGTH);

    close(fd[i][0]); //close the read end of the pipe

    printf("splitter index : %d ,parent : %d\n",splitter_index,root_pid);

    //update the struct that contains the process info
    splitter_info.index = splitter_index;
    splitter_info.N = N;
    splitter_info.amount = amount;
    splitter_info.file_len = file_len;
    splitter_info.file = file;
    splitter_info.root_pid = root_pid;
    splitter_info.exe1 = exe1;
    splitter_info.exe2 = exe2;

    return splitter_info;
}


