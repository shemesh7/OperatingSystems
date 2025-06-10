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
#define INTRO \
"══════════════════════════════════════════════\n" \
">> Scheduler Mode : %s\n" \
">> Engine Status  : Initialized\n" \
"──────────────────────────────────────────────\n\n"

#define OUTRO_TW \
"\n──────────────────────────────────────────────\n" \
">> Engine Status  : Completed\n" \
">> Summary        :\n" \
"   └─ Average Waiting Time : %.2f time units\n" \
">> End of Report\n" \
"══════════════════════════════════════════════\n\n"

#define OUTRO_TA \
"\n──────────────────────────────────────────────\n" \
">> Engine Status  : Completed\n" \
">> Summary        :\n" \
"   └─ Total Turnaround Time : %d time units\n\n" \
">> End of Report\n" \
"══════════════════════════════════════════════\n\n"


typedef struct {
    char name[MAX_NAME];
    char description[MAX_DESCRIPTION];
    int arrival_time;
    int burst_time;
    int priority;
    int index;
} Process;

typedef struct {
    Process procs[MAX_PROCESSORS];
    int size;
    int (*CmpProcs)(Process, Process);
} ReadyQueue;

typedef struct {
    char* name;
    bool CalculateTurnAround; // if false, it's Round Robin. else, it isn't.
    int (*CmpProcs)(Process, Process);
    int timeQuantum;
} Scheduler;


void parseCSV(char* FilePath, Process* P, int * count);
Process parseProcess(const char* line);
void sigalarmIgnore();
int cmpFCFS(Process p1, Process p2);
int cmpLowerPriority(Process p1, Process p2);
int cmpSJF(Process p1, Process p2);
int cmpRR(Process p1, Process p2);
void Enqueue(ReadyQueue* queue, Process item);
Process Dequeue(ReadyQueue* queue);
int GetTotalTime(struct timespec starting);
void RunScheduler(Scheduler scheduler, Process procs[], int count);


/**
 * Main
 * @param processesCsvFilePath
 * @param timeQuantum
 */
void runCPUScheduler(char* processesCsvFilePath, int timeQuantum) {
    int count = 0;
    Process procs[MAX_PROCESSORS];

    parseCSV(processesCsvFilePath, procs, &count);

    // FCFS scheduler
    Scheduler FCFS;
    FCFS.name = "FCFS";
    FCFS.CmpProcs = cmpFCFS;
    FCFS.CalculateTurnAround = false;       // print total wait instead
    FCFS.timeQuantum = -1;                  //not relevant for this scheduler
    RunScheduler(FCFS, procs, count);

    // SJF scheduler
    Scheduler SJF;
    SJF.name = "SJF";
    SJF.CmpProcs = cmpSJF;
    SJF.CalculateTurnAround = false;        // print total wait instead
    SJF.timeQuantum = -1;                   //not relevant for this scheduler
    RunScheduler(SJF, procs, count);

    // Priority scheduler
    Scheduler Priority;
    Priority.name = "Priority";
    Priority.CmpProcs = cmpLowerPriority;
    Priority.CalculateTurnAround = false;   // print total wait instead
    Priority.timeQuantum = -1;              //not relevant for this scheduler
    RunScheduler(Priority, procs, count);

    // Round Robin scheduler
    Scheduler RoundRobin;
    RoundRobin.name = "Round Robin";
    RoundRobin.CmpProcs = cmpRR;
    RoundRobin.CalculateTurnAround = true;
    RoundRobin.timeQuantum = timeQuantum;
}


/**
 * initialize all of the processes from the CSV file
 * @param FilePath
 * @param P
 * @param count
 */
void parseCSV(char* FilePath, Process* P, int * count) {
    FILE* file = NULL;
    if ((file = fopen(FilePath, "r")) == NULL) {
        perror("fopen error");
        exit(1);
    }
    // parse every process, line by line
    char L[MAX_LINE];
    while (fscanf(file, "%[^\n]%*c", L) == 1)
    {
        Process proc = parseProcess(L);
        proc.index = *count;
        P[*count] = proc;
        (*count)++;
    }
    if (ferror(file))
    {
        perror("fscanf error");
        exit(1);
    }

    // sort the processes by arrival time using bubble sort
    for (int i = 0; i < *count; i++) {
        for (int j = 0; j < *count; j++) {
            if (P[j].arrival_time > P[j+1].arrival_time) { // swap
                Process temp = P[j];
                P[j] = P[j+1];
                P[j+1] = temp;
            }
        }
    }
}


/**
 * read a single line from the CSV and return the process
 * @param line
 * @return the process
 */
Process parseProcess(const char* line) {
    Process process;
    char *dup = strdup(line);
    char *save_ptr;  // keep original pointer for free
    save_ptr = dup;

    strcpy(process.name, strtok_r(dup, ",", &dup));
    strcpy(process.description, strtok_r(dup, ",", &dup));
    process.arrival_time = atoi(strtok_r(dup, ",", &dup));
    process.burst_time = atoi(strtok_r(dup, ",", &dup));
    process.priority = atoi(strtok_r(dup, ",", &dup));
    // the index will be initialized outside this function

    free(save_ptr);
    return process;
}


void RunScheduler(Scheduler scheduler, Process procs[], int count) {
    // init ready queue
    ReadyQueue Q;
    Q.size = 0;
    Q.CmpProcs = scheduler.CmpProcs;

    // ignore sigalarms - they are just timers for our purpose
    struct sigaction act;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    act.sa_handler = sigalarmIgnore;
    sigaction(SIGALRM, &act, NULL);
    // print intro
    printf(INTRO, scheduler.name);
    fflush(stdout);

    struct timespec startT;
    clock_gettime(CLOCK_MONOTONIC, &startT);
    /**
     * NEED TO CONTINUE
     */
}

int GetTotalTime(struct timespec starting) {
    struct timespec current;
    clock_gettime(CLOCK_MONOTONIC, &current);
    return (int) (current.tv_sec - starting.tv_sec);
}

int cmpFCFS(Process p1, Process p2) {
    return 0;
}

int cmpLowerPriority(Process p1, Process p2) {
    return p1.priority - p2.priority;
}
int cmpSJF(Process p1, Process p2) {
    return p1.burst_time - p2.burst_time;
}
int cmpRR(Process p1, Process p2) {
    return p1.arrival_time - p2.arrival_time;
}
