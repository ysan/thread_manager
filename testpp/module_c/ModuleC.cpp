#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ModuleC.h"
#include "ModuleC_extern.h"
#include "modules.h"


CModuleC::CModuleC (std::string name, uint8_t nQueNum)
	: CThreadMgrBase (name, nQueNum)
{
	ThreadManager::SEQ_BASE_t seqs[CModuleC_extern::EN_SEQ_NUM] = {
		{(ThreadManager::PFN_SEQ_BASE)&CModuleC::startUp, "startUp"},
		{(ThreadManager::PFN_SEQ_BASE)&CModuleC::testRegNotify, "testRegNotify"},
		{(ThreadManager::PFN_SEQ_BASE)&CModuleC::testUnregNotify, "testUnregNotify"},
		{(ThreadManager::PFN_SEQ_BASE)&CModuleC::testNotify, "testNotify"},
		{(ThreadManager::PFN_SEQ_BASE)&CModuleC::echo00, "echo00"},
		{(ThreadManager::PFN_SEQ_BASE)&CModuleC::echo01, "echo01"},
		{(ThreadManager::PFN_SEQ_BASE)&CModuleC::echo02, "echo02"},
	};
	setSeqs (seqs, CModuleC_extern::EN_SEQ_NUM);
}

CModuleC::~CModuleC (void)
{
}


void CModuleC::startUp (ThreadManager::CThreadMgrIf *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_REQ_TEST_NOTIFY,
		SECTID_WAIT_TEST_NOTIFY,
		SECTID_END,
		SECTID_ERR_END,
	};

	nSectId = pIf->getSectId();
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);

	switch (nSectId) {
	case SECTID_ENTRY:
		nSectId = SECTID_REQ_TEST_NOTIFY;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_TEST_NOTIFY:
		requestAsync (EN_MODULE_C, CModuleC_extern::EN_SEQ_TEST_NOTIFY);
		nSectId = SECTID_WAIT_TEST_NOTIFY;
		enAct = EN_THM_ACT_WAIT;
		break;

	case SECTID_WAIT_TEST_NOTIFY: {
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
		const char *msg = "ModuleC startup end.";
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

void CModuleC::testRegNotify (ThreadManager::CThreadMgrIf *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_END,
	};

	nSectId = pIf->getSectId();
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);

	uint8_t client_id;
	bool rslt = pIf->regNotify (CModuleC_extern::EN_NOTIFY_CAT_1, &client_id);

	EN_THM_RSLT enRslt;
	if (rslt) {
		enRslt = EN_THM_RSLT_SUCCESS;
	} else {
		enRslt = EN_THM_RSLT_ERROR;
	}

	// client_idをmsgで返す
	pIf->reply (enRslt, (uint8_t*)&client_id, sizeof(client_id));

	nSectId = THM_SECT_ID_INIT;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (nSectId, enAct);
}

void CModuleC::testUnregNotify (ThreadManager::CThreadMgrIf *pIf)
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
	bool rslt = pIf->unregNotify (CModuleC_extern::EN_NOTIFY_CAT_1, id);
	if (rslt) {
		enRslt = EN_THM_RSLT_SUCCESS;
	} else {
		enRslt = EN_THM_RSLT_ERROR;
	}

	pIf->reply (enRslt);

	nSectId = THM_SECT_ID_INIT;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (nSectId, enAct);
}

void CModuleC::testNotify (ThreadManager::CThreadMgrIf *pIf)
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
		pIf->clearTimeout ();
		pIf->setTimeout (10000);
		nSectId = SECTID_SEND_NOTIFY;
		enAct = EN_THM_ACT_WAIT;
		break;
	case SECTID_SEND_NOTIFY: {
		const char *msg = "this is notify message...";
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

void CModuleC::echo00 (ThreadManager::CThreadMgrIf *pIf)
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

void CModuleC::echo01 (ThreadManager::CThreadMgrIf *pIf)
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

void CModuleC::echo02 (ThreadManager::CThreadMgrIf *pIf)
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
