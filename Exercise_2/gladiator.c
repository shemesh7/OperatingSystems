// gladiator.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define NUM_OPPONENTS 3

int get_opponent_attack(int opponent) {
    char filename[10];
    sprintf(filename, "G%d.txt", opponent);

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open opponent file");
        exit(1);
    }

    int health, attack;
    fscanf(file, "%d, %d", &health, &attack);
    fclose(file);

    return attack;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <gladiator_file>\n", argv[0]);
        exit(1);
    }

    char* gladiator_file = argv[1];

    FILE *file = fopen(gladiator_file, "r");
    if (!file) {
        perror("Failed to open gladiator file");
        exit(1);
    }

    int health, attack, opponents[NUM_OPPONENTS];
    fscanf(file, "%d, %d, %d, %d, %d", &health, &attack, &opponents[0], &opponents[1], &opponents[2]);
    fclose(file);

    // Open log file
    char log_filename[20];
    strncpy(log_filename, gladiator_file, 2);   // Copy "Gi"
    log_filename[2] = '\0';                     // End the string
    strcat(log_filename, "_log.txt");           // Add "_log.txt"

    FILE *logFile = fopen(log_filename, "w");
    if (!logFile) {
        perror("Failed to open log file");
        exit(1);
    }

    fprintf(logFile, "Gladiator process started. %d:\n", getpid());

    while (health > 0) {
        for (int i = 0; i < NUM_OPPONENTS; i++) {
            int opponent_attack = get_opponent_attack(opponents[i]);
            fprintf(logFile, "Facing opponent %d... Taking %d damage\n", opponents[i], opponent_attack);
            health -= opponent_attack;
            if (health > 0) {
                fprintf(logFile, "Are you not entertained? Remaining health: %d\n", health);
            } else {
                fprintf(logFile, "The gladiator has fallen... Final health: %d\n", health);
                break;
            }
        }
    }

    fclose(logFile);
    return 0;
}