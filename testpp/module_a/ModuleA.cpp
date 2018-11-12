#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ModuleA.h"
#include "ModuleA_extern.h"

#include "mgr_reg_tbl.h"



CModuleA::CModuleA (char *pszName, uint8_t nQueNum)
	:CThreadMgrBase (pszName, nQueNum)
{
	mSeqs [EN_SEQ_STARTUP] = {(PFN_SEQ_BASE)&CModuleA::startUp, (char*)"startUp"};
	mSeqs [EN_SEQ_FUNC00] = {(PFN_SEQ_BASE)&CModuleA::func00, (char*)"func00"};
	mSeqs [EN_SEQ_FUNC01] = {(PFN_SEQ_BASE)&CModuleA::func01, (char*)"func01"};
	setSeqs (mSeqs, EN_SEQ_NUM);
}

CModuleA::~CModuleA (void)
{
}


void CModuleA::startUp (CThreadMgrIf *pIf)
{
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

void CModuleA::func00 (CThreadMgrIf *pIf)
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
//		reqFunc01ThreadA(NULL);
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

void CModuleA::func01 (CThreadMgrIf *pIf)
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

//		nSectId = SECTID_REQ_THREAD_B_FUNC00;
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

