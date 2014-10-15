#ifndef CSOCKETCOMMANDSERVICE_H
#define	CSOCKETCOMMANDSERVICE_H

#include "thread.h"

class CSocketCommandService : public CThread
{
public:
    CSocketCommandService();
    virtual ~CSocketCommandService();
protected:
    virtual int Run();
private:

};

#endif	/* CSOCKETCOMMANDSERVICE_H */

