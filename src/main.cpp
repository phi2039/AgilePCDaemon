#include "application.h"
#include "daemon.h"

int main(int argc, char *argv[])
{
    // TODO: Initialize, then daemonize, then run...
    return daemon_main(argc, argv);
}
