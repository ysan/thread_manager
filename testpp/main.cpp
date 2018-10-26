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


class CTest
{
public:
	CTest (void) {}
	virtual ~CTest (void) {}


	static void seq (CThreadMgrIf *pIf)
	{
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


private:
};

typedef void (*PCB_THM_SEQ_PP) (CThreadMgrIf *pIf);
const PCB_THM_SEQ_PP gpSeqsPP [1] = {
	CTest::seq,
};


ST_THM_REG_TBL gstRegTbl [1] = {
	{
		"thread_A",
		NULL,
		5,
		NULL,	// after
		0,		// after
		NULL,
		NULL,
	},
};

static void seq (ST_THM_IF *pIf)
{
	puts ("test");
	CThreadMgrIf thmIf (pIf);
	(void) (gpSeqsPP [pIf->pstSrcInfo->nSeqIdx]) (&thmIf);
}


int main (void)
{
	PCB_THM_SEQ gpSeqs [1];
	gpSeqs [0] = seq;

	gstRegTbl[0].pcbSeqArray = gpSeqs;
	gstRegTbl[0].nSeqNum = 1;

	if (!(gpIf = setupThreadMgr(gstRegTbl, (uint32_t)1))) {
		exit (EXIT_FAILURE);
	}


	gpIf->pfnCreateExternalCp();
	gpIf->pfnRequestAsync (0, 0, NULL, NULL);


	pause ();

	exit (EXIT_SUCCESS);
}
