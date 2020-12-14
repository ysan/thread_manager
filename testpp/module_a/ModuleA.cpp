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
{
	vector<SEQ_BASE_t> seqs;
	seqs.push_back ({(PFN_SEQ_BASE)&CModuleA::startUp, "startUp"});
	seqs.push_back ({(PFN_SEQ_BASE)&CModuleA::func00, "func00"});
	seqs.push_back ({(PFN_SEQ_BASE)&CModuleA::func01, "func01"});
	seqs.push_back ({(PFN_SEQ_BASE)&CModuleA::func02, "func02"});
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
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);

	const char *msg = "ModuleA startup end.\0";
	pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, strlen(msg)+1);

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
	};

	nSectId = pIf->getSectId();
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);

	switch (nSectId) {
	case SECTID_ENTRY:
		nSectId = SECTID_REQ_THREAD_A_FUNC01;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_THREAD_A_FUNC01: {

		// 自スレのfunc01にリクエスト
		requestAsync(EN_MODULE_A, CModuleA_extern::EN_SEQ_FUNC01);

		nSectId = SECTID_WAIT_THREAD_A_FUNC01;
		enAct = EN_THM_ACT_WAIT;
		} break;

	case SECTID_WAIT_THREAD_A_FUNC01: {
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

void CModuleA::func01 (CThreadMgrIf *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_SYNC_REQ_THREAD_B_FUNC00,
		SECTID_SYNC_REQ_THREAD_B_FUNC00_TIMEOUT,
		SECTID_REQ_THREAD_B_FUNC00,
		SECTID_WAIT_THREAD_B_FUNC00,
		SECTID_REQ_THREAD_B_FUNC00_TIMEOUT,
		SECTID_WAIT_THREAD_B_FUNC00_TIMEOUT,
		SECTID_REQ_THREAD_B_FUNC00_MULTI,
		SECTID_WAIT_THREAD_B_FUNC00_MULTI,
		SECTID_REQ_REG_NOTIFY_THC,
		SECTID_WAIT_REG_NOTIFY_THC,
		SECTID_END,
	};

	nSectId = pIf->getSectId();
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);

	switch (nSectId) {
	case SECTID_ENTRY:
		nSectId = SECTID_SYNC_REQ_THREAD_B_FUNC00;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_SYNC_REQ_THREAD_B_FUNC00: {

		// スレッドBのfunc00に同期リクエスト
		CModuleB_extern ex (getExternalIf());
		ex.syncReqFunc00 ();

		EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;
		THM_LOG_I ("syncReqFunc00ThreadB return [%d] msg:[%s]\n", enRslt, pIf->getSrcInfo()->msg.pMsg);

		assert (enRslt == EN_THM_RSLT_SUCCESS);

		std::string s = (char*)pIf->getSrcInfo()->msg.pMsg;
		assert (s == std::string("thread_b func00 end."));


		nSectId = SECTID_SYNC_REQ_THREAD_B_FUNC00_TIMEOUT;
		enAct = EN_THM_ACT_CONTINUE;

		}
		break;

	case SECTID_SYNC_REQ_THREAD_B_FUNC00_TIMEOUT: {

		// set req-timeout
		uint32_t opt = getRequestOption ();
		opt |= REQUEST_OPTION__WITH_TIMEOUT_MSEC;
		opt &= 0x0000ffff; // clear timeout val
		opt |= 1000 << 16; // set timeout 1sec
		setRequestOption (opt);

		// スレッドBのfunc00に同期リクエスト
		CModuleB_extern ex (getExternalIf());
		ex.syncReqFunc00 ();

		// reset req-timeout
		opt &= ~REQUEST_OPTION__WITH_TIMEOUT_MSEC;
		setRequestOption (opt);

		EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;
		THM_LOG_I ("syncReqFunc00ThreadB return [%d] msg:[%s]\n", enRslt, pIf->getSrcInfo()->msg.pMsg);

		// タイムアウトを受け取ること
		assert (enRslt == EN_THM_RSLT_REQ_TIMEOUT);

		nSectId = SECTID_REQ_THREAD_B_FUNC00;
		enAct = EN_THM_ACT_CONTINUE;

		}
		break;

	case SECTID_REQ_THREAD_B_FUNC00: {

		// スレッドBのfunc00にリクエスト
		CModuleB_extern ex (getExternalIf());
		ex.reqFunc00 ();

		nSectId = SECTID_WAIT_THREAD_B_FUNC00;
		enAct = EN_THM_ACT_WAIT;

		}
		break;

	case SECTID_WAIT_THREAD_B_FUNC00: {

		EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;
		THM_LOG_I ("reqFunc00ThreadB return [%d] msg:[%s]\n", enRslt, pIf->getSrcInfo()->msg.pMsg);

		assert (enRslt == EN_THM_RSLT_SUCCESS);

		nSectId = SECTID_REQ_THREAD_B_FUNC00_TIMEOUT;
		enAct = EN_THM_ACT_CONTINUE;

		} break;

	case SECTID_REQ_THREAD_B_FUNC00_TIMEOUT: {

		// set req-timeout
		uint32_t opt = getRequestOption ();
		opt |= REQUEST_OPTION__WITH_TIMEOUT_MSEC;
		opt &= 0x0000ffff; // clear timeout val
		opt |= 1000 << 16; // set timeout 1sec
		setRequestOption (opt);

		// スレッドBのfunc00にリクエスト
		CModuleB_extern ex (getExternalIf());
		ex.reqFunc00 ();

		// reset req-timeout
		opt &= ~REQUEST_OPTION__WITH_TIMEOUT_MSEC;
		setRequestOption (opt);

		nSectId = SECTID_WAIT_THREAD_B_FUNC00_TIMEOUT;
		enAct = EN_THM_ACT_WAIT;

		}
		break;

	case SECTID_WAIT_THREAD_B_FUNC00_TIMEOUT: {

		EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;
		THM_LOG_I ("reqFunc00ThreadB return [%d] msg:[%s]\n", enRslt, pIf->getSrcInfo()->msg.pMsg);

		// タイムアウトを受け取ること
		assert (enRslt == EN_THM_RSLT_REQ_TIMEOUT);

		nSectId = SECTID_REQ_THREAD_B_FUNC00_MULTI;
		enAct = EN_THM_ACT_CONTINUE;

		} break;

	case SECTID_REQ_THREAD_B_FUNC00_MULTI: {

		// set req-timeout
		uint32_t opt = getRequestOption ();
		opt |= REQUEST_OPTION__WITH_TIMEOUT_MSEC;
		opt &= 0x0000ffff; // clear timeout val
		opt |= (60*1000) << 16; // set timeout 60sec
		setRequestOption (opt);

		// スレッドBのfunc00にリクエスト(セクション中に複数リクエスト)
		CModuleB_extern ex (getExternalIf());
		ex.reqFunc00 ();
		ex.reqFunc00 ();
		ex.reqFunc00 ();
		ex.reqFunc00 ();
		ex.reqFunc00 (&mTmpReqId);
		THM_LOG_I ("reqFunc00ThreadB reqid:[%d]\n", mTmpReqId);

		// reset req-timeout
		opt &= ~REQUEST_OPTION__WITH_TIMEOUT_MSEC;
		setRequestOption (opt);

		nSectId = SECTID_WAIT_THREAD_B_FUNC00_MULTI;
		enAct = EN_THM_ACT_WAIT;

		}
		break;

	case SECTID_WAIT_THREAD_B_FUNC00_MULTI: {
		// 最後に送ったリクエストのIDを期待する
		if (pIf->getSrcInfo()->nReqId != mTmpReqId) {
			THM_LOG_W ("different reqid %d\n", pIf->getSrcInfo()->nReqId);
			nSectId = SECTID_WAIT_THREAD_B_FUNC00_MULTI;
			enAct = EN_THM_ACT_WAIT;

		} else {
			EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;
			THM_LOG_I ("reqFunc00ThreadB return [%d] msg:[%s]\n", enRslt, pIf->getSrcInfo()->msg.pMsg);

			assert (enRslt == EN_THM_RSLT_SUCCESS);

			nSectId = SECTID_REQ_REG_NOTIFY_THC;
			enAct = EN_THM_ACT_CONTINUE;
		}

		} break;

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

		assert (enRslt == EN_THM_RSLT_SUCCESS);

		nSectId = SECTID_END;
		enAct = EN_THM_ACT_CONTINUE;

		} break;

	case SECTID_END: {
		const char *msg = "success\0";
		pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, strlen(msg)+1);
		nSectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		} break;

	default:
		break;
	}

	pIf->setSectId (nSectId, enAct);
}

void CModuleA::func02 (CThreadMgrIf *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_REQ_UNREG_NOTIFY_THC,
		SECTID_WAIT_UNREG_NOTIFY_THC,
		SECTID_END,
	};

	nSectId = pIf->getSectId();
	THM_LOG_I ("%s nSectId %d\n", __PRETTY_FUNCTION__, nSectId);

	switch (nSectId) {
	case SECTID_ENTRY:
		THM_LOG_I ("msg [%s]\n", (char*)pIf->getSrcInfo()->msg.pMsg);

		nSectId = SECTID_REQ_UNREG_NOTIFY_THC;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_UNREG_NOTIFY_THC: {
		CModuleC_extern ex(getExternalIf());
		ex.reqUnregNotify (mClientId);
		nSectId = SECTID_WAIT_UNREG_NOTIFY_THC;
		enAct = EN_THM_ACT_WAIT;
		}
		break;

	case SECTID_WAIT_UNREG_NOTIFY_THC: {
		EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;
		THM_LOG_I ("return reqUnregNotifyThreadC [%d]\n", enRslt);

		assert (enRslt == EN_THM_RSLT_SUCCESS);

		nSectId = SECTID_END;
		enAct = EN_THM_ACT_CONTINUE;

		} break;

	case SECTID_END:
		// dump
		kill (getpid(), SIGQUIT);
		// req-destroy
		kill (getpid(), SIGTERM);

		pIf->reply (EN_THM_RSLT_SUCCESS);
		nSectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}

	pIf->setSectId (nSectId, enAct);
}

void CModuleA::onReceiveNotify (CThreadMgrIf *pIf)
{
	THM_LOG_I ("%s\n", __PRETTY_FUNCTION__);

	assert(pIf->getSrcInfo()->nClientId == mClientId);

	if (pIf->getSrcInfo()->nClientId == mClientId) {
		THM_LOG_I ("recv notify  id:[%d] msg:[%s]\n",
						pIf->getSrcInfo()->nClientId, (char*)pIf->getSrcInfo()->msg.pMsg);

		// set without-reply
		uint32_t opt = getRequestOption ();
		opt |= REQUEST_OPTION__WITHOUT_REPLY;
		setRequestOption (opt);

		requestAsync(EN_MODULE_A, CModuleA_extern::EN_SEQ_FUNC02);

		// reset without-reply
		opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
		setRequestOption (opt);
	}
}
