

CC = gcc
CFLAGS = -g

all: final

final: mysort.o coordinator.o splitter.o sorter.o utility.o 
	$(CC) $(CFLAGS) -o final mysort.o coordinator.o splitter.o sorter.o utility.o 

mysort: mysort.c 
	$(CC) $(CFLAGS) -o mysort mysort.c 

coordinator: coordinator.c coordinator.h
	$(CC) $(CFLAGS) -o coordinator coordinator.c 

splitter: splitter.c splitter.h
	$(CC) $(CFLAGS) -o splitter splitter.c 

sorter: sorter.c sorter.h
	$(CC) $(CFLAGS) -o sorter sorter.c

utility: utility.c utility.h
	$(CC) $(CFLAGS) -o utility utility.c

sorting1: sorting1.c
	$(CC) $(CFLAGS) -o sorting1 sorting1.c

sorting2: sorting2.c
	$(CC) $(CFLAGS) -o sorting2 sorting2.c

clean: 
	rm -rf *.o final
