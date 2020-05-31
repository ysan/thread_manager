#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <signal.h>

#include "ModuleB.h"
#include "ModuleB_extern.h"
#include "ModuleC_extern.h"
#include "modules.h"


using namespace ThreadManager;



CModuleB::CModuleB (std::string name, uint8_t nQueNum)
	: CThreadMgrBase (name, nQueNum)
{
	vector<SEQ_BASE_t> seqs;
	seqs.push_back ({(PFN_SEQ_BASE)&CModuleB::startUp, "startUp"});
	seqs.push_back ({(PFN_SEQ_BASE)&CModuleB::func00, "func00"});
	setSeqs (seqs);
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
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);

	switch (nSectId) {
	case SECTID_ENTRY:
		nSectId = SECTID_REQ_REG_NOTIFY_THC;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_REG_NOTIFY_THC: {
		CModuleC_extern ex(getExternalIf());
		ex.reqRegNotify ();
		nSectId = SECTID_WAIT_REG_NOTIFY_THC;
		enAct = EN_THM_ACT_WAIT;
		}
		break;

	case SECTID_WAIT_REG_NOTIFY_THC: {
		EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;
		mClientId = *(pIf->getSrcInfo()->msg.pMsg);
		THM_LOG_I ("return reqRegNotifyThreadC [%d] mClientId:[%d]\n", enRslt, mClientId);

		if (enRslt == EN_THM_RSLT_SUCCESS) {
			nSectId = SECTID_END;
			enAct = EN_THM_ACT_CONTINUE;
		} else {
			nSectId = SECTID_ERR_END;
			enAct = EN_THM_ACT_CONTINUE;
		}

		} break;

	case SECTID_END: {
		const char *msg = "ModuleB startup end.\0";
		pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, strlen(msg)+1);
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

void CModuleB::func00 (CThreadMgrIf *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = 0,
		SECTID_END,
	};

	nSectId = pIf->getSectId();
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);

	THM_LOG_I ("execute. sleep 2sec.\n");
	sleep (2);

	char *msg = "thread_b func00 end.\0";
	pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, strlen(msg)+1);

	nSectId = 0;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (nSectId, enAct);
}

void CModuleB::onReceiveNotify (CThreadMgrIf *pIf)
{
	THM_LOG_I ("%s\n", __PRETTY_FUNCTION__);

	if (pIf->getSrcInfo()->nClientId == mClientId) {
		THM_LOG_I ("recv notify  id:[%d] msg:[%s]\n", 
						pIf->getSrcInfo()->nClientId, (char*)pIf->getSrcInfo()->msg.pMsg);

		kill (getpid(), SIGQUIT);
		kill (getpid(), SIGTERM);
	}
}
