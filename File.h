#ifndef FILE_H
#define FILE_H


#include <stdlib.h>
#include <stdio.h>
#define FILE_NAME_LENGTH 20


typedef struct{
    int file_desc;
    char *file_name;
    FILE *fptr;
}File;

#endif // FILE_H
