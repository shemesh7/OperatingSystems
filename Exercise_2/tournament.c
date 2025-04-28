// tournament.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define NUM_GLADIATORS 4

int main() {
    char* gladiator_names[NUM_GLADIATORS] = {"Maximus", "Lucius", "Commodus", "Spartacus"};
    char* gladiator_files[NUM_GLADIATORS] = {"G1.txt", "G2.txt", "G3.txt", "G4.txt"};
    pid_t pid_arr[NUM_GLADIATORS];
    int i;

    for (i = 0; i < NUM_GLADIATORS; i++) {
        pid_arr[i] = fork();
        if (pid_arr[i] < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        if (pid_arr[i] == 0) {                      // Child process
            execl("./gladiator", "gladiator", gladiator_files[i], (char*)NULL);
            perror("exec failed");
            exit(EXIT_FAILURE);
        }
    }

    int status;
    pid_t last_gladiator = 0;
    for (i = 0; i < NUM_GLADIATORS; i++) {
        pid_t ended_pid = wait(&status);
        last_gladiator = ended_pid;
    }

    for (i = 0; i < NUM_GLADIATORS; i++) {
        if (pid_arr[i] == last_gladiator) {
            printf("The gods have spoken, the winner of the tournament is %s!\n", gladiator_names[i]);
            break;
        }
    }

    return 0;
}