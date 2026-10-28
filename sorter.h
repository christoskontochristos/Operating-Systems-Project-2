// sorter.h

#ifndef SORTER_H
#define SORTER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Info.h"

#define BUFFER_LENGTH 100


//function prototypes

void Pick_Sorting_Program(Process_Info info,int splitter_index,int pipe_fd[][2],int index,int n);
Process_Info Init_Sorter(int fd[][2], int i);

#endif
