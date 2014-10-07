#include "application.h"

#include <unistd.h>
#include <signal.h>

// Global application instance
CApplication g_Application;

// Process signal handler
static void handle_signal(int signal, siginfo_t *info, void *unused)
{
    switch (signal)
    {
    case SIGHUP:
        g_Application.ReloadConfig();
        break;
    case SIGINT:
    case SIGTERM:
        g_Application.Quit();
        // TODO: How do we know when it's done...?
        break;
    default:
        break;        
    };
}

// Workaround for namespace conflict (function and type with the same name)
typedef struct sigaction sigaction_t;

bool config_handlers()
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

int logwrite(const char* format, ...)
{
    return 0;
}

int main(int argc, char *argv[])
{
    bool daemonize = false;
    
    // TODO: Parse args
    
    // Set-up signal handlers (for daemon control)
    config_handlers();
    
    // Initialize application
    printf("Agile Process Control Server: Initializing...\n");

    if (!g_Application.Initialize(daemonize))
    {
        printf("Initialization Failed.\r\n");
        exit(EXIT_FAILURE);
    }
    printf("Agile Process Control Server: Initialized\n");
    
    if (daemonize)
    {
        printf("Agile Process Control Server: Daemonizing...");
        if (daemon(0,0)) // Failed to detach process
        {
            printf("FAILED\r\n");
            exit(EXIT_FAILURE);
        }
    }

    // TODO: Write PID to file (for daemon control)
    
    // Run the application
    return g_Application.Run(); 
}