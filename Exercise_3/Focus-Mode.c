#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

// Signal definitions for the 3 distractions
#define EMAIL_SIGNAL SIGUSR1        // Signal for email notification
#define PICKUP_SIGNAL SIGUSR2     // Signal for delivery reminder  
#define DOORBELL_SIGNAL SIGTERM     // Signal for doorbell

#define PENDING_MSG "──────────────────────────────────────────────\n" \
"        Checking pending distractions...      \n"                     \
"──────────────────────────────────────────────\n"

#define BACK_TO_FOCUS "──────────────────────────────────────────────\n" \
"             Back to Focus Mode.              \n" \
"══════════════════════════════════════════════\n"

#define FOCUS_ROUND "══════════════════════════════════════════════\n" \
"                Focus Round %d                \n" \
"──────────────────────────────────────────────\n"

#define SIMULATE_OPTIONS "\nSimulate a distraction:\n" \
"  1 = Email notification\n" \
"  2 = Reminder to pick up delivery\n" \
"  3 = Doorbell Ringing\n" \
"  q = Quit\n" \
">> "

// signal handlers won't be called during focus time so can be empty
void email_handler(int sig) {}
void delivery_handler(int sig) {}
void doorbell_handler(int sig) {}

// helper functions
void run_focus_round(int round_number, int duration);
void handle_pending_distractions();
void setup_signal_handlers();


// main
void runFocusMode(int numOfRounds, int roundDuration) {
    setup_signal_handlers();

    // only the 3 distraction signals
    sigset_t block_set;
    sigemptyset(&block_set);
    sigaddset(&block_set, EMAIL_SIGNAL);
    sigaddset(&block_set, PICKUP_SIGNAL);
    sigaddset(&block_set, DOORBELL_SIGNAL);

    printf("Entering Focus Mode. All distractions are blocked.\n");
    fflush(stdout);

    // block them
    sigprocmask(SIG_BLOCK, &block_set, NULL);

    for (int r = 1; r <= numOfRounds; r++)
        run_focus_round(r, roundDuration);

    // Unblock signals
    sigprocmask(SIG_UNBLOCK, &block_set, NULL);
    printf("\nFocus Mode complete. All distractions are now unblocked.\n");
    fflush(stdout);
}


void setup_signal_handlers() {
    struct sigaction email_action, delivery_action, doorbell_action;

    // Setup email handler
    email_action.sa_handler = email_handler;
    sigemptyset(&email_action.sa_mask);     // no signals should be blocked
    email_action.sa_flags = 0;
    sigaction(EMAIL_SIGNAL, &email_action, NULL);

    // Setup delivery handler
    delivery_action.sa_handler = delivery_handler;
    sigemptyset(&delivery_action.sa_mask);  // no signals should be blocked
    delivery_action.sa_flags = 0;
    sigaction(PICKUP_SIGNAL, &delivery_action, NULL);

    // Setup doorbell handler
    doorbell_action.sa_handler = doorbell_handler;
    sigemptyset(&doorbell_action.sa_mask);  // no signals should be blocked
    doorbell_action.sa_flags = 0;
    sigaction(DOORBELL_SIGNAL, &doorbell_action, NULL);
}


void handle_pending_distractions() {
    printf(PENDING_MSG);
    fflush(stdout);

    sigset_t pending_set;
    sigemptyset(&pending_set);
    sigpending(&pending_set);

    int distraction_found = 0;

    // check signals in order
    if (sigismember(&pending_set, EMAIL_SIGNAL)) {
        printf(" - Email notification is waiting.\n");
        fflush(stdout);
        printf("[Outcome:] The TA announced: Everyone get 100 on the exercise!\n");
        fflush(stdout);
        distraction_found = 1;
    }

    if (sigismember(&pending_set, PICKUP_SIGNAL)) {
        printf(" - You have a reminder to pick up your delivery.\n");
        fflush(stdout);
        printf("[Outcome:] You picked it up just in time.\n");
        fflush(stdout);
        distraction_found = 1;
    }

    if (sigismember(&pending_set, DOORBELL_SIGNAL)) {
        printf(" - The doorbell is ringing.\n");
        fflush(stdout);
        printf("[Outcome:] Food delivery is here.\n");
        fflush(stdout);
        distraction_found = 1;
    }

    if (!distraction_found) {
        printf("No distractions reached you this round.\n");
        fflush(stdout);
    }

    // free the pending signals
    sigset_t empty;
    sigemptyset(&empty);
    sigprocmask(SIG_SETMASK, &empty, NULL);

    printf(BACK_TO_FOCUS);
    fflush(stdout);

    // block the 3 distraction signals again
    sigset_t block_set;
    sigemptyset(&block_set);
    sigaddset(&block_set, EMAIL_SIGNAL);
    sigaddset(&block_set, PICKUP_SIGNAL);
    sigaddset(&block_set, DOORBELL_SIGNAL);
    sigprocmask(SIG_SETMASK, &block_set, NULL);
}

// a single focus round
void run_focus_round(int round_number, int duration) {
    printf(FOCUS_ROUND, round_number);
    fflush(stdout);

    for (int i = 0; i < duration; i++) {
        printf(SIMULATE_OPTIONS);
        fflush(stdout);

        char user_input;
        scanf(" %c", &user_input);
        if (user_input == 'q') {         //quit
            break;
        }
        else if (user_input == '1') {    //email
            kill(getpid(), EMAIL_SIGNAL);
        }
        else if (user_input == '2') {    //pickup
            kill(getpid(), PICKUP_SIGNAL);
        }
        else if (user_input == '3') {    //doorbell
            kill(getpid(), DOORBELL_SIGNAL);
        }
        // ignore invalid input
    }
    handle_pending_distractions();
}
