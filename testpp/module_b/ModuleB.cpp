#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ModuleB.h"
#include "ModuleB_extern.h"
#include "ModuleC_extern.h"
#include "modules.h"


CModuleB::CModuleB (std::string name, uint8_t nQueNum)
	: CThreadMgrBase (name, nQueNum)
{
	vector<ThreadManager::SEQ_BASE_t> seqs;
	seqs.push_back ({(ThreadManager::PFN_SEQ_BASE)&CModuleB::startUp, "startUp"});
	seqs.push_back ({(ThreadManager::PFN_SEQ_BASE)&CModuleB::echo00, "echo00"});
	seqs.push_back ({(ThreadManager::PFN_SEQ_BASE)&CModuleB::echo01, "echo01"});
	seqs.push_back ({(ThreadManager::PFN_SEQ_BASE)&CModuleB::echo02, "echo02"});
	setSeqs (seqs);
}

CModuleB::~CModuleB (void)
{
}


void CModuleB::startUp (ThreadManager::CThreadMgrIf *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_END,
	};

	nSectId = pIf->getSectId();
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);

	const char *msg = "ModuleB startup end.";
	pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, strlen(msg)+1);

	nSectId = THM_SECT_ID_INIT;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (nSectId, enAct);
}

void CModuleB::echo00 (ThreadManager::CThreadMgrIf *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = 0,
		SECTID_END,
	};

	nSectId = pIf->getSectId();
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);

	size_t msglen = pIf->getSrcInfo()->msg.size;
	if (msglen == 0) {
		pIf->reply (EN_THM_RSLT_SUCCESS);
	} else {
		char *msg = (char*)pIf->getSrcInfo()->msg.pMsg;
		pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, msglen);
	}

	nSectId = 0;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (nSectId, enAct);
}

void CModuleB::echo01 (ThreadManager::CThreadMgrIf *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = 0,
		SECTID_END,
	};

	nSectId = pIf->getSectId();
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);

	THM_LOG_I ("execute. sleep 1sec.\n");
	sleep (1);

	size_t msglen = pIf->getSrcInfo()->msg.size;
	if (msglen == 0) {
		pIf->reply (EN_THM_RSLT_SUCCESS);
	} else {
		char *msg = (char*)pIf->getSrcInfo()->msg.pMsg;
		pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, msglen);
	}

	nSectId = 0;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (nSectId, enAct);
}

void CModuleB::echo02 (ThreadManager::CThreadMgrIf *pIf)
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

	size_t msglen = pIf->getSrcInfo()->msg.size;
	if (msglen == 0) {
		pIf->reply (EN_THM_RSLT_SUCCESS);
	} else {
		char *msg = (char*)pIf->getSrcInfo()->msg.pMsg;
		pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, msglen);
	}

	nSectId = 0;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (nSectId, enAct);
}
