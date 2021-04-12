#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <signal.h>

#include <cassert>

#include "ModuleA.h"

#include "ModuleA_extern.h"
#include "ModuleB_extern.h"
#include "ModuleC_extern.h"

#include "modules.h"
#include "threadmgr_if.h"


CModuleA::CModuleA (std::string name, uint8_t nQueNum)
	:CThreadMgrBase (name, nQueNum)
	,m_tmp_req_id (0)
	,m_client_id (0)
	,m_is_notified (false)
{
	vector<ThreadManager::SEQ_BASE_t> seqs;
	seqs.push_back ({(ThreadManager::PFN_SEQ_BASE)&CModuleA::startUp, "startUp"});
	seqs.push_back ({(ThreadManager::PFN_SEQ_BASE)&CModuleA::testReqRep, "testReqRep"});
	seqs.push_back ({(ThreadManager::PFN_SEQ_BASE)&CModuleA::testReqRepNotify, "testReqRepNotify"});
	seqs.push_back ({(ThreadManager::PFN_SEQ_BASE)&CModuleA::testLock, "testLock"});
	seqs.push_back ({(ThreadManager::PFN_SEQ_BASE)&CModuleA::testLockIntr, "testLockIntr"});
	seqs.push_back ({(ThreadManager::PFN_SEQ_BASE)&CModuleA::testOverwrite, "testOverwrite"});
	seqs.push_back ({(ThreadManager::PFN_SEQ_BASE)&CModuleA::testDestroy, "testDestroy"});
	setSeqs (seqs);

	m_lock_check.clear();
	m_ow_check.clear();
}

CModuleA::~CModuleA (void)
{
}


void CModuleA::startUp (ThreadManager::CThreadMgrIf *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_END,
	};

	nSectId = pIf->getSectId();
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);

	const char *msg = "ModuleA startup end.";
	pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, strlen(msg)+1);

	nSectId = THM_SECT_ID_INIT;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (nSectId, enAct);
}

void CModuleA::testReqRep (ThreadManager::CThreadMgrIf *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_REQ_TEST_REQREPNOTIFY,
		SECTID_WAIT_TEST_REQREPNOTIFY,
		SECTID_END,
	};

	nSectId = pIf->getSectId();
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);

	switch (nSectId) {
	case SECTID_ENTRY:
		nSectId = SECTID_REQ_TEST_REQREPNOTIFY;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_TEST_REQREPNOTIFY: {

		// 自スレのtestReqRepNotifyにリクエスト
		requestAsync(EN_MODULE_A, CModuleA_extern::EN_SEQ_TEST_REQREPNOTIFY);

		nSectId = SECTID_WAIT_TEST_REQREPNOTIFY;
		enAct = EN_THM_ACT_WAIT;
		} break;

	case SECTID_WAIT_TEST_REQREPNOTIFY: {
		EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;
		THM_LOG_I ("reqFunc01ThreadA return [%d] msg:[%s]\n", enRslt, (char*)pIf->getSrcInfo()->msg.pMsg);

		assert (enRslt == EN_THM_RSLT_SUCCESS);

		std::string s = (char*)pIf->getSrcInfo()->msg.pMsg;
		assert (s == std::string("success"));

		nSectId = SECTID_END;
		enAct = EN_THM_ACT_CONTINUE;

		} break;

	case SECTID_END:
		pIf->reply (EN_THM_RSLT_SUCCESS);
		nSectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}

	pIf->setSectId (nSectId, enAct);
}

void CModuleA::testReqRepNotify (ThreadManager::CThreadMgrIf *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_REQ_ECHO02_THB,
		SECTID_WAIT_ECHO02_THB,
		SECTID_REQ_ECHO02_THB_SYNC,
		SECTID_REQ_ECHO02_THB_SYNC_TIMEOUT,
		SECTID_REQ_ECHO02_THB_TIMEOUT,
		SECTID_WAIT_ECHO02_THB_TIMEOUT,
		SECTID_REQ_ECHO02_THB_MULTI,
		SECTID_WAIT_ECHO02_THB_MULTI,
		SECTID_REQ_TEST_REG_NOTIFY_THC,
		SECTID_WAIT_TEST_REG_NOTIFY_THC,
		SECTID_CHECK_NOTIFY,
		SECTID_WAIT_NOTIFY,
		SECTID_REQ_TEST_UNREG_NOTIFY_THC,
		SECTID_WAIT_TEST_UNREG_NOTIFY_THC,
		SECTID_END,
	};

	nSectId = pIf->getSectId();
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);

	switch (nSectId) {
	case SECTID_ENTRY:

		m_is_notified = false;

		nSectId = SECTID_REQ_ECHO02_THB;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_ECHO02_THB: {

		// スレッドBのecho00にリクエスト
		CModuleB_extern ex (getExternalIf());
		ex.reqEcho02 ();

		nSectId = SECTID_WAIT_ECHO02_THB;
		enAct = EN_THM_ACT_WAIT;

		}
		break;

	case SECTID_WAIT_ECHO02_THB: {

		EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;
		THM_LOG_I ("reqEcho02 ThreadB return [%d]\n", enRslt);

		assert (enRslt == EN_THM_RSLT_SUCCESS);

		nSectId = SECTID_REQ_ECHO02_THB_TIMEOUT;
		enAct = EN_THM_ACT_CONTINUE;

		} break;

	case SECTID_REQ_ECHO02_THB_TIMEOUT: {

		// set req-timeout
		uint32_t opt = getRequestOption ();
		opt |= REQUEST_OPTION__WITH_TIMEOUT_MSEC;
		opt &= 0x0000ffff; // clear timeout val
		opt |= 1000 << 16; // set timeout 1sec
		setRequestOption (opt);

		// スレッドBのecho00にリクエスト
		CModuleB_extern ex (getExternalIf());
		ex.reqEcho02 ();

		// reset req-timeout
		opt &= ~REQUEST_OPTION__WITH_TIMEOUT_MSEC;
		setRequestOption (opt);

		nSectId = SECTID_WAIT_ECHO02_THB_TIMEOUT;
		enAct = EN_THM_ACT_WAIT;

		}
		break;

	case SECTID_WAIT_ECHO02_THB_TIMEOUT: {

		EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;
		THM_LOG_I ("reqEcho02 ThreadB return [%d]\n", enRslt);

		// タイムアウトを受け取ること
		assert (enRslt == EN_THM_RSLT_REQ_TIMEOUT);

		nSectId = SECTID_REQ_ECHO02_THB_SYNC;
		enAct = EN_THM_ACT_CONTINUE;

		} break;

	case SECTID_REQ_ECHO02_THB_SYNC: {

		// スレッドBのecho00に同期リクエスト
		CModuleB_extern ex (getExternalIf());
		ex.reqEcho02sync ();

		EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;
		THM_LOG_I ("reqEcho02sync ThreadB return [%d]\n", enRslt);

		assert (enRslt == EN_THM_RSLT_SUCCESS);

		nSectId = SECTID_REQ_ECHO02_THB_SYNC_TIMEOUT;
		enAct = EN_THM_ACT_CONTINUE;

		}
		break;

	case SECTID_REQ_ECHO02_THB_SYNC_TIMEOUT: {

		// set req-timeout
		uint32_t opt = getRequestOption ();
		opt |= REQUEST_OPTION__WITH_TIMEOUT_MSEC;
		opt &= 0x0000ffff; // clear timeout val
		opt |= 1000 << 16; // set timeout 1sec
		setRequestOption (opt);

		// スレッドBのecho00に同期リクエスト
		CModuleB_extern ex (getExternalIf());
		ex.reqEcho02sync ();

		// reset req-timeout
		opt &= ~REQUEST_OPTION__WITH_TIMEOUT_MSEC;
		setRequestOption (opt);

		EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;
		THM_LOG_I ("reqEcho02sync ThreadB return [%d]\n", enRslt);

		// タイムアウトを受け取ること
		assert (enRslt == EN_THM_RSLT_REQ_TIMEOUT);

		nSectId = SECTID_REQ_ECHO02_THB_MULTI;
		enAct = EN_THM_ACT_CONTINUE;

		}
		break;

	case SECTID_REQ_ECHO02_THB_MULTI: {

		// set req-timeout
		uint32_t opt = getRequestOption ();
		opt |= REQUEST_OPTION__WITH_TIMEOUT_MSEC;
		opt &= 0x0000ffff; // clear timeout val
		opt |= (60*1000) << 16; // set timeout 60sec
		setRequestOption (opt);

		// スレッドBのfunc00にリクエスト(セクション中に複数リクエスト)
		CModuleB_extern ex (getExternalIf());
		ex.reqEcho02 ();
		ex.reqEcho02 ();
		ex.reqEcho02 ();
		ex.reqEcho02 ();
		ex.reqEcho02 (&m_tmp_req_id);
		THM_LOG_I ("reqEcho02 ThreadB reqid:[%d]\n", m_tmp_req_id);

		// reset req-timeout
		opt &= ~REQUEST_OPTION__WITH_TIMEOUT_MSEC;
		setRequestOption (opt);

		nSectId = SECTID_WAIT_ECHO02_THB_MULTI;
		enAct = EN_THM_ACT_WAIT;

		}
		break;

	case SECTID_WAIT_ECHO02_THB_MULTI: {
		EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;
		THM_LOG_I ("reqEcho02 ThreadB return [%d]\n", enRslt);

		assert (enRslt == EN_THM_RSLT_SUCCESS);

		// 最後に送ったリクエストのリプライがくるまで待ちます
		if (pIf->getSrcInfo()->nReqId != m_tmp_req_id) {
			THM_LOG_W ("different reqid %d\n", pIf->getSrcInfo()->nReqId);
			nSectId = SECTID_WAIT_ECHO02_THB_MULTI;
			enAct = EN_THM_ACT_WAIT;

		} else {
			nSectId = SECTID_REQ_TEST_REG_NOTIFY_THC;
			enAct = EN_THM_ACT_CONTINUE;
		}

		} break;

	case SECTID_REQ_TEST_REG_NOTIFY_THC: {
		CModuleC_extern ex(getExternalIf());
		ex.reqTestRegNotify ();
		nSectId = SECTID_WAIT_TEST_REG_NOTIFY_THC;
		enAct = EN_THM_ACT_WAIT;
		}
		break;

	case SECTID_WAIT_TEST_REG_NOTIFY_THC: {
		EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;
		m_client_id = *(pIf->getSrcInfo()->msg.pMsg);
		THM_LOG_I ("return reqRegNotify ThreadC [%d] m_client_id:[%d]\n", enRslt, m_client_id);

		assert (enRslt == EN_THM_RSLT_SUCCESS);

		nSectId = SECTID_CHECK_NOTIFY;
		enAct = EN_THM_ACT_CONTINUE;

		} break;

	case SECTID_CHECK_NOTIFY:
		pIf->clearTimeout();
		pIf->setTimeout(500);
		nSectId = SECTID_WAIT_NOTIFY;
		enAct = EN_THM_ACT_WAIT;
		break;

	case SECTID_WAIT_NOTIFY:
		if (m_is_notified) {
			m_is_notified = false;
			nSectId = SECTID_REQ_TEST_UNREG_NOTIFY_THC;
			enAct = EN_THM_ACT_CONTINUE;
		} else {
			nSectId = SECTID_CHECK_NOTIFY;
			enAct = EN_THM_ACT_CONTINUE;
		}
		break;

	case SECTID_REQ_TEST_UNREG_NOTIFY_THC: {
		CModuleC_extern ex(getExternalIf());
		ex.reqTestUnregNotify (m_client_id);
		nSectId = SECTID_WAIT_TEST_UNREG_NOTIFY_THC;
		enAct = EN_THM_ACT_WAIT;
		}
		break;

	case SECTID_WAIT_TEST_UNREG_NOTIFY_THC: {
		EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;
		THM_LOG_I ("return reqUnregNotify ThreadC [%d]\n", enRslt);

		assert (enRslt == EN_THM_RSLT_SUCCESS);

		nSectId = SECTID_END;
		enAct = EN_THM_ACT_CONTINUE;

		} break;

	case SECTID_END: {
		const char *msg = "success";
		pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, strlen(msg)+1);
		nSectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		} break;

	default:
		break;
	}

	pIf->setSectId (nSectId, enAct);
}

void CModuleA::testLock (ThreadManager::CThreadMgrIf *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_REQ_ECHO01_THB,
		SECTID_WAIT_ECHO01_THB,
		SECTID_END,
	};

	nSectId = pIf->getSectId();
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);

	switch (nSectId) {
	case SECTID_ENTRY: {

		bool is_enable_lock = *(bool*)pIf->getSrcInfo()->msg.pMsg;
		if (is_enable_lock) {
			pIf->lock();
		}

		nSectId = SECTID_REQ_ECHO01_THB;
		enAct = EN_THM_ACT_CONTINUE;

		}
		break;

	case SECTID_REQ_ECHO01_THB: {

		// スレッドBのecho01にリクエスト
		CModuleB_extern ex (getExternalIf());
		ex.reqEcho01 ();

		nSectId = SECTID_WAIT_ECHO01_THB;
		enAct = EN_THM_ACT_WAIT;
		}
		break;

	case SECTID_WAIT_ECHO01_THB: {
		EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;
		THM_LOG_I ("reqEcho01 ThreadB return [%d]\n", enRslt);

		assert (enRslt == EN_THM_RSLT_SUCCESS);

		nSectId = SECTID_END;
		enAct = EN_THM_ACT_CONTINUE;

		}
		break;

	case SECTID_END:
		pIf->unlock();

		m_lock_check << "checked.";

		pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)m_lock_check.str().c_str(), m_lock_check.str().length());

		nSectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}

	pIf->setSectId (nSectId, enAct);
}

void CModuleA::testLockIntr (ThreadManager::CThreadMgrIf *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_END,
	};

	nSectId = pIf->getSectId();
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);

	m_lock_check << "intr.";

	pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)m_lock_check.str().c_str(), m_lock_check.str().length());

	nSectId = THM_SECT_ID_INIT;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (nSectId, enAct);
}

void CModuleA::testOverwrite (ThreadManager::CThreadMgrIf *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_REQ_ECHO02_THB,
		SECTID_WAIT_ECHO02_THB,
		SECTID_END,
	};

	nSectId = pIf->getSectId();
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);

	switch (nSectId) {
	case SECTID_ENTRY: {

		pIf->enableOverwrite();

		m_ow_check << "ch";
		
		nSectId = SECTID_REQ_ECHO02_THB;
		enAct = EN_THM_ACT_CONTINUE;

		}
		break;

	case SECTID_REQ_ECHO02_THB: {

		// スレッドBのecho02にリクエスト
		CModuleB_extern ex (getExternalIf());
		ex.reqEcho02 (&m_tmp_req_id);
		THM_LOG_I ("reqEcho02 ThreadB reqid:[%d]\n", m_tmp_req_id);

		nSectId = SECTID_WAIT_ECHO02_THB;
		enAct = EN_THM_ACT_WAIT;
		}
		break;

	case SECTID_WAIT_ECHO02_THB: {
		EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;
		THM_LOG_I ("reqEcho02 ThreadB return [%d]\n", enRslt);

		assert (enRslt == EN_THM_RSLT_SUCCESS);

		// 最後に送ったリクエストのリプライがくるまで待ちます
		if (pIf->getSrcInfo()->nReqId != m_tmp_req_id) {
			THM_LOG_W ("different reqid %d\n", pIf->getSrcInfo()->nReqId);
			nSectId = SECTID_WAIT_ECHO02_THB;
			enAct = EN_THM_ACT_WAIT;

		} else {
			nSectId = SECTID_END;
			enAct = EN_THM_ACT_CONTINUE;
		}

		}
		break;

	case SECTID_END:
		pIf->disableOverwrite();

		m_ow_check << "ecked.";

		pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)m_ow_check.str().c_str(), m_ow_check.str().length());

		nSectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}

	pIf->setSectId (nSectId, enAct);
}

void CModuleA::testDestroy (ThreadManager::CThreadMgrIf *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_END,
	};

	nSectId = pIf->getSectId();
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);


	// dump
	kill (getpid(), SIGQUIT);
	// req-destroy
	kill (getpid(), SIGTERM);


	nSectId = THM_SECT_ID_INIT;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (nSectId, enAct);
}

void CModuleA::onReceiveNotify (ThreadManager::CThreadMgrIf *pIf)
{
	THM_LOG_I ("%s\n", __PRETTY_FUNCTION__);

	assert(pIf->getSrcInfo()->nClientId == m_client_id);

	if (pIf->getSrcInfo()->nClientId == m_client_id) {
		THM_LOG_I ("recv notify  id:[%d] msg:[%s]\n",
						pIf->getSrcInfo()->nClientId, (char*)pIf->getSrcInfo()->msg.pMsg);

//		// set without-reply
//		uint32_t opt = getRequestOption ();
//		opt |= REQUEST_OPTION__WITHOUT_REPLY;
//		setRequestOption (opt);
//
//		requestAsync(EN_MODULE_A, CModuleA_extern::EN_SEQ_FUNC02);
//
//		// reset without-reply
//		opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
//		setRequestOption (opt);

		m_is_notified = true;
	}
}
