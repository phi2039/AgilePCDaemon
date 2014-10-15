#include <unistd.h>

#include "SocketCommandService.h"
#include "logging.h"

CSocketCommandService::CSocketCommandService()
{
    
}

CSocketCommandService::~CSocketCommandService()
{
    
}

int CSocketCommandService::Run()
{
    CLog::Write(APC_LOG_FLAG_INFO, "SocketCommandService", "Starting Socket Command Service");
    
    // Open the socket for communication
    
    // Listen for and service connections
    // TODO: There must be a better way to do this...
    while (true)
    {
        // TODO: Use wait with timeout instead of sleep, so that we can trigger a speedier exit...
        sleep(1);

        // Check for new connections
        // Wait on connection OR quit event
        
        // Read command
        
        // Parse command
        
        // Pass command to handler
        
        // If response requested, register callback
        
        // If no response requested close connection
        
        if (m_QuitFlag)
          break;

        CLog::Write(APC_LOG_FLAG_INFO, "SocketCommandService", "Stopping Socket Command Service");

        return 0;
    }
}