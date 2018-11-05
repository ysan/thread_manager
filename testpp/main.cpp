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
#include "ThreadMgr.h"


using namespace ThreadManager;


class CModuleA : public CThreadMgrBase
{
public:
	CModuleA (char *pszName, uint8_t nQueNum) : CThreadMgrBase (pszName, nQueNum)
	{
//		mpfnSeqs [0] = (PFN_SEQ_BASE)&CModuleA::startUp;
//		mpfnSeqs [1] = (PFN_SEQ_BASE)&CModuleA::func00;
//		mpfnSeqs [2] = (PFN_SEQ_BASE)&CModuleA::func01;
//		setSeqs (mpfnSeqs, sizeof(mpfnSeqs) / sizeof(PFN_SEQ_BASE));
		mSeqs [0] = {(PFN_SEQ_BASE)&CModuleA::startUp, (char*)"startUp"};
		mSeqs [1] = {(PFN_SEQ_BASE)&CModuleA::func00, (char*)"func00"};
		mSeqs [2] = {(PFN_SEQ_BASE)&CModuleA::func01, (char*)"func01"};
		setSeqs (mSeqs, sizeof(mSeqs) / sizeof(ST_SEQ_BASE));
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
			CThreadMgr::getInstance()->getExternalIf()->requestAsync (0, 2, NULL, NULL);

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
//	PFN_SEQ_BASE mpfnSeqs [3]; // entity
	ST_SEQ_BASE mSeqs [3]; // entity

	uint32_t mTmpReqId;

};

class CModuleB : public CThreadMgrBase
{
public:
	CModuleB (char *pszName, uint8_t nQueNum) : CThreadMgrBase (pszName, nQueNum)
	{
		mSeqs [0] = {(PFN_SEQ_BASE)&CModuleB::startUp, (char*)"startUp"};
		setSeqs (mSeqs, sizeof(mSeqs) / sizeof(ST_SEQ_BASE));
	}
	virtual ~CModuleB (void) {}


	void startUp (CThreadMgrIf *pIf) {
		uint8_t nSectId;
		EN_THM_ACT enAct;
		enum {
			SECTID_ENTRY = THM_SECT_ID_INIT,
			SECTID_END,
		};

		nSectId = pIf->getSectId();
		THM_LOG_I ("nSectId %d\n", nSectId);

		pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)"ModuleB startup end.");

		nSectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		pIf->setSectId (nSectId, enAct);
	}

private:
	ST_SEQ_BASE mSeqs [1]; // entity

};


CModuleA g_moduleA ((char*)"ModuleA", 10);
CModuleB g_moduleB ((char*)"ModuleB", 10);


CThreadMgrBase *gpthreads [] = {
	&g_moduleA,
	&g_moduleB,
};

int main (void)
{
	CThreadMgr *pMgr = CThreadMgr::getInstance();

	if (!pMgr->setup (gpthreads, 2)) {
		exit (EXIT_FAILURE);
	}

	pMgr->getExternalIf()->createExternalCp();

	pMgr->getExternalIf()->requestAsync (0, 0);
	ST_THM_SRC_INFO* r = pMgr->getExternalIf()-> receiveExternal();
	if (r) {
		THM_LOG_I ("dddddddddddddddddd r [%d][%s]", r->enRslt, r->pszMsg);
	} else {
		THM_LOG_E ("dddddddddddddddddd r null");
	}

	pMgr->getExternalIf()->requestAsync (1, 0);
	r = pMgr->getExternalIf()-> receiveExternal();
	if (r) {
		THM_LOG_I ("dddddddddddddddddd r [%d][%s]", r->enRslt, r->pszMsg);
	} else {
		THM_LOG_E ("dddddddddddddddddd r null");
	}


	pMgr->getExternalIf()->requestAsync (0, 1, (uint8_t*)"test request");


	pause ();




	pMgr->teardown();


	exit (EXIT_SUCCESS);
}
