//bubble sort implementation to sort the given binary file 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/times.h>
#include <unistd.h>
#include "record.h"
#include "File.h"


#define BUFFER_LENGTH 100 //line length



//compares 2 string alphabetically
//returns 1 if the first string given is in higher alphabetical order
//returns 2 if the second string given in in higher alphabetical order
//returns 0 if the string are equal 
int Compare_Strings(char *string1,char *string2){
    if(strcmp(string1,string2) < 0) return 1; //string1 is greater than string2
    if(strcmp(string1,string2) > 0) return 2; //string1 is less than string2
    return 0; //means the given strings are equal 
}

//function to swap 2 Records
void swap(Record *x,Record *y){
	Record temp = *x;
	*x = *y;
	*y = temp;
}

//Bubble Sort
void BubbleSort(Record *array,int size){  //function to sort the file
    int i,j;
    bool flag;
    for(i=0;i<size-1;i++){
        flag = false;
        for(j=0;j<size-i-1;j++){
            //compare the last names
            int value = Compare_Strings(array[j].lastname,array[j+1].lastname);
            //if the value is 2 then swap them
            if(value == 2){
                swap(&array[j],&array[j+1]);
                flag = true;
            }
            //if the value is 0 it means they are equal
            else if(value == 0){
                //compare the first names
                value = Compare_Strings(array[j].name,array[j+1].name);
                //if the value is 2 then swap them
                if(value == 2){
                    swap(&array[j],&array[j+1]);
                    flag = true;
                }
                //if the value is 0 it means they are equal
                else if(value == 0){
                    //compare the ids and swap them accordingly
                    if(array[j].id < array[j+1].id){
                        swap(&array[j],&array[j+1]);
                        flag = true;
                    }
                }
            }
        }
        if(flag == false) break;
    }
    return;
}


//function to open a binary file
FILE *Read_Binary_File(char *bin_file){
    FILE *file_ptr;
    file_ptr = fopen(bin_file,"rb");
    if(file_ptr == NULL){
        printf("%s file can't be opened!\n",bin_file);
        return NULL;
    }
    return file_ptr;
}




int main(int argc,char **argv){

    //to count the real time and the cpu time
    double t1, t2, cpu_time, real_time; 
    struct tms tb1, tb2; 
    double ticspersec;

    ticspersec = (double) sysconf(_SC_CLK_TCK); 
    t1 = (double) times(&tb1);

    Record rec;

    //create a struct File object (File.h)
    File *file = malloc(sizeof(File));
    if(file == NULL){
        perror("malloc");
        exit(1);
    }

    if(argc < 7){
        printf("ERROR: More arguments required!\n");
        exit(1);
    }
    
    file->file_name = argv[1];
    file->file_desc = 1; //unecessary I just didnt have time to remove it
    FILE *fptr = Read_Binary_File(file->file_name);
    file->fptr = fptr;

    //get the number of records in the file
    fseek(file->fptr,0,SEEK_END);
    int size = ftell(file->fptr);
    rewind(file->fptr);
    int records = (int) size/sizeof(rec);

    int index = atoi(argv[2]); //sorter index
    int amount_to_sort = atoi(argv[3]); //the amount each sorter has to sort
    int N = atoi(argv[4]); //number of splitters
    int splitter_index = atoi(argv[5]); //splitter index
    int sorters_num = atoi(argv[6]); //number of sorters from the specific splitter
    

    //calculate the end and the start index
    //the start index is the one where the sorter will start reading from the file
    //and the end is where the reading ends
    
    int splitter_amount = records / N;  //the amount of records given to each splitter from coordinator

    //start = sorter index * amount the sorter has to sort  +  splitter index * splitter records amount
    int start = (index * amount_to_sort) + (splitter_index * splitter_amount);

    //end = amount the sorter has to sort * (sorter index+1) + splitter index * splitter record amount
    int end = amount_to_sort * (index + 1) + (splitter_index * splitter_amount);

    //if we are at the last sorter of the current splitter then 
    //read all the records left from the splitter amount
    if(index == sorters_num-1){
        end = splitter_amount * (splitter_index + 1);
    }

    //if we are at the last splitter then read all the records left
    if(splitter_index == N-1){
        end = records;
    }

    Record rec_array[end - start]; 

    for(int i=start;i<end;i++){
        int val = i;
        fseek(file->fptr, val * sizeof(rec), SEEK_SET); //set the file pointer to the correct position
        fread(&rec, sizeof(rec), 1, file->fptr); //read from binary file
        rec_array[i - start] = rec;
    }

    int array_size = end - start;
    BubbleSort(rec_array,array_size);

    int len = array_size;

    //write the data to stdout so they will be redirected to the pipe
    if(write(STDOUT_FILENO, &len, sizeof(int)) == -1){
        perror("write");
        exit(1);
    }
    if(write(STDOUT_FILENO, rec_array, sizeof(Record)*(array_size)) == -1){
        perror("write");
        exit(1);
    }

    free(file);
    fclose(file->fptr);

    t2 = (double) times(&tb2);
    //total cpu time
    cpu_time = (double) ((tb2.tms_utime + tb2.tms_stime) - (tb1.tms_utime + tb1.tms_stime));
    //total real time
    real_time = t2 - t1;
    printf("real time : %lf sec, cpu time : %lf sec\n",real_time,cpu_time);

    return 0;
}
