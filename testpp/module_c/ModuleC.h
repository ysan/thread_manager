#ifndef _MODULEC_H_
#define _MODULEC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrBase.h"
#include "ThreadMgrpp.h"

#include "ModuleC_extern.h"
#include "modules.h"


class CModuleC : public threadmgr::CThreadMgrBase
{
public:
	CModuleC (std::string name, uint8_t que_max)
		: CThreadMgrBase (name, que_max)
	{
		threadmgr::sequence_t sequences[static_cast<int>(CModuleC_extern::sequence::max)] = {
			{[&](threadmgr::CThreadMgrIf *p_if){startup(p_if);}, "startup"},
			{[&](threadmgr::CThreadMgrIf *p_if){test_reg_notify(p_if);}, "test_reg_notify"},
			{[&](threadmgr::CThreadMgrIf *p_if){test_unreg_noitfy(p_if);}, "test_unreg_notify"},
			{[&](threadmgr::CThreadMgrIf *p_if){test_notify(p_if);}, "test_notify"},
			{[&](threadmgr::CThreadMgrIf *p_if){echo00(p_if);}, "echo00"},
			{[&](threadmgr::CThreadMgrIf *p_if){echo01(p_if);}, "echo01"},
			{[&](threadmgr::CThreadMgrIf *p_if){echo02(p_if);}, "echo02"},
		};
		set_sequences (sequences, static_cast<int>(CModuleC_extern::sequence::max));
	}

	virtual ~CModuleC (void) {
	}

private:
	void startup (threadmgr::CThreadMgrIf *p_if) {
		threadmgr::section_id::type section_id;
		threadmgr::action act;
		enum {
			SECTID_ENTRY = threadmgr::section_id::init,
			SECTID_REQ_TEST_NOTIFY,
			SECTID_WAIT_TEST_NOTIFY,
			SECTID_END,
			SECTID_ERR_END,
		};

		section_id = p_if->get_section_id();
		THM_LOG_I ("%s section_id %d\n", __PRETTY_FUNCTION__, section_id);

		switch (section_id) {
		case SECTID_ENTRY:
			section_id = SECTID_REQ_TEST_NOTIFY;
			act = threadmgr::action::continue_;
			break;

		case SECTID_REQ_TEST_NOTIFY:
			request_async (static_cast<int>(module::module_c), static_cast<int>(CModuleC_extern::sequence::test_notify));
			section_id = SECTID_WAIT_TEST_NOTIFY;
			act = threadmgr::action::wait;
			break;

		case SECTID_WAIT_TEST_NOTIFY: {
			threadmgr::result enRslt = p_if->get_source().get_result();
			if (enRslt == threadmgr::result::success) {
				section_id = SECTID_END;
				act = threadmgr::action::continue_;
			} else {
				section_id = SECTID_ERR_END;
				act = threadmgr::action::continue_;
			}

			}
			break;

		case SECTID_END: {
			const char *msg = "ModuleC startup end.";
			p_if->reply (threadmgr::result::success, reinterpret_cast<uint8_t*>(const_cast<char*>(msg)), strlen(msg)+1);
			section_id = threadmgr::section_id::init;
			act = threadmgr::action::done;
			} break;

		case SECTID_ERR_END:
			p_if->reply (threadmgr::result::error);
			section_id = threadmgr::section_id::init;
			act = threadmgr::action::done;
			break;

		default:
			break;
		}

		p_if->set_section_id (section_id, act);
	}
	
	void test_reg_notify (threadmgr::CThreadMgrIf *p_if) {
		threadmgr::section_id::type section_id;
		threadmgr::action act;
		enum {
			SECTID_ENTRY = threadmgr::section_id::init,
			SECTID_END,
		};

		section_id = p_if->get_section_id();
		THM_LOG_I ("%s section_id %d\n", __PRETTY_FUNCTION__, section_id);

		uint8_t client_id;
		bool rslt = p_if->reg_notify (static_cast<int>(CModuleC_extern::notify_cat::cat_1), &client_id);

		threadmgr::result enRslt;
		if (rslt) {
			enRslt = threadmgr::result::success;
		} else {
			enRslt = threadmgr::result::error;
		}

		// client_idをmsgで返す
		p_if->reply (enRslt, reinterpret_cast<uint8_t*>(&client_id), sizeof(client_id));

		section_id = threadmgr::section_id::init;
		act = threadmgr::action::done;
		p_if->set_section_id (section_id, act);
	}
	
	void test_unreg_noitfy (threadmgr::CThreadMgrIf *p_if) {
		threadmgr::section_id::type section_id;
		threadmgr::action act;
		enum {
			SECTID_ENTRY = threadmgr::section_id::init,
			SECTID_END,
		};

		section_id = p_if->get_section_id();
		THM_LOG_I ("%s section_id %d\n", __PRETTY_FUNCTION__, section_id);

		threadmgr::result enRslt;

		// msgからidを取得
		uint8_t id = *(p_if->get_source().get_message().data);
		bool rslt = p_if->unreg_notify (static_cast<int>(CModuleC_extern::notify_cat::cat_1), id);
		if (rslt) {
			enRslt = threadmgr::result::success;
		} else {
			enRslt = threadmgr::result::error;
		}

		p_if->reply (enRslt);

		section_id = threadmgr::section_id::init;
		act = threadmgr::action::done;
		p_if->set_section_id (section_id, act);
	}
	
	void test_notify (threadmgr::CThreadMgrIf *p_if) {
		threadmgr::section_id::type section_id;
		threadmgr::action act;
		enum {
			SECTID_ENTRY = threadmgr::section_id::init,
			SECTID_CYCLE,
			SECTID_SEND_NOTIFY,
			SECTID_END,
		};

		section_id = p_if->get_section_id();
		THM_LOG_I ("%s section_id %d\n", __PRETTY_FUNCTION__, section_id);

		switch (section_id) {
		case SECTID_ENTRY:
			// 先にreplyしておく
			p_if->reply (threadmgr::result::success);
	
			section_id = SECTID_CYCLE;
			act = threadmgr::action::continue_;
			break;
		case SECTID_CYCLE:
			p_if->clear_timeout ();
			p_if->set_timeout (10000);
			section_id = SECTID_SEND_NOTIFY;
			act = threadmgr::action::wait;
			break;
		case SECTID_SEND_NOTIFY: {
			const char *msg = "this is notify message...";
			p_if->notify (static_cast<int>(CModuleC_extern::notify_cat::cat_1), reinterpret_cast<uint8_t*>(const_cast<char*>(msg)), strlen(msg)+1);
	
			section_id = SECTID_CYCLE;
			act = threadmgr::action::continue_;
			} break;
		case SECTID_END:
			break;
		default:
			break;
		}

		p_if->set_section_id (section_id, act);
	}
	
	void echo00 (threadmgr::CThreadMgrIf *p_if) {
		threadmgr::section_id::type section_id;
		threadmgr::action act;
		enum {
			SECTID_ENTRY = 0,
			SECTID_END,
		};

		section_id = p_if->get_section_id();
		THM_LOG_I ("%s section_id %d\n", __PRETTY_FUNCTION__, section_id);

		size_t msglen = p_if->get_source().get_message().size;
		if (msglen == 0) {
			p_if->reply (threadmgr::result::success);
		} else {
			char *msg = reinterpret_cast<char*>(p_if->get_source().get_message().data);
			p_if->reply (threadmgr::result::success, reinterpret_cast<uint8_t*>(msg), msglen);
		}

		section_id = 0;
		act = threadmgr::action::done;
		p_if->set_section_id (section_id, act);
	}
	
	void echo01 (threadmgr::CThreadMgrIf *p_if) {
		threadmgr::section_id::type section_id;
		threadmgr::action act;
		enum {
			SECTID_ENTRY = 0,
			SECTID_END,
		};

		section_id = p_if->get_section_id();
		THM_LOG_I ("%s section_id %d\n", __PRETTY_FUNCTION__, section_id);

		THM_LOG_I ("execute. sleep 1sec.\n");
		sleep (1);

		size_t msglen = p_if->get_source().get_message().size;
		if (msglen == 0) {
			p_if->reply (threadmgr::result::success);
		} else {
			char *msg = reinterpret_cast<char*>(p_if->get_source().get_message().data);
			p_if->reply (threadmgr::result::success, reinterpret_cast<uint8_t*>(msg), msglen);
		}

		section_id = 0;
		act = threadmgr::action::done;
		p_if->set_section_id (section_id, act);
	}
	
	void echo02 (threadmgr::CThreadMgrIf *p_if) {
		threadmgr::section_id::type section_id;
		threadmgr::action act;
		enum {
			SECTID_ENTRY = 0,
			SECTID_END,
		};

		section_id = p_if->get_section_id();
		THM_LOG_I ("%s section_id %d\n", __PRETTY_FUNCTION__, section_id);

		THM_LOG_I ("execute. sleep 2sec.\n");
		sleep (2);

		size_t msglen = p_if->get_source().get_message().size;
		if (msglen == 0) {
			p_if->reply (threadmgr::result::success);
		} else {
			char *msg = reinterpret_cast<char*>(p_if->get_source().get_message().data);
			p_if->reply (threadmgr::result::success, reinterpret_cast<uint8_t*>(msg), msglen);
		}

		section_id = 0;
		act = threadmgr::action::done;
		p_if->set_section_id (section_id, act);
	}

};

#endif
