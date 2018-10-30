#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgr.h"


namespace ThreadManager {

/*
 * Constant define
 */
// ここは threadmgr.c の宣言と同じ値にする
#define THREAD_IDX_MAX					(0x20) // 32  uint8の為 255以下の値で THREAD_IDX_BLANKより小さい値
#define SEQ_IDX_MAX						(0x40) // 64  uint8の為 255以下の値で SEQ_IDX_BLANKより小さい値
#define QUE_WORKER_MIN					(5)
#define QUE_WORKER_MAX					(100)


static void dispatcher (
	EN_THM_DISPATCH_TYPE enType,
	uint8_t nThreadIdx,
	uint8_t nSeqIdx,
	ST_THM_IF *pIf
);


CThreadMgrBase *gpThreads [THREAD_IDX_MAX];


CThreadMgr::CThreadMgr (void)
{
}

CThreadMgr:: ~CThreadMgr (void)
{
}


CThreadMgr* CThreadMgr::getInstance (void)
{
	static CThreadMgr singleton;
	return &singleton;
}

bool CThreadMgr::registerThreads (CThreadMgrBase *pThreads[], int threadNum)
{
	if ((!pThreads) || (threadNum <= 0)) {
		THM_LOG_E ("invalid argument.\n");
		return false;
	}

	int i = 0;
	while (i < threadNum) {
		if (i == THREAD_IDX_MAX) {
			// over flow
			THM_LOG_E ("thread idx is over. (inner thread table)\n");
			return false;
		}

//		if ((pThreads [i]->nQueNum < QUE_WORKER_MIN) && (pThreads [i]->nQueNum > QUE_WORKER_MAX)) {
//			THM_LOG_E ("que num is invalid. (inner thread table)\n");
//			return false;
//		}

//		if ((pThreads [i]->nSeqNum <= 0) && (pThreads [i]->nSeqNum > SEQ_IDX_MAX)) {
//			THM_LOG_E ("func idx is invalid. (inner thread table)\n");
//			return false;
//		}

		gpThreads [i] = pThreads [i];
		++ i;
	}

	return true;
}

bool CThreadMgr::setup (CThreadMgrBase *pThreads[], int threadNum)
{
	if (!registerThreads (pThreads, threadNum)) {
		THM_LOG_E ("registerThreads() is failure.\n");
		return false;
	}

	mThreadNum = threadNum;

	setupDispatcher (dispatcher);

	ST_THM_REG_TBL *pTbl = (ST_THM_REG_TBL*) malloc (sizeof(ST_THM_REG_TBL) * mThreadNum);
	if (!pTbl) {
		THM_PERROR ("malloc");
		return false;
	}

	for (int i = 0; i < mThreadNum; ++ i) {
		ST_THM_REG_TBL *p = pTbl + i;

		p->pszName = gpThreads[i]->mName;
		const_cast <PCB_CREATE&> (p->pcbCreate) = NULL;				// not use
		const_cast <PCB_DESTROY&> (p->pcbDestroy) = NULL;			// not use
		p->nQueNum = gpThreads[i]->mQueNum;
		p->pcbSeqArray = NULL;										// not use
		p->nSeqNum = gpThreads[i]->mSeqNum;
		const_cast <PCB_RECV_NOTIFY&> (p->pcbRecvNotify) = NULL;	// not use
	}

	ST_THM_EXTERNAL_IF* pExtIf = setupThreadMgr(pTbl, (uint32_t)mThreadNum);
	if (!pExtIf) {
		THM_LOG_E ("setupThreadMgr() is failure.\n");
		return false;
	}

	mpExtIf = new CThreadMgrExternalIf (pExtIf);
	if (!mpExtIf) {
		THM_LOG_E ("mpExtIf is null.\n");
		return false;
	}

	return true;
}

void CThreadMgr::teardown (void) {
	if (mpExtIf) {
		mpExtIf->finalize ();

		delete mpExtIf;
		mpExtIf = NULL;
	}
}

CThreadMgrExternalIf * CThreadMgr::getExternalIf (void) {
	return mpExtIf;
}

static void dispatcher (
	EN_THM_DISPATCH_TYPE enType,
	uint8_t nThreadIdx,
	uint8_t nSeqIdx,
	ST_THM_IF *pIf
) {
	CThreadMgrBase *pBase = gpThreads [nThreadIdx];
	if (!pBase) {
		THM_LOG_E ("gpThreads [%d] is null.", nThreadIdx);
		return ;
	}

	///////////////////////////////

	pBase->exec (enType, nSeqIdx, pIf);

	///////////////////////////////
}

} // namespace ThreadManager
