//utility.h
//contains utility functions that will be used throughout the program 

#ifndef UTILITY_H
#define UTILITY_H

#include "record.h"
#include "Data.h"
#include <stdbool.h>

//linked list node 
struct UtilityList{
    Record *array; //array that contains Record Objects  
    int size; //size of the array
    struct UtilityList *next; //pointer to the next node of the linked list
};

//function protorypes
int read_full(int fd, void *buffer, size_t count);
bool IsInt(char *string);
FILE *Read_Binary_File(char *bin_file);
int Split_File(char *file,int N);
void Send_Signal_To_Root(pid_t parent_pid,int SIGUSR);
int Compare_Records(const void *r1,const void *r2);
void Sort_Records(Record *array,int size,int (*cmpr)(const void *, const void*));
int Get_Final_Size(struct UtilityList *list);
Record * ConcatenateArrays(struct UtilityList *list);
Data Merge(struct UtilityList *list);
void InsertItem(Record *array,int size,struct UtilityList **head);
int Get_List_Size(struct UtilityList *head);

#endif // UTILITY_H
