#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ModuleA.h"
#include "ModuleA_extern.h"

#include "modules.h"



CModuleA::CModuleA (std::string name, uint8_t nQueNum)
	:CThreadMgrBase (name, nQueNum)
{
	vector<SEQ_BASE_t> seqs;
	seqs.push_back ({(PFN_SEQ_BASE)&CModuleA::startUp, "startUp"});
	seqs.push_back ({(PFN_SEQ_BASE)&CModuleA::func00, "func00"});
	seqs.push_back ({(PFN_SEQ_BASE)&CModuleA::func01, "func01"});
	setSeqs (seqs);
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

	const char *msg = "ModuleA startup end.";
	pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, strlen(msg));

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
		THM_LOG_I ("msg [%s]\n", (char*)pIf->getSrcInfo()->msg.pMsg);

		nSectId = SECTID_REQ_THREAD_A_FUNC01;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_THREAD_A_FUNC01: {

		// 自スレのfunc01にリクエスト
		CModuleA_extern *ex = new CModuleA_extern (CThreadMgr::getInstance()->getExternalIf());
		const char *msg = "req func01";
		ex->reqFunc01 (msg, strlen(msg));
		delete ex;
		ex = NULL;

		nSectId = SECTID_WAIT_THREAD_A_FUNC01;
		enAct = EN_THM_ACT_WAIT;
		} break;

	case SECTID_WAIT_THREAD_A_FUNC01: {
		EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;
		THM_LOG_I ("reqFunc01ThreadA return [%d] msg:[%s]\n", enRslt, (char*)pIf->getSrcInfo()->msg.pMsg);

		if (enRslt == EN_THM_RSLT_SUCCESS) {
			nSectId = SECTID_END;
			enAct = EN_THM_ACT_CONTINUE;
		} else {
			nSectId = SECTID_ERR_END;
			enAct = EN_THM_ACT_CONTINUE;
		}

		} break;

	case SECTID_END:
		pIf->reply (EN_THM_RSLT_SUCCESS);
		nSectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	case SECTID_ERR_END:
		pIf->reply (EN_THM_RSLT_ERROR);
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
		THM_LOG_I ("msg [%s]\n", (char*)pIf->getSrcInfo()->msg.pMsg);

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
			THM_LOG_I ("reqFunc00ThreadB return [%d] msg:[%s]\n", enRslt, pIf->getSrcInfo()->pMsg);

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
	case SECTID_END: {
		const char *msg = "aaaaaaaaaaaaaaaaa";
		pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, strlen(msg));
		nSectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		} break;

	case SECTID_ERR_END:
		pIf->reply (EN_THM_RSLT_ERROR);
		nSectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}

	pIf->setSectId (nSectId, enAct);
}

