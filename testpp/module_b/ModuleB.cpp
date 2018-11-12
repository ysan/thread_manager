#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ModuleB.h"
#include "ModuleB_extern.h"
#include "mgr_reg_tbl.h"


using namespace ThreadManager;



CModuleB::CModuleB (char *pszName, uint8_t nQueNum) : CThreadMgrBase (pszName, nQueNum)
{
	mSeqs [EN_SEQ_MODULE_B_STARTUP] = {(PFN_SEQ_BASE)&CModuleB::startUp, (char*)"startUp"};
	setSeqs (mSeqs, EN_SEQ_MODULE_B_NUM);
}

CModuleB::~CModuleB (void)
{
}


void CModuleB::startUp (CThreadMgrIf *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = 0,
		SECTID_REQ_REG_NOTIFY_THC,
		SECTID_WAIT_REG_NOTIFY_THC,
		SECTID_END,
		SECTID_ERR_END,
	};

	nSectId = pIf->getSectId();
	THM_LOG_I ("nSectId %d\n", nSectId);

	switch (nSectId) {
	case SECTID_ENTRY:
//		nSectId = SECTID_REQ_REG_NOTIFY_THC;
		nSectId = SECTID_END;
		enAct = EN_THM_ACT_CONTINUE;
		break;

//	case SECTID_REQ_REG_NOTIFY_THC:
//		reqRegNotifyThreadC (NULL);
//		nSectId = SECTID_WAIT_REG_NOTIFY_THC;
//		enAct = EN_THM_ACT_WAIT;
//		break;
//
//	case SECTID_WAIT_REG_NOTIFY_THC: {
//		EN_THM_RSLT enRslt = pIf->pstSrcInfo->enRslt;
//		gnClientId = *(pIf->pstSrcInfo->pszMsg);
//		THM_LOG_I ("return reqRegNotifyThreadC [%d] gnClientId:[%d]\n", enRslt, gnClientId);
//
//		if (enRslt == EN_THM_RSLT_SUCCESS) {
//			nSectId = SECTID_END;
//			enAct = EN_THM_ACT_CONTINUE;
//		} else {
//			nSectId = SECTID_ERR_END;
//			enAct = EN_THM_ACT_CONTINUE;
//		}
//
//		} break;

	case SECTID_END:
		pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)"ModuleB startup end.");
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

