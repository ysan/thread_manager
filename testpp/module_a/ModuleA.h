#ifndef _MODULEA_H_
#define _MODULEA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <string>
#include <sstream>

#include "ThreadMgrpp.h"

#include "ModuleA_extern.h"


class CModuleA : public ThreadManager::CThreadMgrBase
{
public:
	CModuleA (std::string name, uint8_t nQueNum);
	virtual ~CModuleA (void);


private:
	void startUp (ThreadManager::CThreadMgrIf *pIf);
	void testReqRep (ThreadManager::CThreadMgrIf *pIf);
	void testReqRepNotify (ThreadManager::CThreadMgrIf *pIf);
	void testLock (ThreadManager::CThreadMgrIf *pIf);
	void testLockIntr (ThreadManager::CThreadMgrIf *pIf);
	void testOverwrite (ThreadManager::CThreadMgrIf *pIf);
	void testDestroy (ThreadManager::CThreadMgrIf *pIf);

	void onReceiveNotify (ThreadManager::CThreadMgrIf *pIf);


	uint32_t m_tmp_req_id;
	uint8_t m_client_id;
	bool m_is_notified;
	std::stringstream m_lock_check;
	std::stringstream m_ow_check;
};

#endif
