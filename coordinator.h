// coordinator.h

#ifndef COORDINATOR_H
#define COORDINATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "record.h"
#include "utility.h"

#define BUFFER_LENGTH_1 20
#define BUFFER_LENGTH_2 100 // line length



//function prototypes
void Create_Splitters(int N, char *file, int amount, char *exe1, char *exe2);
void Print_Final_Data(Record *array,int size);

#endif
