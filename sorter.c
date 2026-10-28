//also make the sorters have an id which will also be i and if its odd they will use sorting1.c and 
//if its even they will use sorting2.c (if(sorter_id%2 == 0) execvp("./sorting1",args) else ...)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>


#include "Info.h"
#include "sorter.h"
#include "macros.h"
#include "record.h"





void Pick_Sorting_Program(Process_Info info,int splitter_index,int pipe_fd[][2],int i,int n){

    //whatever the sorting executable writes it goes to the pipe fd_1 in splitter.c
    close(pipe_fd[i][READ]);
    dup2(pipe_fd[i][WRITE], STDOUT_FILENO); //redirect to stdout
    close(pipe_fd[i][WRITE]);


    //cast the sorter index into a string
    char index[BUFFER_LENGTH_1];
    sprintf(index,"%d",info.index); 

    //cast the amount to sort into a string
    char amount[BUFFER_LENGTH_1];
    sprintf(amount,"%d",info.amount);

    //cast the N into a string
    char num[BUFFER_LENGTH_1];
    sprintf(num,"%d",info.N);

    //cast the splitter index into a string
    char splitter_idx[BUFFER_LENGTH_1];
    sprintf(splitter_idx,"%d",splitter_index);

    //cast the file desc into a string
    char sorters_num[BUFFER_LENGTH_1];
    sprintf(sorters_num,"%d",n);

    //call the sorting program according to the id of the sorter
    //if id mod 2 = 0 -> sorting1
    //else -> sorting2
    if(info.index % 2 == 0){ 
        //anything that info.exe1 writes with write() goes to the pipe
        char *args[] = {info.exe1,info.file,index,amount,num,splitter_idx,sorters_num,NULL};
        fflush(stdout);
        execvp(info.exe1,args); //call the 1st independent sorting program 
        perror("execvp");
        exit(0);
    }
    else{
        //anything that info.exe2 writes with write() goes to the pipe
        char *args[] = {info.exe2,info.file,index,amount,num,splitter_idx,sorters_num,NULL};
        fflush(stdout);
        execvp(info.exe2,args); //call the 2nd independent sorting program
        perror("execvp");
        exit(0);
    }
}

//function to initialize the sorter info
//reads the data from the read end of the pipe and
Process_Info Init_Sorter(int fd[][2],int i){

    Process_Info sorter_info;
    int sorter_index, N, amount, file_len;
    pid_t root_pid;
    char *file;

    //allocate memory for the sorting executables 1 and 2
    char *exe1 = malloc(sizeof(char)*EXE_LENGTH), *exe2 = malloc(sizeof(char)*EXE_LENGTH);
    if(exe1 == NULL || exe2 == NULL){
        perror("malloc");
        exit(0);
    }

    //read all the info the splitter passed down to the root through the pipe
    close(fd[i][1]); //close the write end of the pipe
    
    read(fd[i][0], &sorter_index, sizeof(int));
    read(fd[i][0], &N, sizeof(int));
    read(fd[i][0], &amount, sizeof(int));
    read(fd[i][0], &root_pid, sizeof(pid_t));
    read(fd[i][0], &file_len, sizeof(int));

    //allocate memory for the file
    file = malloc(sizeof(char) * (file_len+1));
    if(file == NULL){
        perror("malloc");
        exit(0);
    }

    read(fd[i][0], file, (file_len+1) * sizeof(char));
    read(fd[i][0], exe1, sizeof(char) * EXE_LENGTH);
    read(fd[i][0], exe2, sizeof(char) * EXE_LENGTH);

    close(fd[i][0]);  //close the read end of the pipe

    //update the struct that contains the process info
    sorter_info.index = sorter_index;
    sorter_info.N = N;
    sorter_info.amount = amount;
    sorter_info.root_pid = root_pid;
    sorter_info.file_len = file_len;
    sorter_info.file = file;
    sorter_info.exe1 = exe1;
    sorter_info.exe2 = exe2;
        
    return sorter_info;
}
