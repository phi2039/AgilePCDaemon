#include "application.h"

#include <unistd.h>
#include <signal.h>

CApplication g_Application;

// Process signal handler
static void handle_signal(int signal, siginfo_t *info, void *unused)
{
    switch (signal)
    {
    case SIGHUP:
        // TODO: Reload configuration file
        break;
    case SIGINT:
    case SIGTERM:
        printf("Quitting...\r\n");
        g_Application.Quit();
        // TODO: How do we know when it's done...?
        break;
    default:
        break;        
    };
}

typedef struct sigaction sigaction_t; // Workaround for namespace conflict

bool config_handler()
{
    // Set up signal handlers
    sigaction_t action;
    action.sa_sigaction = handle_signal;
    sigemptyset(&action.sa_mask); // No mask
    action.sa_flags = 0; // No flags
    
    sigaction(SIGTERM, &action, NULL); // Handle Terminate signal
    sigaction(SIGINT, &action, NULL); // Handle Interrupt signal
    sigaction(SIGHUP, &action, NULL); // Handle Hangup signal
}

int main(int argc, char *argv[])
{
    // Install signal handler
    // TODO: Should NOT need struct specifier here...
    sigaction_t action; // Have to use 
    action.sa_flags = SA_SIGINFO; // Use a handler for all signals, rather than the default action
    sigemptyset(&action.sa_mask); // Don't block any signals
    action.sa_sigaction = handle_signal;
    
    // TODO: Install handler for other signals
    if (sigaction(SIGTERM, &action, NULL) == -1)
        return 1;
    
    // Initialize application
    printf("Agile Process Control Server: Initializing...\r\n");
    if (!g_Application.Initialize())
    {
        printf("Initialization Failed.\r\n");
        exit(EXIT_FAILURE);
    }
    printf("Initialized\r\n");
    
//    printf("\tDaemonizing...\r\n");
//    if (daemon(0,0)) // TODO: Use custom daemon code for portability??
//    {
//        printf("FAILED\r\n");
//        exit(EXIT_FAILURE);
//    }
    
    // TODO: Write PID to file
    
    // Run the application
    return g_Application.Run(); 
}