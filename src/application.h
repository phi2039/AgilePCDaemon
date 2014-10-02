#ifndef APPLICATION_H
#define APPLICATION_H

#include "datamanager.h"

#include <string>

class CApplication
{
public:
	CApplication();
        bool Initialize();
	virtual ~CApplication();
	int Run();
        void Quit();
protected:
	CDataManager* m_pDataManager;
	bool m_QuitFlag;
private:

};

#endif // APPLICATION_H