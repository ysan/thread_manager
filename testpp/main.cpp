#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "threadmgr_if.h"
#include "threadmgr_util.h"

#include "ThreadMgrIf.h"
#include "ThreadMgrExternalIf.h"


static void dispatcher (
	EN_THM_DISPATCH_TYPE enType,
	uint8_t nThreadIdx,
	uint8_t nSeqIdx,
	ST_THM_IF *pIf
);


class CThreadMgrBase;
typedef void (CThreadMgrBase:: *PFN_SEQ_BASE) (CThreadMgrIf *pIf);

class CThreadMgrBase
{
public:
	CThreadMgrBase (void) {}
	virtual ~CThreadMgrBase (void) {}


	void exec (EN_THM_DISPATCH_TYPE enType, uint8_t nSeqIdx, ST_THM_IF *pIf) {

		switch (enType) {
		case EN_THM_DISPATCH_TYPE_CREATE:
			onCreate ();
			break;

		case EN_THM_DISPATCH_TYPE_DESTROY:
			onDestroy ();
			break;

		case EN_THM_DISPATCH_TYPE_REQ_REPLY:
			{
			CThreadMgrIf thmIf (pIf);
			(void) (this->*mpfnSeqsBase [nSeqIdx]) (&thmIf);
			break;
			}

		case EN_THM_DISPATCH_TYPE_NOTIFY:
			{
			CThreadMgrIf thmIf (pIf);
			onReceiveNotify (&thmIf);
			break;
			}

		default:
			puts ("BUG");
			break;
		}
	}


protected:
	void setSeqs (PFN_SEQ_BASE pfnSeqs []) {
		if (pfnSeqs) {
			mpfnSeqsBase = pfnSeqs;
		}
	}

	virtual void onCreate (void) {
	}

	virtual void onDestroy (void) {
	}

	virtual void onReceiveNotify (CThreadMgrIf *pIf) {
	}


private:
	PFN_SEQ_BASE *mpfnSeqsBase ; // double pointer

};

class CThreadMgr;
CThreadMgr *gpMgr = NULL;
class CThreadMgr
{
public:
	CThreadMgr (int n) {
		mThreadNum = n;
	}
	virtual ~CThreadMgr (void) {}


	bool init (void) {
		if (mThreadNum < 1) {
			// error
			return false;
		}

		setupDispatcher (dispatcher);

		ST_THM_REG_TBL *pTbl = (ST_THM_REG_TBL*) malloc (sizeof(ST_THM_REG_TBL) * mThreadNum);
		if (!pTbl) {
			// error
			return false;
		}

		for (int i = 0; i < mThreadNum; ++ i) {
			ST_THM_REG_TBL *p = pTbl + i;

			p->pszName = "ModuleA";
			const_cast <PCB_CREATE&> (p->pcbCreate) = NULL; // not use
			const_cast <PCB_DESTROY&> (p->pcbDestroy) = NULL; // not use
			p->nQueNum = 10;
			p->pcbSeqArray = NULL; // not use
			p->nSeqNum = 3;
			const_cast <PCB_RECV_NOTIFY&> (p->pcbRecvNotify) = NULL; // not use
		}

		ST_THM_EXTERNAL_IF* pExtIf = setupThreadMgr(pTbl, (uint32_t)mThreadNum);
		if (!pExtIf) {
			// error
			return false;
		}

		mpExtIf = new CThreadMgrExternalIf (pExtIf);
		if (!mpExtIf) {
			// error
			return false;
		}

		return true;
	}

	void finaliz (void) {
		if (mpExtIf) {
			delete mpExtIf;
			mpExtIf = NULL;
		}
	}

	CThreadMgrExternalIf * getExternalIf (void) {
		return mpExtIf;
	}


private:
	int mThreadNum;
	CThreadMgrExternalIf *mpExtIf;

};

class CModuleA : public CThreadMgrBase
{
public:
	CModuleA (void) {
		mpfnSeqs [0] = PFN_SEQ_BASE (&CModuleA::startUp);
		mpfnSeqs [1] = PFN_SEQ_BASE (&CModuleA::func00);
		mpfnSeqs [2] = PFN_SEQ_BASE (&CModuleA::func01);
		setSeqs (mpfnSeqs);
	}
	virtual ~CModuleA (void) {}

	void startUp (CThreadMgrIf *pIf) {
		uint8_t nSectId;
		EN_THM_ACT enAct;
		enum {
			SECTID_ENTRY = THM_SECT_ID_INIT,
			SECTID_END,
		};

		nSectId = pIf->getSectId();
		THM_LOG_I ("nSectId %d\n", nSectId);

		pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)"ModuleA startup end.");

		nSectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		pIf->setSectId (nSectId, enAct);
	}

	void func00 (CThreadMgrIf *pIf)
	{
		uint8_t nSectId;
		EN_THM_ACT enAct;
		enum {
			SECTID_ENTRY = THM_SECT_ID_INIT,
			SECTID_REQ_THREAD_A_FUNC01,
			SECTID_WAIT_THREAD_A_FUNC01,
			SECTID_END,
			SECTID_ERR_END,
		};

		nSectId = pIf->getSectId();
		THM_LOG_I ("nSectId %d\n", nSectId);

		switch (nSectId) {
		case SECTID_ENTRY:
			THM_LOG_I ("msg [%s]\n", pIf->getSrcInfo()->pszMsg);

			nSectId = SECTID_REQ_THREAD_A_FUNC01;
			enAct = EN_THM_ACT_CONTINUE;
			break;

		case SECTID_REQ_THREAD_A_FUNC01:

			// 自スレのfunc01にリクエスト
//			reqFunc01ThreadA(NULL);
			gpMgr->getExternalIf()->requestAsync (0, 2, NULL, NULL);

			nSectId = SECTID_WAIT_THREAD_A_FUNC01;
			enAct = EN_THM_ACT_WAIT;
			break;

		case SECTID_WAIT_THREAD_A_FUNC01: {
			EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;
			THM_LOG_I ("reqFunc01ThreadA return [%d] msg:[%s]\n", enRslt, pIf->getSrcInfo()->pszMsg);

			if (enRslt == EN_THM_RSLT_SUCCESS) {
				nSectId = SECTID_END;
				enAct = EN_THM_ACT_CONTINUE;
			} else {
				nSectId = SECTID_ERR_END;
				enAct = EN_THM_ACT_CONTINUE;
			}

			} break;

		case SECTID_END:
			pIf->reply (EN_THM_RSLT_SUCCESS, NULL);
			nSectId = THM_SECT_ID_INIT;
			enAct = EN_THM_ACT_DONE;
			break;

		case SECTID_ERR_END:
			pIf->reply (EN_THM_RSLT_ERROR, NULL);
			nSectId = THM_SECT_ID_INIT;
			enAct = EN_THM_ACT_DONE;
			break;

		default:
			break;
		}

		pIf->setSectId (nSectId, enAct);
	}

	void func01 (CThreadMgrIf *pIf)
	{
		uint8_t nSectId;
		EN_THM_ACT enAct;
		enum {
			SECTID_ENTRY = THM_SECT_ID_INIT,
			SECTID_REQ_THREAD_B_FUNC00,
			SECTID_WAIT_THREAD_B_FUNC00,
			SECTID_END,
			SECTID_ERR_END,
		};

		nSectId = pIf->getSectId();
		THM_LOG_I ("nSectId %d\n", nSectId);

		switch (nSectId) {
		case SECTID_ENTRY:
			THM_LOG_I ("msg [%s]\n", pIf->getSrcInfo()->pszMsg);

//			nSectId = SECTID_REQ_THREAD_B_FUNC00;
			nSectId = SECTID_END;
			enAct = EN_THM_ACT_CONTINUE;
			break;
/**
		case SECTID_REQ_THREAD_B_FUNC00:

			// スレッドBのfunc00にリクエスト
			reqFunc00ThreadB (NULL);
			reqFunc00ThreadB (NULL);
			reqFunc00ThreadB (NULL);
			reqFunc00ThreadB (NULL);
			reqFunc00ThreadB (&mTmpReqId);
			THM_LOG_I ("reqFunc00ThreadB reqid:[%d]\n", mTmpReqId);

			nSectId = SECTID_WAIT_THREAD_B_FUNC00;
			enAct = EN_THM_ACT_WAIT;
			break;

		case SECTID_WAIT_THREAD_B_FUNC00: {
			if (pIf->getSrcInfo()->nReqId != mTmpReqId) {
				THM_LOG_W ("different reqid %d\n", pIf->getSrcInfo()->nReqId);
				nSectId = SECTID_WAIT_THREAD_B_FUNC00;
				enAct = EN_THM_ACT_WAIT;

			} else {
				EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;
				THM_LOG_I ("reqFunc00ThreadB return [%d] msg:[%s]\n", enRslt, pIf->getSrcInfo()->pszMsg);

				if (enRslt == EN_THM_RSLT_SUCCESS) {
					nSectId = SECTID_END;
					enAct = EN_THM_ACT_CONTINUE;
				} else {
					nSectId = SECTID_ERR_END;
					enAct = EN_THM_ACT_CONTINUE;
				}
			}

			} break;
**/
		case SECTID_END:
			pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)"aaaaaaaaaaaaaaaaa");
			nSectId = THM_SECT_ID_INIT;
			enAct = EN_THM_ACT_DONE;
			break;

		case SECTID_ERR_END:
			pIf->reply (EN_THM_RSLT_ERROR, NULL);
			nSectId = THM_SECT_ID_INIT;
			enAct = EN_THM_ACT_DONE;
			break;

		default:
			break;
		}

		pIf->setSectId (nSectId, enAct);
	}


private:
	PFN_SEQ_BASE mpfnSeqs [3];

	uint32_t mTmpReqId;

};


CModuleA gthread;

CThreadMgrBase *gpthreads [] = {
	&gthread,
};

static void dispatcher (
	EN_THM_DISPATCH_TYPE enType,
	uint8_t nThreadIdx,
	uint8_t nSeqIdx,
	ST_THM_IF *pIf
) {
	CThreadMgrBase *p = gpthreads [nThreadIdx];
	if (!p) {
		printf ("Err: mpThreads [%d] is null.", nThreadIdx);
		return ;
	}

	////////////////////////////////
	p->exec (enType, nSeqIdx, pIf);
	////////////////////////////////
}

int main (void)
{
	gpMgr = new CThreadMgr (1);
	gpMgr->init();

	gpMgr->getExternalIf()->createExternalCp();

	gpMgr->getExternalIf()->requestAsync (0, 0, NULL, NULL);
	ST_THM_SRC_INFO* res = gpMgr->getExternalIf()-> receiveExternal();
	if (res) {
		THM_LOG_I ("dddddddddddddddddd res [%d][%s]", res->enRslt, res->pszMsg);
	} else {
		THM_LOG_E ("dddddddddddddddddd res null");
	}

	gpMgr->getExternalIf()->requestAsync (0, 1, (uint8_t*)"test request", NULL);

	pause ();

	gpMgr->finaliz();


	exit (EXIT_SUCCESS);
}
