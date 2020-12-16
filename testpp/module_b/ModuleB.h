#ifndef _MODULEB_H_
#define _MODULEB_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrpp.h"

#include "ModuleB_extern.h"


class CModuleB : public ThreadManager::CThreadMgrBase
{
public:
	CModuleB (std::string name, uint8_t nQueNum);
	virtual ~CModuleB (void);

private:
	void startUp (ThreadManager::CThreadMgrIf *pIf);
	void echo00 (ThreadManager::CThreadMgrIf *pIf);
	void echo01 (ThreadManager::CThreadMgrIf *pIf);
	void echo02 (ThreadManager::CThreadMgrIf *pIf);
};

#endif
