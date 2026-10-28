// splitter.h

#ifndef SPLITTER_H
#define SPLITTER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Info.h"
#include "record.h"
#include "utility.h"

#define FILE_BUFFER 20

//function prototypes
void Create_Sorters(int n, Process_Info info, int pipe_fd[][2]);
Process_Info InitSplitter(int fd[][2], int i);

#endif

