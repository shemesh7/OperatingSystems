#include <stdbool.h>
#include <curses.h>
#include <sys/signal.h>

// Define signals for the 3 distractions
#define EMAIL_SIGNAL = SIGUSR1
#define DELIVERY_SIGNAL = SIGUSR2
#define DOORBELL_SIGNAL = SIGTERM


void runFocusMode(int numOfRounds, int duration) {

}
