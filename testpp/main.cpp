#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "threadmgr_if.h"
#include "threadmgr_util.h"

#include "ThreadMgrIf.h"
#include "ThreadMgrExternalIf.h"


ST_THM_EXTERNAL_IF *gpIf;



class CThreadMgrBase;
typedef void (CThreadMgrBase:: *PFN_SEQ) (CThreadMgrIf *pIf);

class CThreadMgrBase
{
public:
	CThreadMgrBase (void) {
		initSeqs ();
	}
	virtual ~CThreadMgrBase (void) {}

	void initSeqs (void) {
		mpfnSeqs [0] = &CThreadMgrBase::testSeq;
	}

	void exec (EN_THM_DISPATCH_TYPE enType, uint8_t nSeqIdx, CThreadMgrIf *pIf) {

		(void) (this->*mpfnSeqs[nSeqIdx]) (pIf);

	}

	void onCreate (void) {
	}

	void testSeq (CThreadMgrIf *pIf) {
		uint8_t nSectId;
		EN_THM_ACT enAct;
		enum {
			SECTID_ENTRY = THM_SECT_ID_INIT,
			SECTID_END,
		};

		nSectId = pIf->getSectId();
		THM_LOG_I ("nSectId %d\n", nSectId);

		pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)"threadA func");

		nSectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		pIf->setSectId (nSectId, enAct);
	}

	void onReceiveNotify (CThreadMgrIf *pIf) {
	}


private:
	PFN_SEQ mpfnSeqs [1];

};

CThreadMgrBase gbase;

static void dispatcher (EN_THM_DISPATCH_TYPE enType, uint8_t nThreadIdx, uint8_t nSeqIdx, ST_THM_IF *pIf)
{
	puts ("dispatcher");

	CThreadMgrIf thmIf (pIf);
	gbase.exec (enType, nSeqIdx, &thmIf);

}

int main (void)
{
	int n = 1;
	ST_THM_REG_TBL *pTbl = (ST_THM_REG_TBL*) malloc (sizeof(ST_THM_REG_TBL) * n);
	pTbl->pszName = "test";
	const_cast <PCB_SETUP&> (pTbl->pcbSetup) = NULL;
	pTbl->nQueNum = 10;
	pTbl->pcbSeqArray = NULL;
	pTbl->nSeqNum = 1;
	const_cast <PCB_RECV_ASYNC_REPLY&> (pTbl->pcbRecvAsyncReply) = NULL;
	const_cast <PCB_RECV_NOTIFY&> (pTbl->pcbRecvNotify) = NULL;

	setupDispatcher (dispatcher);

	if (!(gpIf = setupThreadMgr(pTbl, (uint32_t)1))) {
		exit (EXIT_FAILURE);
	}


	gpIf->pfnCreateExternalCp();
	gpIf->pfnRequestAsync (0, 0, NULL, NULL);


	pause ();

	exit (EXIT_SUCCESS);
}
