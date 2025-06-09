#include <math.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#define MAX_NAME 51
#define MAX_DESCRIPTION 101
#define MAX_PROCESSORS 1000
#define MAX_LINE 257


typedef struct
{
    char name[MAX_NAME];
    char description[MAX_DESCRIPTION];
    int arrival_time;
    int burst_time;
    int priority;
    int index;
} Process;

typedef struct
{
    Process procs[MAX_PROCESSORS];
    int size;
    int (*CmpPriority)(Process, Process);
} ReadyQueue;


void parse_CSV(char* FilePath, Process* P, int * count);
Process parse_process(const char* line);
void Enqueue(ReadyQueue* queue, Process item);
Process Dequeue(ReadyQueue* queue);
double GetTimeElapsed(struct timespec startingTime);


/**
 * Main
 * @param processesCsvFilePath
 * @param timeQuantum
 */
void runCPUScheduler(char* processesCsvFilePath, int timeQuantum) {
    int procsCount = 0;
    Process procs[MAX_PROCESSORS];

    parse_CSV(processesCsvFilePath, procs, &procsCount);

    // FCFS scheduler

    // SJF scheduler

    // Priority scheduler

    // Round Robin scheduler
}


/**
 * initialize all of the processes from the CSV file
 * @param FilePath
 * @param P
 * @param count
 */
void parse_CSV(char* FilePath, Process* P, int * count) {
    FILE* file = NULL;
    if ((file = fopen(FilePath, "r")) == NULL) {
        perror("fopen error");
        exit(1);
    }
    // parse every process, line by line
    char L[MAX_LINE];
    while (fgets(L, MAX_LINE, file) != NULL)
    {
        Process proc = parse_process(L);
        proc.index = *count;
        P[*count] = proc;
        (*count)++;
    }
    if (ferror(file))
    {
        perror("fgets error");
        exit(1);
    }

    // sort the processes by arrival time using bubble sort
    for (int i = 0; i < *count; i++) {
        for (int j = 0; j < *count; j++) {
            if (P[j].arrival_time > P[j+1].arrival_time) { //swap
                Process temp = P[j];
                P[j] = P[j+1];
                P[j+1] = temp;
            }
        }
    }
}