#ifndef _CTHREADMGR_BASE_H_
#define _CTHREADMGR_BASE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "threadmgr_if.h"
#include "threadmgr_util.h"

#include "ThreadMgrIf.h"


namespace ThreadManager {

class CThreadMgr;
class CThreadMgrBase;
typedef void (CThreadMgrBase:: *PFN_SEQ_BASE) (CThreadMgrIf *pIf);

class CThreadMgrBase
{
public:
	friend class CThreadMgr;


	CThreadMgrBase (char *pszName, uint8_t nQueNum);
	virtual ~CThreadMgrBase (void);


	void exec (EN_THM_DISPATCH_TYPE enType, uint8_t nSeqIdx, ST_THM_IF *pIf);


protected:
	void setSeqs (PFN_SEQ_BASE pfnSeqs [], uint8_t seqNum);

	virtual void onCreate (void);
	virtual void onDestroy (void);
	virtual void onReceiveNotify (CThreadMgrIf *pIf);


private:
	PFN_SEQ_BASE *mpfnSeqsBase ; // double pointer

	char mName [16]; //THREAD_NAME_SIZE
	uint8_t mQueNum;
	uint8_t mSeqNum;
	
};

} // namespace ThreadManager

#endif
