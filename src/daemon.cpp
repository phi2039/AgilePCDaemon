// Linux only...

#include "daemon.h"
#include "application.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

using namespace std;

int daemon_main(int argc, char *argv[])
{
    printf("Daemonizing...\r\n");
    // Fork the parent (calling) process and create a new process
    // This single call returns twice (once in the parent and once in the child)
    // In the parent, the return value is the original PID for success, 
    // or a negative number for failure. In the child, the return value is zero.
    
    pid_t pid = fork();
    if (pid < 0) // Failed to fork
        exit(EXIT_FAILURE); // Close the parent process with failure

    if (pid > 0) // This is the parent process
        exit(EXIT_SUCCESS); // Close the parent process with success
    
    // Continue on in the child process. The parent is now gone...
    
    //Change File Mask
    umask(0);
    
    // Create a new Session 
    // (detach from the original parent process and attach to init)
    pid_t sid = setsid();
    if (sid < 0) // Failed create a new Session
        exit(EXIT_FAILURE);
    
    //Change to target working directory
    if ((chdir("/")) < 0) // Failed to find the working directory
        exit(EXIT_FAILURE);
    
    // Close standard file descriptors (inherited from parent), and go headless
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);    

    CApplication app;
    return app.Run(); 
}