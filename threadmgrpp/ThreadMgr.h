#ifndef _CTHREADMGR_H_
#define _CTHREADMGR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "threadmgr_if.h"
#include "threadmgr_util.h"

#include "ThreadMgrIf.h"
#include "ThreadMgrExternalIf.h"
#include "ThreadMgrBase.h"


class CThreadMgr
{
public:
	CThreadMgr (void);
	virtual ~CThreadMgr (void);


	static CThreadMgr *getInstance (void);

	bool registerThreads (CThreadMgrBase *pThreads[], int threadNum);
	bool setup (CThreadMgrBase *pThreads[], int threadNum);
	void teardown (void);

	CThreadMgrExternalIf * getExternalIf (void);


private:
	int mThreadNum;

	CThreadMgrExternalIf *mpExtIf;

};

#endif
