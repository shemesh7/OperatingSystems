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
#define RUNNING_MSG "%d → %d: %s Running %s.\n"
#define IDLE_MSG "%d → %d: Idle.\n"

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
void sigalarm_ignore();
int cmpFCFS(__attribute__((unused)) Process p1, Process p2);
int cmpLowerPriority(Process p1, Process p2);
int cmpBurstTime(Process p1, Process p2);
int cmpArrival(Process p1, Process p2);
void Enqueue(ReadyQueue* q, Process p);
Process Dequeue(ReadyQueue* q);
void EnqueueNewArrivals(ReadyQueue* queue, Process procs[], int* startIndex, int count, struct timespec startingTime);
int GetTotalTime(struct timespec starting);
void RunScheduler(Scheduler scheduler, Process procs[], int count);
void SortProcs (Process procs[], int count, int  (cmpProcs) (Process p1, Process p2));


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
    SJF.CmpProcs = cmpBurstTime;
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
    RoundRobin.CmpProcs = cmpArrival;
    RoundRobin.CalculateTurnAround = true;
    RoundRobin.timeQuantum = timeQuantum;
    RunScheduler(RoundRobin, procs, count);
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
    while (fgets(L, MAX_LINE, file) != NULL) {
        sscanf(L, "%[^,],%[^,],%d,%d,%d", P[*count].name, P[*count].description, &P[*count].arrival_time,
               &P[*count].burst_time, &P[*count].priority);
        P[*count].index = *count;
        (*count)++;
    }
    if (ferror(file)) {
        perror("fscanf error");
        exit(1);
    }
    fclose(file);
    SortProcs(P, *count, cmpArrival);
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

    strcpy(process.name, strtok_r(dup, ",", &save_ptr));
    strcpy(process.description, strtok_r(dup, ",", &save_ptr));
    process.arrival_time = atoi(strtok_r(dup, ",", &save_ptr));
    process.burst_time = atoi(strtok_r(dup, ",", &save_ptr));
    process.priority = atoi(strtok_r(dup, ",", &save_ptr));
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
    act.sa_handler = sigalarm_ignore;
    sigaction(SIGALRM, &act, NULL);
    // print intro
    printf(INTRO, scheduler.name);
    fflush(stdout);

    // start timer
    struct timespec startT;
    clock_gettime(CLOCK_MONOTONIC, &startT);

    /**
     * Main loop: enqueue new arrivals and check if the current process is done
    */
    int starting = 0;
    Process currProc;
    bool isRunning = false;
    bool isIdle = false;
    struct timespec procStartTime;
    bool isProcArrived = starting >= count;
    int turnAround = 0;
    int totalWaiting = 0;
    int iter = 0;
    int idleStart = -1;

    while ((!isProcArrived) || (Q.size > 0) || isRunning) {
        isProcArrived = starting >= count;
        if (!isProcArrived) {
            EnqueueNewArrivals(&Q, procs, &starting, count, startT);
            isProcArrived = starting >= count;
        }
        if (isRunning) {
            int processUptime = (int)GetTotalTime(procStartTime);
            int schedulerUptime = (int)GetTotalTime(startT);
            if (processUptime >= currProc.burst_time) {
                totalWaiting += schedulerUptime - currProc.burst_time - currProc.arrival_time;
                isRunning = false;

                printf(RUNNING_MSG, schedulerUptime - currProc.burst_time, schedulerUptime, currProc.name, currProc.description);
                fflush(stdout);

                if (isProcArrived && (Q.size == 0)) {
                    turnAround = (int)GetTotalTime(startT);
                    break;
                }
            }
            else if (scheduler.timeQuantum != -1) {
                if (processUptime >= scheduler.timeQuantum) {
                    totalWaiting += schedulerUptime - scheduler.timeQuantum - currProc.arrival_time;
                    isRunning = false;

                    printf(RUNNING_MSG, schedulerUptime - scheduler.timeQuantum, schedulerUptime, currProc.name, currProc.description);
                    fflush(stdout);

                    Process modifiedProcess = currProc;
                    modifiedProcess.arrival_time = schedulerUptime;
                    modifiedProcess.burst_time -= scheduler.timeQuantum;
                    // Give the process that finished its time quantum priority over new arrivals by setting its index to a negative value based on its original index
                    modifiedProcess.index = -(count + currProc.index);
                    Enqueue(&Q, modifiedProcess);
                }
            }
        }


        if (!isRunning && (Q.size > 0)) {
            if (isIdle) {
                int schedulerUptime = (int)GetTotalTime(startT);
                printf(IDLE_MSG, idleStart, schedulerUptime);
                fflush(stdout);
                isIdle = false;
                idleStart = -1;
            }
            isRunning = true;
            clock_gettime(CLOCK_MONOTONIC, &procStartTime);
            currProc = Dequeue(&Q);
        }

        if (!isRunning && (Q.size == 0) && (!isProcArrived) && !isIdle) {
            isIdle = true;
            idleStart = (int)GetTotalTime(startT);
        }
        iter++;
        ualarm((int)1e5, 0);
        pause();
    }

    // print outro
    if (scheduler.CalculateTurnAround) {
        printf(OUTRO_TA, turnAround); // add the parameters
        fflush(stdout);
    }
    else {
        printf(OUTRO_TW, (double) totalWaiting / count); // add the parameters
        fflush(stdout);
    }
}

int GetTotalTime(struct timespec starting) {
    struct timespec current;
    clock_gettime(CLOCK_MONOTONIC, &current);
    return (int) (current.tv_sec - starting.tv_sec);
}

void Enqueue(ReadyQueue* q, Process p) {
    q->procs[q->size] = p;
    q->size++;
    SortProcs(q->procs, q->size, q->CmpProcs);  // re-sort
}

Process Dequeue(ReadyQueue* q) {
    if (q->size == 0) {
        perror("queue is empty");
        exit(1);
    }
    Process first = q->procs[0];
    q->size--;
    for (int i = 0; i < q->size; i++)
        q->procs[i] = q->procs[i+1];

    return first;
}

void EnqueueNewArrivals(ReadyQueue* queue, Process procs[], int* startIndex, int count, struct timespec startingTime) {
    int current = (int)GetTotalTime(startingTime);
    // add to ready queue
    for (; *startIndex < count; (*startIndex)++) {
        if (procs[*startIndex].arrival_time <= current)
            Enqueue(queue, procs[*startIndex]);
        else break;
    }
}

void SortProcs (Process procs[], int count, int  (cmpProcs) (Process p1, Process p2)) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - 1; j++) {
            if (cmpProcs(procs[j], procs[j+1]) > 0) { // swap
                Process temp = procs[j];
                procs[j] = procs[j+1];
                procs[j+1] = temp;
            }
        }
    }
}

void sigalarm_ignore() {}

int cmpFCFS(__attribute__((unused)) Process p1, __attribute__((unused)) Process p2) {
    return 0;
}
int cmpLowerPriority(Process p1, Process p2) {
    if (p1.priority != p2.priority)
        return p1.priority - p2.priority;
    return cmpArrival(p1,p2);
}
int cmpBurstTime(Process p1, Process p2) {
    if (p1.burst_time != p2.burst_time)
        return p1.burst_time - p2.burst_time;
    return cmpArrival(p1,p2);
}
int cmpArrival(Process p1, Process p2) {
    if (p1.arrival_time != p2.arrival_time)
        return p1.arrival_time - p2.arrival_time;
    return p1.index - p2.index;
}
