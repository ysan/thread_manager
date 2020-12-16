#ifndef _MODULEC_H_
#define _MODULEC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrpp.h"

#include "ModuleC_extern.h"


class CModuleC : public ThreadManager::CThreadMgrBase
{
public:
	CModuleC (std::string name, uint8_t nQueNum);
	virtual ~CModuleC (void);


private:
	void startUp (ThreadManager::CThreadMgrIf *pIf) ;
	void testRegNotify (ThreadManager::CThreadMgrIf *pIf) ;
	void testUnregNotify (ThreadManager::CThreadMgrIf *pIf) ;
	void testNotify (ThreadManager::CThreadMgrIf *pIf) ;
	void echo00 (ThreadManager::CThreadMgrIf *pIf) ;
	void echo01 (ThreadManager::CThreadMgrIf *pIf) ;
	void echo02 (ThreadManager::CThreadMgrIf *pIf) ;

};

#endif
