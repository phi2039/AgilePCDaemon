#include <unistd.h>
#include <signal.h>
#include <stdio.h>

#include "application.h"

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

int main(int argc, char *argv[])
{
    // TODO: Parse args
    // - Config File
    // - Logging
    // - Daemon
    
    // TODO: See if an instance is already running
    // - PID File
    // - Process List?
    
    bool daemonize = false;
    
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

    // Write PID to file (for daemon control)
    const char* pidFileName = "/var/run/agilepcd.pid";
    FILE* pidFile = fopen(pidFileName, "w");
    if (!pidFile)
    {
        // Probably need to open before daemonizing to make sure we have it...
        // But daemonizing will close it, so...
        // TODO: How do we want to handle this...?
    }
    else
    {
        pid_t pid = getpid(); // Get PID
        fprintf(pidFile,"%d\n", pid);
        fclose(pidFile);
    }
    // Run the application
    int result = g_Application.Run(); 
    
    printf("Agile Process Control Server: Exiting...");
    // Remove the PID file (or at least try to...)
    remove(pidFileName);
    
    return result;
}