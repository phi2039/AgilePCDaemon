#ifndef SERVICE_H
#define	SERVICE_H

#include <vector>

class IService 
{
public:
    virtual ~IService() {}
    virtual bool Initialize() = 0;
    virtual bool Start() = 0;
    virtual void Stop() = 0;
    virtual void SendMessage() = 0;
protected:
private:
};

class IServiceManager
{
public:
    virtual ~IServiceManager() {}
protected:
private:
};

#endif	/* SERVICE_H */

