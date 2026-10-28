#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <ctype.h> 
#include <sys/wait.h>
#include <errno.h>

#include "utility.h"
#include "record.h"
#include "Data.h"



int read_full(int fd, void *buffer, size_t count){

    char *p = buffer;
    size_t done = 0;

    while(done < count){

        ssize_t n = read(fd, p + done, count - done);

        if(n < 0){
            if(errno == EINTR) continue;
            return -1;
        }

        if(n == 0) return -1;

        done += (size_t)n;
    }

    return 0;
}


//utility function to check if a string is an integer
//uses isdigit()
bool IsInt(char *string){
    int len = strlen(string);
    int count = 0;
    //if every character of the string is a digit then the string is an integer
    for(int i=0;i<len;i++){
        if(isdigit(string[i])) count++;
    }
    if(count == len) return true;
    return false;
}

//function to open a binary file and return the FILE pointer
//pointing to that file
FILE *Read_Binary_File(char *bin_file){
    FILE *file_ptr;
    file_ptr = fopen(bin_file,"rb");
    if(file_ptr == NULL){
        printf("%s file can't be opened!\n",bin_file);
        return NULL;
    }
    return file_ptr;
}

//function to split the number of Records that the file Contains 
//according to the number of splitters that will be created
int Split_File(char *file,int N){
    //get the number of records in the file
    Record rec;
    FILE * fptr = Read_Binary_File(file);
    fseek(fptr,0,SEEK_END);
    int size = ftell(fptr);
    rewind(fptr);
    int records = (int) size/sizeof(rec);
    fclose(fptr);
    //return the number of records divided by the number of splitters
    return records / N;
}


//function to send a USR1 or USR2 signal to the root
void Send_Signal_To_Root(pid_t parent_pid,int SIGUSR){
    //sleep(1);
    kill(parent_pid,SIGUSR);
    return;
}


//function to Compare the records
int Compare_Records(const void *r1,const void *r2){
    const Record *record1 = (const Record *)r1;
    const Record *record2 = (const Record *)r2;
    //compare the last names
    int cmp_val = strcmp(record1->lastname,record2->lastname);
    if(cmp_val == 0){ //means the last names are equal
        //compare the first names
        int cmp_val_1 = strcmp(record1->name,record2->name);
        if(cmp_val_1 == 0){ //means the first names are equal
            //compare the ids
            return record2->id - record1->id;
        }    
        return cmp_val_1; //else return the value strcmp returns
    }
    return cmp_val; //else return the value strcmp returns
}

//sort the concatenated array (merge) it uses qsort() to sort the file efficiently and quickly
void Sort_Records(Record *array,int size,int (*cmpr)(const void *, const void*)){
    qsort(array,size,sizeof(Record),cmpr);
}


//funtion to get the sum of all the sizes of the arrays in the list
//this will be the size of the final merged array
int Get_Final_Size(struct UtilityList *list){

    int final_size = 0;

    struct UtilityList *temp = list;
    while(temp != NULL){
        final_size += temp->size;
        temp = temp->next;
    }

    return final_size;
}

//function to concatenate n arrays
Record * ConcatenateArrays(struct UtilityList *list){

    int final_size = Get_Final_Size(list); //get the final size 

    Record * final = malloc(final_size * sizeof(Record)); //allocate memory for the final array 
    if(final == NULL){
        perror("malloc");
        return NULL;

    }
    //iterate through the list of arrays and copy the memory contents of each array to the final array
    //using memcpy() and an offset variable so it copies the data in the correct mamory place
    struct UtilityList *temp = list;
    size_t offset = 0; 
    while(temp != NULL){
        memcpy(final + offset,temp->array,temp->size * sizeof(Record));
        offset += temp->size;
        temp = temp->next;
    } 

    return final;
}

//function to merge n sorted arrays 
//takes a linked list as input that each node contains one array
Data Merge(struct UtilityList *list)
{
    Data data;
    data.array = NULL;
    data.len = 0;

    int k = Get_List_Size(list);
    int total = Get_Final_Size(list);

    if(total == 0 || k == 0){
        return data;
    }

    //store the linked-list nodes so we can access
    //each sorted array independently.
    struct UtilityList **nodes =
        malloc(k * sizeof(struct UtilityList *));

    int *pos = calloc(k, sizeof(int));

    Record *result = malloc(total * sizeof(Record));

    if(nodes == NULL || pos == NULL || result == NULL){
        perror("malloc");
        free(nodes);
        free(pos);
        free(result);
        return data;
    }

    struct UtilityList *current = list;

    for(int i = 0; i < k; i++){
        nodes[i] = current;
        current = current->next;
    }

    //produce the final sorted array.
    for(int out = 0; out < total; out++){

        int best = -1;

        //find the smallest available record among
        //the sorted arrays.
        for(int i = 0; i < k; i++){

            if(pos[i] >= nodes[i]->size) continue;
    
            if(best == -1 ||Compare_Records(&nodes[i]->array[pos[i]],&nodes[best]->array[pos[best]]) < 0) best = i;
            
        }

        //no record should be missing if the sizes
        //and input arrays are valid.
        if(best == -1){
            free(nodes);
            free(pos);
            free(result);

            fprintf(stderr, "Merge error: missing records\n");
            return data;
        }

        result[out] = nodes[best]->array[pos[best]];
        pos[best]++;
    }

    free(nodes);
    free(pos);

    data.array = result;
    data.len = total;

    return data;
}


//utility function to create an initilize a linked list node 
struct UtilityList * CreateNode(Record *array,int size){

    struct UtilityList *new_node = malloc(sizeof(struct UtilityList));

    if(new_node == NULL){
        perror("malloc");
        return NULL;
    }

    new_node->size = size;
    new_node->array = array;
    new_node->next = NULL;

    return new_node;
}

//function to insert an item in the linked list 
void InsertItem(Record *array,int size,struct UtilityList **head){

    struct UtilityList *new_node = CreateNode(array,size);

    if(new_node == NULL) return;

    if(*head == NULL){
        *head = new_node;
        return;
    }

    struct UtilityList *temp = *head;
    while(temp->next != NULL){
        temp = temp->next;
    }
    temp->next = new_node;

    return;
}

//function to get the size of the list
int Get_List_Size(struct UtilityList *head){

    int count = 0;

    if(head == NULL) {
        printf("List Is Empty!\n");
        return count;
    }

    struct UtilityList *temp = head;
    while(temp->next != NULL){
        temp = temp->next;
        count++;
    }

    count++;
    return count;
}


















//////// OLD FUNCTIONS ////////

// Data Read_Data_From_Splitter(int fd[][2],int i,struct UtilityList * list){

//     Data data;
//     int len;
//     close(fd[i][1]); //close the write end of the pipe
//     read(fd[i][0], &len, sizeof(int));

//     //allocate memory for the array and check if the allocation fails
//     Record * array = malloc(sizeof(Record) * len);
//     if(array == NULL){
//         perror("malloc");
//         exit(1);
//     }
//     //read the array from the read end of the pipe
//     read(fd[i][0], array, sizeof(Record) * len);

//     //update the struct
//     data.array = array;
//     data.len = len;

//     close(fd[i][0]); //close the read end of the pipe
//     return data;
// }


// Data Read_Data_From_Sorter(int fd[][2],int i,struct UtilityList * list){

//     Data data;
//     int len;
//     close(fd[i][1]); //close the write end of the pipe
//     read(fd[i][0], &len, sizeof(int));

//     //allocate memory for the array and check if the allocation fails
//     Record * array = malloc(sizeof(Record) * len);
//     if(array == NULL){
//         perror("malloc");
//         exit(1);
//     }
//     //read the array from the read end of the pipe
//     read(fd[i][0], array, sizeof(Record) * len);

//     //update the struct
//     data.array = array;
//     data.len = len;

//     close(fd[i][0]); //close the read end of the pipe
//     return data;
// }


// Data Merge(struct UtilityList *list){
//     Record * array = ConcatenateArrays(list);
//     int size = Get_Final_Size(list);
//     Sort_Records(array,size,Compare_Records);
//     Data data;
//     data.array = array;
//     data.len = size;
//     return data;
// }