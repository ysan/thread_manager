#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ModuleC.h"
#include "ModuleC_extern.h"
#include "modules.h"


using namespace ThreadManager;



CModuleC::CModuleC (std::string name, uint8_t nQueNum)
	: CThreadMgrBase (name, nQueNum)
{
	vector<SEQ_BASE_t> seqs;
	seqs.push_back ({(PFN_SEQ_BASE)&CModuleC::startUp, "startUp"});
	seqs.push_back ({(PFN_SEQ_BASE)&CModuleC::regNotify, "regNotify"});
	seqs.push_back ({(PFN_SEQ_BASE)&CModuleC::unregNotify, "unregNotify"});
	seqs.push_back ({(PFN_SEQ_BASE)&CModuleC::cycleFunc, "cycleFunc"});
	seqs.push_back ({(PFN_SEQ_BASE)&CModuleC::func00, "func00"});
	setSeqs (seqs);
}

CModuleC::~CModuleC (void)
{
}


void CModuleC::startUp (CThreadMgrIf *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_REQ_CYCLE_FUNC,
		SECTID_WAIT_CYCLE_FUNC,
		SECTID_END,
		SECTID_ERR_END,
	};

	nSectId = pIf->getSectId();
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);

	switch (nSectId) {
	case SECTID_ENTRY:
		nSectId = SECTID_REQ_CYCLE_FUNC;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_CYCLE_FUNC:
		requestAsync (EN_MODULE_C, CModuleC_extern::EN_SEQ_CYCLE_FUNC);
		nSectId = SECTID_WAIT_CYCLE_FUNC;
		enAct = EN_THM_ACT_WAIT;
		break;

	case SECTID_WAIT_CYCLE_FUNC: {
		EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;
		if (enRslt == EN_THM_RSLT_SUCCESS) {
			nSectId = SECTID_END;
			enAct = EN_THM_ACT_CONTINUE;
		} else {
			nSectId = SECTID_ERR_END;
			enAct = EN_THM_ACT_CONTINUE;
		}

		}
		break;

	case SECTID_END: {
		const char *msg = "ModuleC startup end.\0";
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

void CModuleC::regNotify (CThreadMgrIf *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_END,
	};

	nSectId = pIf->getSectId();
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);

	// 複数クライアントがいるときはちゃんとid管理すること
	bool rslt = pIf->regNotify (CModuleC_extern::EN_NOTIFY_CAT_1, &mClientId);

	EN_THM_RSLT enRslt;
	if (rslt) {
		enRslt = EN_THM_RSLT_SUCCESS;
	} else {
		enRslt = EN_THM_RSLT_ERROR;
	}

	// clientIdをmsgで返す
	pIf->reply (enRslt, (uint8_t*)&mClientId, sizeof(mClientId));

	nSectId = THM_SECT_ID_INIT;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (nSectId, enAct);
}

void CModuleC::unregNotify (CThreadMgrIf *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_END,
	};

	nSectId = pIf->getSectId();
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);

	EN_THM_RSLT enRslt;
	// msgからidを取得
	uint8_t id = *(pIf->getSrcInfo()->msg.pMsg);
	if (id != mClientId) {
		// ここではidの一致だけ確認 本来はちゃんと管理すべき
		THM_LOG_E ("clientId is not match.");
		pIf->reply (EN_THM_RSLT_ERROR);
		enRslt = EN_THM_RSLT_ERROR;
	} else {
		bool rslt = pIf->unregNotify (CModuleC_extern::EN_NOTIFY_CAT_1, id);
		if (rslt) {
			enRslt = EN_THM_RSLT_SUCCESS;
		} else {
			enRslt = EN_THM_RSLT_ERROR;
		}
	}

	pIf->reply (enRslt);

	nSectId = THM_SECT_ID_INIT;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (nSectId, enAct);
}

void CModuleC::cycleFunc (CThreadMgrIf *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_CYCLE,
		SECTID_SEND_NOTIFY,
		SECTID_END,
	};

	nSectId = pIf->getSectId();
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);

	switch (nSectId) {
	case SECTID_ENTRY:
		// 先にreplyしておく
		pIf->reply (EN_THM_RSLT_SUCCESS);

		nSectId = SECTID_CYCLE;
		enAct = EN_THM_ACT_CONTINUE;
		break;
	case SECTID_CYCLE:
		pIf->setTimeout (15000);
		nSectId = SECTID_SEND_NOTIFY;
		enAct = EN_THM_ACT_WAIT;
		break;
	case SECTID_SEND_NOTIFY: {
		const char *msg = "this is notify message...\0";
		pIf->notify (CModuleC_extern::EN_NOTIFY_CAT_1, (uint8_t*)msg, strlen(msg)+1);

		nSectId = SECTID_CYCLE;
		enAct = EN_THM_ACT_CONTINUE;
		} break;
	case SECTID_END:
		break;
	default:
		break;
	}

	pIf->setSectId (nSectId, enAct);
}

void CModuleC::func00 (CThreadMgrIf *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_END,
	};

	nSectId = pIf->getSectId();
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);

	sleep (60);
	THM_LOG_I ("reply");
	pIf->reply (EN_THM_RSLT_SUCCESS);

	nSectId = THM_SECT_ID_INIT;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (nSectId, enAct);
}
