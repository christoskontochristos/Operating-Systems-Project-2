#ifndef DATA_H
#define DATA_H

#define BUFFER 100

//this struct will contain the sorted data 
//that will be read from the child processes
typedef struct{
    Record * array; //merged sorted array
    int len; //length of the array
}Data;


#endif // DATA_H
