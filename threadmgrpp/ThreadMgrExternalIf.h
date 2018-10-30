#ifndef _THREAD_MGR_EXTERNAL_IF_H_
#define _THREAD_MGR_EXTERNAL_IF_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "threadmgr_if.h"

using namespace std;


namespace ThreadManager {

class CThreadMgrExternalIf 
{
public:
	CThreadMgrExternalIf (ST_THM_EXTERNAL_IF *pExtIf);
	virtual ~CThreadMgrExternalIf (void);


	bool requestSync (uint8_t nThreadIdx, uint8_t nSeqIdx, uint8_t *pszMsg);
	bool requestAsync (uint8_t nThreadIdx, uint8_t nSeqIdx, uint8_t *pszMsg, uint32_t *pnReqId);
	bool createExternalCp (void);
	void destroyExternalCp (void);
	ST_THM_SRC_INFO* receiveExternal (void);
	void finalize (void);

private:
	ST_THM_EXTERNAL_IF *mpExtIf;
};

} // namespace ThreadManager

#endif
