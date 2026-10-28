#ifndef INFO_H
#define INFO_H

#define BUFFER 100

//this struct contains the info a process (splitter or sorter)
typedef struct{
    int index; //the index of the process
    char *file; //the file
    int N; //the N (obtained by the -k flag)
    int amount; //the amount to sort 
    int file_len; //the length of the file
    pid_t root_pid; //the pid of the root
    char *exe1; //sorting executable 1
    char *exe2; //sorting executable 2
}Process_Info;


#endif // INFO_H
