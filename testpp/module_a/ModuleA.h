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

#include "ThreadMgrBase.h"
#include "ThreadMgrIf.h"
#include "ThreadMgrpp.h"

#include "ModuleA_extern.h"
#include "ModuleB_extern.h"
#include "ModuleC_extern.h"
#include "modules.h"
#include "threadmgr_if.h"


class CModuleA : public threadmgr::CThreadMgrBase
{
public:
	CModuleA (std::string name, uint8_t que_max)
		:CThreadMgrBase (name, que_max)
		,m_tmp_req_id (0)
		,m_client_id (0)
		,m_is_notified (false)
	{
		std::vector<threadmgr::sequence_t> sequences;
		sequences.push_back ({[&](threadmgr::CThreadMgrIf *p_if){startup(p_if);}, "startUp"});
		sequences.push_back ({[&](threadmgr::CThreadMgrIf *p_if){test_reqrep(p_if);}, "testReqRep"});
		sequences.push_back ({[&](threadmgr::CThreadMgrIf *p_if){test_reqrep_notify(p_if);}, "testReqRepNotify"});
		sequences.push_back ({[&](threadmgr::CThreadMgrIf *p_if){test_lock(p_if);}, "testLock"});
		sequences.push_back ({[&](threadmgr::CThreadMgrIf *p_if){test_lock_intr(p_if);}, "testLockIntr"});
		sequences.push_back ({[&](threadmgr::CThreadMgrIf *p_if){test_overwrite(p_if);}, "test_overwrite"});
		sequences.push_back ({[&](threadmgr::CThreadMgrIf *p_if){test_destroy(p_if);}, "testDestroy"});
		set_sequences (sequences);
	
		m_lock_check.clear();
		m_ow_check.clear();
	}

	virtual ~CModuleA (void) {
		reset_sequences();
	}


private:
	void startup (threadmgr::CThreadMgrIf *p_if) {
		threadmgr::section_id::type section_id;
		threadmgr::action act;
		enum {
			SECTID_ENTRY = threadmgr::section_id::init,
			SECTID_END,
		};

		section_id = p_if->get_section_id();
		THM_LOG_I ("%s section_id %d\n", __PRETTY_FUNCTION__, section_id);

		const char *msg = "ModuleA startup end.";
		p_if->reply (threadmgr::result::success, reinterpret_cast<uint8_t*>(const_cast<char*>(msg)), strlen(msg)+1);

		section_id = threadmgr::section_id::init;
		act = threadmgr::action::done;
		p_if->set_section_id (section_id, act);
	}

	void test_reqrep (threadmgr::CThreadMgrIf *p_if) {
		threadmgr::section_id::type section_id;
		threadmgr::action act;
		enum {
			SECTID_ENTRY = threadmgr::section_id::init,
			SECTID_REQ_TEST_REQREPNOTIFY,
			SECTID_WAIT_TEST_REQREPNOTIFY,
			SECTID_END,
		};

		section_id = p_if->get_section_id();
		THM_LOG_I ("%s section_id %d\n", __PRETTY_FUNCTION__, section_id);

		switch (section_id) {
		case SECTID_ENTRY:
			section_id = SECTID_REQ_TEST_REQREPNOTIFY;
			act = threadmgr::action::continue_;
			break;

		case SECTID_REQ_TEST_REQREPNOTIFY: {

			// 自スレのtestReqRepNotifyにリクエスト
			request_async((int)module::module_a, (int)CModuleA_extern::sequence::test_reqrep_notify);

			section_id = SECTID_WAIT_TEST_REQREPNOTIFY;
			act = threadmgr::action::wait;
			} break;

		case SECTID_WAIT_TEST_REQREPNOTIFY: {
			threadmgr::result rslt = p_if->get_source().get_result();
			THM_LOG_I ("reqFunc01ThreadA return [%d] msg:[%s]\n", rslt, reinterpret_cast<char*>(p_if->get_source().get_message().data));

			assert (rslt == threadmgr::result::success);

			std::string s = reinterpret_cast<char*>(p_if->get_source().get_message().data);
			assert (s == std::string("success"));

			section_id = SECTID_END;
			act = threadmgr::action::continue_;

			} break;

		case SECTID_END:
			p_if->reply (threadmgr::result::success);
			section_id = threadmgr::section_id::init;
			act = threadmgr::action::done;
			break;

		default:
			break;
		}

		p_if->set_section_id (section_id, act);

	}

	void test_reqrep_notify (threadmgr::CThreadMgrIf *p_if) {
		threadmgr::section_id::type section_id;
		threadmgr::action act;
		enum {
			SECTID_ENTRY = threadmgr::section_id::init,
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

		section_id = p_if->get_section_id();
		THM_LOG_I ("%s section_id %d\n", __PRETTY_FUNCTION__, section_id);

		switch (section_id) {
		case SECTID_ENTRY:

			m_is_notified = false;

			section_id = SECTID_REQ_ECHO02_THB;
			act = threadmgr::action::continue_;
			break;

		case SECTID_REQ_ECHO02_THB: {

			// スレッドBのecho02にリクエスト
			CModuleB_extern ex (get_external_if());
			ex.req_echo02 ();

			section_id = SECTID_WAIT_ECHO02_THB;
			act = threadmgr::action::wait;

			}
			break;

		case SECTID_WAIT_ECHO02_THB: {
	
			threadmgr::result rslt = p_if->get_source().get_result();
			THM_LOG_I ("reqEcho02 ThreadB return [%d]\n", rslt);

			assert (rslt == threadmgr::result::success);

			section_id = SECTID_REQ_ECHO02_THB_TIMEOUT;
			act = threadmgr::action::continue_;

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

			section_id = SECTID_WAIT_ECHO02_THB_TIMEOUT;
			act = threadmgr::action::wait;

			}
			break;

		case SECTID_WAIT_ECHO02_THB_TIMEOUT: {

			threadmgr::result rslt = p_if->get_source().get_result();
			THM_LOG_I ("reqEcho02 ThreadB return [%d]\n", rslt);

			// タイムアウトを受け取ること
			assert (rslt == threadmgr::result::request_timeout);
	
			section_id = SECTID_REQ_ECHO02_THB_SYNC;
			act = threadmgr::action::continue_;

			} break;

		case SECTID_REQ_ECHO02_THB_SYNC: {

			// スレッドBのecho02に同期リクエスト
			CModuleB_extern ex (get_external_if());
			ex.req_echo02_sync ();

			threadmgr::result rslt = p_if->get_source().get_result();
			THM_LOG_I ("reqEcho02sync ThreadB return [%d]\n", rslt);

			assert (rslt == threadmgr::result::success);

			section_id = SECTID_REQ_ECHO02_THB_SYNC_TIMEOUT;
			act = threadmgr::action::continue_;

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

			threadmgr::result rslt = p_if->get_source().get_result();
			THM_LOG_I ("reqEcho02sync ThreadB return [%d]\n", rslt);

			// タイムアウトを受け取ること
			assert (rslt == threadmgr::result::request_timeout);

			section_id = SECTID_REQ_ECHO02_THB_MULTI;
			act = threadmgr::action::continue_;

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

			section_id = SECTID_WAIT_ECHO02_THB_MULTI;
			act = threadmgr::action::wait;

			}
			break;

		case SECTID_WAIT_ECHO02_THB_MULTI: {
			threadmgr::result rslt = p_if->get_source().get_result();
			THM_LOG_I ("reqEcho02 ThreadB return [%d]\n", rslt);

			assert (rslt == threadmgr::result::success);

			// 最後に送ったリクエストのリプライがくるまで待ちます
			if (p_if->get_source().get_request_id() != m_tmp_req_id) {
				THM_LOG_W ("different reqid %d\n", p_if->get_source().get_request_id());
				section_id = SECTID_WAIT_ECHO02_THB_MULTI;
				act = threadmgr::action::wait;

			} else {
				section_id = SECTID_REQ_TEST_REG_NOTIFY_THC;
				act = threadmgr::action::continue_;
			}

			} break;

		case SECTID_REQ_TEST_REG_NOTIFY_THC: {
			CModuleC_extern ex(get_external_if());
			ex.req_test_reg_notify ();
			section_id = SECTID_WAIT_TEST_REG_NOTIFY_THC;
			act = threadmgr::action::wait;
			}
			break;

		case SECTID_WAIT_TEST_REG_NOTIFY_THC: {
			threadmgr::result rslt = p_if->get_source().get_result();
			m_client_id = *(p_if->get_source().get_message().data);
			THM_LOG_I ("return reqRegNotify ThreadC [%d] m_client_id:[%d]\n", rslt, m_client_id);

			assert (rslt == threadmgr::result::success);

			section_id = SECTID_CHECK_NOTIFY;
			act = threadmgr::action::continue_;

			} break;

		case SECTID_CHECK_NOTIFY:
			p_if->clear_timeout();
			p_if->set_timeout(500);
			section_id = SECTID_WAIT_NOTIFY;
			act = threadmgr::action::wait;
			break;

		case SECTID_WAIT_NOTIFY:
			if (m_is_notified) {
				m_is_notified = false;
				section_id = SECTID_REQ_TEST_UNREG_NOTIFY_THC;
				act = threadmgr::action::continue_;
			} else {
				section_id = SECTID_CHECK_NOTIFY;
				act = threadmgr::action::continue_;
			}
			break;

		case SECTID_REQ_TEST_UNREG_NOTIFY_THC: {
			CModuleC_extern ex(get_external_if());
			ex.req_test_unreg_notify (m_client_id);
			section_id = SECTID_WAIT_TEST_UNREG_NOTIFY_THC;
			act = threadmgr::action::wait;
			}
			break;

		case SECTID_WAIT_TEST_UNREG_NOTIFY_THC: {
			threadmgr::result rslt = p_if->get_source().get_result();
			THM_LOG_I ("return reqUnregNotify ThreadC [%d]\n", rslt);

			assert (rslt == threadmgr::result::success);

			section_id = SECTID_END;
			act = threadmgr::action::continue_;

			} break;

		case SECTID_END: {
			const char *msg = "success";
			p_if->reply (threadmgr::result::success, reinterpret_cast<uint8_t*>(const_cast<char*>(msg)), strlen(msg)+1);
			section_id = threadmgr::section_id::init;
			act = threadmgr::action::done;
			} break;

		default:
			break;
		}

		p_if->set_section_id (section_id, act);
	}

	void test_lock (threadmgr::CThreadMgrIf *p_if) {
		threadmgr::section_id::type section_id;
		threadmgr::action act;
		enum {
			SECTID_ENTRY = threadmgr::section_id::init,
			SECTID_REQ_ECHO01_THB,
			SECTID_WAIT_ECHO01_THB,
			SECTID_END,
		};

		section_id = p_if->get_section_id();
		THM_LOG_I ("%s section_id %d\n", __PRETTY_FUNCTION__, section_id);

		switch (section_id) {
		case SECTID_ENTRY: {

			bool is_enable_lock = *(reinterpret_cast<bool*>(p_if->get_source().get_message().data));
			if (is_enable_lock) {
				p_if->lock();
			}

			section_id = SECTID_REQ_ECHO01_THB;
			act = threadmgr::action::continue_;

			}
			break;

		case SECTID_REQ_ECHO01_THB: {

			// スレッドBのecho01にリクエスト
			CModuleB_extern ex (get_external_if());
			ex.req_echo01 ();

			section_id = SECTID_WAIT_ECHO01_THB;
			act = threadmgr::action::wait;
			}
			break;

		case SECTID_WAIT_ECHO01_THB: {
			threadmgr::result rslt = p_if->get_source().get_result();
			THM_LOG_I ("reqEcho01 ThreadB return [%d]\n", rslt);

			assert (rslt == threadmgr::result::success);

			section_id = SECTID_END;
			act = threadmgr::action::continue_;

			}
			break;

		case SECTID_END:
			p_if->unlock();

			m_lock_check << "checked.";

			p_if->reply (threadmgr::result::success, reinterpret_cast<uint8_t*>(const_cast<char*>(m_lock_check.str().c_str())), m_lock_check.str().length());

			section_id = threadmgr::section_id::init;
			act = threadmgr::action::done;
			break;

		default:
			break;
		}

		p_if->set_section_id (section_id, act);
	}

	void test_lock_intr (threadmgr::CThreadMgrIf *p_if) {
		threadmgr::section_id::type section_id;
		threadmgr::action act;
		enum {
			SECTID_ENTRY = threadmgr::section_id::init,
			SECTID_END,
		};

		section_id = p_if->get_section_id();
		THM_LOG_I ("%s section_id %d\n", __PRETTY_FUNCTION__, section_id);

		m_lock_check << "intr.";

		p_if->reply (threadmgr::result::success, reinterpret_cast<uint8_t*>(const_cast<char*>(m_lock_check.str().c_str())), m_lock_check.str().length());

		section_id = threadmgr::section_id::init;
		act = threadmgr::action::done;
		p_if->set_section_id (section_id, act);
	}

	void test_overwrite (threadmgr::CThreadMgrIf *p_if) {
		threadmgr::section_id::type section_id;
		threadmgr::action act;
		enum {
			SECTID_ENTRY = threadmgr::section_id::init,
			SECTID_REQ_ECHO02_THB,
			SECTID_WAIT_ECHO02_THB,
			SECTID_END,
		};

		section_id = p_if->get_section_id();
		THM_LOG_I ("%s section_id %d\n", __PRETTY_FUNCTION__, section_id);

		switch (section_id) {
		case SECTID_ENTRY: {

			p_if->enable_overwrite();

			m_ow_check << "ch";
		
			section_id = SECTID_REQ_ECHO02_THB;
			act = threadmgr::action::continue_;

			}
			break;

		case SECTID_REQ_ECHO02_THB: {

			// スレッドBのecho02にリクエスト
			CModuleB_extern ex (get_external_if());
			ex.req_echo02 (&m_tmp_req_id);
			THM_LOG_I ("reqEcho02 ThreadB reqid:[%d]\n", m_tmp_req_id);

			section_id = SECTID_WAIT_ECHO02_THB;
			act = threadmgr::action::wait;
			}
			break;

		case SECTID_WAIT_ECHO02_THB: {
			threadmgr::result rslt = p_if->get_source().get_result();
			THM_LOG_I ("reqEcho02 ThreadB return [%d]\n", rslt);

			assert (rslt == threadmgr::result::success);

			// 最後に送ったリクエストのリプライがくるまで待ちます
			if (p_if->get_source().get_request_id() != m_tmp_req_id) {
				THM_LOG_W ("different reqid %d\n", p_if->get_source().get_request_id());
				section_id = SECTID_WAIT_ECHO02_THB;
				act = threadmgr::action::wait;

			} else {
				section_id = SECTID_END;
				act = threadmgr::action::continue_;
			}

			}
			break;

		case SECTID_END:
			p_if->disable_overwrite();

			m_ow_check << "ecked.";

			p_if->reply (threadmgr::result::success, reinterpret_cast<uint8_t*>(const_cast<char*>(m_ow_check.str().c_str())), m_ow_check.str().length());

			section_id = threadmgr::section_id::init;
			act = threadmgr::action::done;
			break;

		default:
			break;
		}

		p_if->set_section_id (section_id, act);
	}

	void test_destroy (threadmgr::CThreadMgrIf *p_if) {
		threadmgr::section_id::type section_id;
		threadmgr::action act;
		enum {
			SECTID_ENTRY = threadmgr::section_id::init,
			SECTID_END,
		};

		section_id = p_if->get_section_id();
		THM_LOG_I ("%s section_id %d\n", __PRETTY_FUNCTION__, section_id);


		// dump
//		kill (getpid(), SIGQUIT);
		// req-destroy
		kill (getpid(), SIGTERM);


		section_id = threadmgr::section_id::init;
		act = threadmgr::action::done;
		p_if->set_section_id (section_id, act);
	}

	void on_receive_notify (threadmgr::CThreadMgrIf *p_if) override {
		THM_LOG_I ("%s\n", __PRETTY_FUNCTION__);

		assert(p_if->get_source().get_client_id() == m_client_id);

		if (p_if->get_source().get_client_id() == m_client_id) {
			THM_LOG_I ("recv notify  id:[%d] msg:[%s]\n",
						p_if->get_source().get_client_id(), reinterpret_cast<char*>(p_if->get_source().get_message().data));

//			// set without-reply
//			uint32_t opt = get_request_option ();
//			opt |= REQUEST_OPTION__WITHOUT_REPLY;
//			set_request_option (opt);
//
//			request_async(EN_MODULE_A, CModuleA_extern::EN_SEQ_FUNC02);
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
