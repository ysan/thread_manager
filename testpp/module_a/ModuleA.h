#ifndef _MODULEA_H_
#define _MODULEA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include <signal.h>

#include <string>
#include <sstream>

#include "ThreadMgrpp.h"

#include "ModuleA_extern.h"
#include "ModuleB_extern.h"
#include "ModuleC_extern.h"
#include "modules.h"


class CModuleA : public threadmgr::CThreadMgrBase
{
public:
	CModuleA (std::string name, uint8_t que_max)
		:CThreadMgrBase (name, que_max)
		,m_tmp_req_id (0)
		,m_client_id (0)
		,m_is_notified (false)
	{
		std::vector<threadmgr::SEQ_BASE_t> seqs;
		seqs.push_back ({[&](threadmgr::CThreadMgrIf *p_if){startup(p_if);}, "startUp"});
		seqs.push_back ({[&](threadmgr::CThreadMgrIf *p_if){test_reqrep(p_if);}, "testReqRep"});
		seqs.push_back ({[&](threadmgr::CThreadMgrIf *p_if){test_reqrep_notify(p_if);}, "testReqRepNotify"});
		seqs.push_back ({[&](threadmgr::CThreadMgrIf *p_if){test_lock(p_if);}, "testLock"});
		seqs.push_back ({[&](threadmgr::CThreadMgrIf *p_if){test_lock_intr(p_if);}, "testLockIntr"});
		seqs.push_back ({[&](threadmgr::CThreadMgrIf *p_if){test_overwrite(p_if);}, "test_overwrite"});
		seqs.push_back ({[&](threadmgr::CThreadMgrIf *p_if){test_destroy(p_if);}, "testDestroy"});
		set_seqs (seqs);
	
		m_lock_check.clear();
		m_ow_check.clear();
	}

	virtual ~CModuleA (void) {
	}


private:
	void startup (threadmgr::CThreadMgrIf *p_if) {
		uint8_t sect_id;
		EN_THM_ACT act;
		enum {
			SECTID_ENTRY = THM_SECT_ID_INIT,
			SECTID_END,
		};

		sect_id = p_if->get_sect_id();
		THM_LOG_I ("%s sect_id %d\n", __PRETTY_FUNCTION__, sect_id);

		const char *msg = "ModuleA startup end.";
		p_if->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, strlen(msg)+1);

		sect_id = THM_SECT_ID_INIT;
		act = EN_THM_ACT_DONE;
		p_if->set_sect_id (sect_id, act);
	}

	void test_reqrep (threadmgr::CThreadMgrIf *p_if) {
		uint8_t sect_id;
		EN_THM_ACT act;
		enum {
			SECTID_ENTRY = THM_SECT_ID_INIT,
			SECTID_REQ_TEST_REQREPNOTIFY,
			SECTID_WAIT_TEST_REQREPNOTIFY,
			SECTID_END,
		};

		sect_id = p_if->get_sect_id();
		THM_LOG_I ("%s sect_id %d\n", __PRETTY_FUNCTION__, sect_id);

		switch (sect_id) {
		case SECTID_ENTRY:
			sect_id = SECTID_REQ_TEST_REQREPNOTIFY;
			act = EN_THM_ACT_CONTINUE;
			break;

		case SECTID_REQ_TEST_REQREPNOTIFY: {

			// 自スレのtestReqRepNotifyにリクエスト
			request_async((int)module::module_a, (int)CModuleA_extern::seq::test_reqrep_notify);

			sect_id = SECTID_WAIT_TEST_REQREPNOTIFY;
			act = EN_THM_ACT_WAIT;
			} break;

		case SECTID_WAIT_TEST_REQREPNOTIFY: {
			EN_THM_RSLT rslt = p_if->get_source()->enRslt;
			THM_LOG_I ("reqFunc01ThreadA return [%d] msg:[%s]\n", rslt, (char*)p_if->get_source()->msg.pMsg);

			assert (rslt == EN_THM_RSLT_SUCCESS);

			std::string s = (char*)p_if->get_source()->msg.pMsg;
			assert (s == std::string("success"));

			sect_id = SECTID_END;
			act = EN_THM_ACT_CONTINUE;

			} break;

		case SECTID_END:
			p_if->reply (EN_THM_RSLT_SUCCESS);
			sect_id = THM_SECT_ID_INIT;
			act = EN_THM_ACT_DONE;
			break;

		default:
			break;
		}

		p_if->set_sect_id (sect_id, act);

	}

	void test_reqrep_notify (threadmgr::CThreadMgrIf *p_if) {
		uint8_t sect_id;
		EN_THM_ACT act;
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

		sect_id = p_if->get_sect_id();
		THM_LOG_I ("%s sect_id %d\n", __PRETTY_FUNCTION__, sect_id);

		switch (sect_id) {
		case SECTID_ENTRY:

			m_is_notified = false;

			sect_id = SECTID_REQ_ECHO02_THB;
			act = EN_THM_ACT_CONTINUE;
			break;

		case SECTID_REQ_ECHO02_THB: {

			// スレッドBのecho02にリクエスト
			CModuleB_extern ex (get_external_if());
			ex.req_echo02 ();

			sect_id = SECTID_WAIT_ECHO02_THB;
			act = EN_THM_ACT_WAIT;

			}
			break;

		case SECTID_WAIT_ECHO02_THB: {
	
			EN_THM_RSLT rslt = p_if->get_source()->enRslt;
			THM_LOG_I ("reqEcho02 ThreadB return [%d]\n", rslt);

			assert (rslt == EN_THM_RSLT_SUCCESS);

			sect_id = SECTID_REQ_ECHO02_THB_TIMEOUT;
			act = EN_THM_ACT_CONTINUE;

			} break;

		case SECTID_REQ_ECHO02_THB_TIMEOUT: {

			// set req-timeout
			uint32_t opt = get_request_option ();
			opt |= REQUEST_OPTION__WITH_TIMEOUT_MSEC;
			opt &= 0x0000ffff; // clear timeout val
			opt |= 1000 << 16; // set timeout 1sec
			set_request_option (opt);

			// スレッドBのecho02にリクエスト
			CModuleB_extern ex (get_external_if());
			ex.req_echo02 ();

			// reset req-timeout
			opt &= ~REQUEST_OPTION__WITH_TIMEOUT_MSEC;
			set_request_option (opt);

			sect_id = SECTID_WAIT_ECHO02_THB_TIMEOUT;
			act = EN_THM_ACT_WAIT;

			}
			break;

		case SECTID_WAIT_ECHO02_THB_TIMEOUT: {

			EN_THM_RSLT rslt = p_if->get_source()->enRslt;
			THM_LOG_I ("reqEcho02 ThreadB return [%d]\n", rslt);

			// タイムアウトを受け取ること
			assert (rslt == EN_THM_RSLT_REQ_TIMEOUT);
	
			sect_id = SECTID_REQ_ECHO02_THB_SYNC;
			act = EN_THM_ACT_CONTINUE;

			} break;

		case SECTID_REQ_ECHO02_THB_SYNC: {

			// スレッドBのecho02に同期リクエスト
			CModuleB_extern ex (get_external_if());
			ex.req_echo02_sync ();

			EN_THM_RSLT rslt = p_if->get_source()->enRslt;
			THM_LOG_I ("reqEcho02sync ThreadB return [%d]\n", rslt);

			assert (rslt == EN_THM_RSLT_SUCCESS);

			sect_id = SECTID_REQ_ECHO02_THB_SYNC_TIMEOUT;
			act = EN_THM_ACT_CONTINUE;

			}
			break;

		case SECTID_REQ_ECHO02_THB_SYNC_TIMEOUT: {

			// set req-timeout
			uint32_t opt = get_request_option ();
			opt |= REQUEST_OPTION__WITH_TIMEOUT_MSEC;
			opt &= 0x0000ffff; // clear timeout val
			opt |= 1000 << 16; // set timeout 1sec
			set_request_option (opt);

			// スレッドBのecho02に同期リクエスト
			CModuleB_extern ex (get_external_if());
			ex.req_echo02_sync ();

			// reset req-timeout
			opt &= ~REQUEST_OPTION__WITH_TIMEOUT_MSEC;
			set_request_option (opt);

			EN_THM_RSLT rslt = p_if->get_source()->enRslt;
			THM_LOG_I ("reqEcho02sync ThreadB return [%d]\n", rslt);

			// タイムアウトを受け取ること
			assert (rslt == EN_THM_RSLT_REQ_TIMEOUT);

			sect_id = SECTID_REQ_ECHO02_THB_MULTI;
			act = EN_THM_ACT_CONTINUE;

			}
			break;

		case SECTID_REQ_ECHO02_THB_MULTI: {

			// set req-timeout
			uint32_t opt = get_request_option ();
			opt |= REQUEST_OPTION__WITH_TIMEOUT_MSEC;
			opt &= 0x0000ffff; // clear timeout val
			opt |= (60*1000) << 16; // set timeout 60sec
			set_request_option (opt);

			// スレッドBのfunc02にリクエスト(セクション中に複数リクエスト)
			CModuleB_extern ex (get_external_if());
			ex.req_echo02 ();
			ex.req_echo02 ();
			ex.req_echo02 ();
			ex.req_echo02 ();
			ex.req_echo02 (&m_tmp_req_id);
			THM_LOG_I ("reqEcho02 ThreadB reqid:[%d]\n", m_tmp_req_id);

			// reset req-timeout
			opt &= ~REQUEST_OPTION__WITH_TIMEOUT_MSEC;
			set_request_option (opt);

			sect_id = SECTID_WAIT_ECHO02_THB_MULTI;
			act = EN_THM_ACT_WAIT;

			}
			break;

		case SECTID_WAIT_ECHO02_THB_MULTI: {
			EN_THM_RSLT rslt = p_if->get_source()->enRslt;
			THM_LOG_I ("reqEcho02 ThreadB return [%d]\n", rslt);

			assert (rslt == EN_THM_RSLT_SUCCESS);

			// 最後に送ったリクエストのリプライがくるまで待ちます
			if (p_if->get_source()->nReqId != m_tmp_req_id) {
				THM_LOG_W ("different reqid %d\n", p_if->get_source()->nReqId);
				sect_id = SECTID_WAIT_ECHO02_THB_MULTI;
				act = EN_THM_ACT_WAIT;

			} else {
				sect_id = SECTID_REQ_TEST_REG_NOTIFY_THC;
				act = EN_THM_ACT_CONTINUE;
			}

			} break;

		case SECTID_REQ_TEST_REG_NOTIFY_THC: {
			CModuleC_extern ex(get_external_if());
			ex.req_test_reg_notify ();
			sect_id = SECTID_WAIT_TEST_REG_NOTIFY_THC;
			act = EN_THM_ACT_WAIT;
			}
			break;

		case SECTID_WAIT_TEST_REG_NOTIFY_THC: {
			EN_THM_RSLT rslt = p_if->get_source()->enRslt;
			m_client_id = *(p_if->get_source()->msg.pMsg);
			THM_LOG_I ("return reqRegNotify ThreadC [%d] m_client_id:[%d]\n", rslt, m_client_id);

			assert (rslt == EN_THM_RSLT_SUCCESS);

			sect_id = SECTID_CHECK_NOTIFY;
			act = EN_THM_ACT_CONTINUE;

			} break;

		case SECTID_CHECK_NOTIFY:
			p_if->clear_timeout();
			p_if->set_timeout(500);
			sect_id = SECTID_WAIT_NOTIFY;
			act = EN_THM_ACT_WAIT;
			break;

		case SECTID_WAIT_NOTIFY:
			if (m_is_notified) {
				m_is_notified = false;
				sect_id = SECTID_REQ_TEST_UNREG_NOTIFY_THC;
				act = EN_THM_ACT_CONTINUE;
			} else {
				sect_id = SECTID_CHECK_NOTIFY;
				act = EN_THM_ACT_CONTINUE;
			}
			break;

		case SECTID_REQ_TEST_UNREG_NOTIFY_THC: {
			CModuleC_extern ex(get_external_if());
			ex.req_test_unreg_notify (m_client_id);
			sect_id = SECTID_WAIT_TEST_UNREG_NOTIFY_THC;
			act = EN_THM_ACT_WAIT;
			}
			break;

		case SECTID_WAIT_TEST_UNREG_NOTIFY_THC: {
			EN_THM_RSLT rslt = p_if->get_source()->enRslt;
			THM_LOG_I ("return reqUnregNotify ThreadC [%d]\n", rslt);

			assert (rslt == EN_THM_RSLT_SUCCESS);

			sect_id = SECTID_END;
			act = EN_THM_ACT_CONTINUE;

			} break;

		case SECTID_END: {
			const char *msg = "success";
			p_if->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, strlen(msg)+1);
			sect_id = THM_SECT_ID_INIT;
			act = EN_THM_ACT_DONE;
			} break;

		default:
			break;
		}

		p_if->set_sect_id (sect_id, act);
	}

	void test_lock (threadmgr::CThreadMgrIf *p_if) {
		uint8_t sect_id;
		EN_THM_ACT act;
		enum {
			SECTID_ENTRY = THM_SECT_ID_INIT,
			SECTID_REQ_ECHO01_THB,
			SECTID_WAIT_ECHO01_THB,
			SECTID_END,
		};

		sect_id = p_if->get_sect_id();
		THM_LOG_I ("%s sect_id %d\n", __PRETTY_FUNCTION__, sect_id);

		switch (sect_id) {
		case SECTID_ENTRY: {

			bool is_enable_lock = *(bool*)p_if->get_source()->msg.pMsg;
			if (is_enable_lock) {
				p_if->lock();
			}

			sect_id = SECTID_REQ_ECHO01_THB;
			act = EN_THM_ACT_CONTINUE;

			}
			break;

		case SECTID_REQ_ECHO01_THB: {

			// スレッドBのecho01にリクエスト
			CModuleB_extern ex (get_external_if());
			ex.req_echo01 ();

			sect_id = SECTID_WAIT_ECHO01_THB;
			act = EN_THM_ACT_WAIT;
			}
			break;

		case SECTID_WAIT_ECHO01_THB: {
			EN_THM_RSLT rslt = p_if->get_source()->enRslt;
			THM_LOG_I ("reqEcho01 ThreadB return [%d]\n", rslt);

			assert (rslt == EN_THM_RSLT_SUCCESS);

			sect_id = SECTID_END;
			act = EN_THM_ACT_CONTINUE;

			}
			break;

		case SECTID_END:
			p_if->unlock();

			m_lock_check << "checked.";

			p_if->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)m_lock_check.str().c_str(), m_lock_check.str().length());

			sect_id = THM_SECT_ID_INIT;
			act = EN_THM_ACT_DONE;
			break;

		default:
			break;
		}

		p_if->set_sect_id (sect_id, act);
	}

	void test_lock_intr (threadmgr::CThreadMgrIf *p_if) {
		uint8_t sect_id;
		EN_THM_ACT act;
		enum {
			SECTID_ENTRY = THM_SECT_ID_INIT,
			SECTID_END,
		};

		sect_id = p_if->get_sect_id();
		THM_LOG_I ("%s sect_id %d\n", __PRETTY_FUNCTION__, sect_id);

		m_lock_check << "intr.";

		p_if->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)m_lock_check.str().c_str(), m_lock_check.str().length());

		sect_id = THM_SECT_ID_INIT;
		act = EN_THM_ACT_DONE;
		p_if->set_sect_id (sect_id, act);
	}

	void test_overwrite (threadmgr::CThreadMgrIf *p_if) {
		uint8_t sect_id;
		EN_THM_ACT act;
		enum {
			SECTID_ENTRY = THM_SECT_ID_INIT,
			SECTID_REQ_ECHO02_THB,
			SECTID_WAIT_ECHO02_THB,
			SECTID_END,
		};

		sect_id = p_if->get_sect_id();
		THM_LOG_I ("%s sect_id %d\n", __PRETTY_FUNCTION__, sect_id);

		switch (sect_id) {
		case SECTID_ENTRY: {

			p_if->enable_overwrite();

			m_ow_check << "ch";
		
			sect_id = SECTID_REQ_ECHO02_THB;
			act = EN_THM_ACT_CONTINUE;

			}
			break;

		case SECTID_REQ_ECHO02_THB: {

			// スレッドBのecho02にリクエスト
			CModuleB_extern ex (get_external_if());
			ex.req_echo02 (&m_tmp_req_id);
			THM_LOG_I ("reqEcho02 ThreadB reqid:[%d]\n", m_tmp_req_id);

			sect_id = SECTID_WAIT_ECHO02_THB;
			act = EN_THM_ACT_WAIT;
			}
			break;

		case SECTID_WAIT_ECHO02_THB: {
			EN_THM_RSLT rslt = p_if->get_source()->enRslt;
			THM_LOG_I ("reqEcho02 ThreadB return [%d]\n", rslt);

			assert (rslt == EN_THM_RSLT_SUCCESS);

			// 最後に送ったリクエストのリプライがくるまで待ちます
			if (p_if->get_source()->nReqId != m_tmp_req_id) {
				THM_LOG_W ("different reqid %d\n", p_if->get_source()->nReqId);
				sect_id = SECTID_WAIT_ECHO02_THB;
				act = EN_THM_ACT_WAIT;

			} else {
				sect_id = SECTID_END;
				act = EN_THM_ACT_CONTINUE;
			}

			}
			break;

		case SECTID_END:
			p_if->disable_overwrite();

			m_ow_check << "ecked.";

			p_if->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)m_ow_check.str().c_str(), m_ow_check.str().length());

			sect_id = THM_SECT_ID_INIT;
			act = EN_THM_ACT_DONE;
			break;

		default:
			break;
		}

		p_if->set_sect_id (sect_id, act);
	}

	void test_destroy (threadmgr::CThreadMgrIf *p_if) {
		uint8_t sect_id;
		EN_THM_ACT act;
		enum {
			SECTID_ENTRY = THM_SECT_ID_INIT,
			SECTID_END,
		};

		sect_id = p_if->get_sect_id();
		THM_LOG_I ("%s sect_id %d\n", __PRETTY_FUNCTION__, sect_id);


		// dump
//		kill (getpid(), SIGQUIT);
		// req-destroy
		kill (getpid(), SIGTERM);


		sect_id = THM_SECT_ID_INIT;
		act = EN_THM_ACT_DONE;
		p_if->set_sect_id (sect_id, act);
	}

	void on_receive_notify (threadmgr::CThreadMgrIf *p_if) override {
		THM_LOG_I ("%s\n", __PRETTY_FUNCTION__);

		assert(p_if->get_source()->nClientId == m_client_id);

		if (p_if->get_source()->nClientId == m_client_id) {
			THM_LOG_I ("recv notify  id:[%d] msg:[%s]\n",
						p_if->get_source()->nClientId, (char*)p_if->get_source()->msg.pMsg);

//			// set without-reply
//			uint32_t opt = get_request_option ();
//			opt |= REQUEST_OPTION__WITHOUT_REPLY;
//			set_request_option (opt);
//
//			requestAsync(EN_MODULE_A, CModuleA_extern::EN_SEQ_FUNC02);
//
//			// reset without-reply
//			opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
//			set_request_option (opt);

			m_is_notified = true;
		}
	}


	uint32_t m_tmp_req_id;
	uint8_t m_client_id;
	bool m_is_notified;
	std::stringstream m_lock_check;
	std::stringstream m_ow_check;
};

#endif
