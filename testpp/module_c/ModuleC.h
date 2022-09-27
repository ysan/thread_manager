#ifndef _MODULEC_H_
#define _MODULEC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrpp.h"

#include "ModuleC_extern.h"
#include "modules.h"


class CModuleC : public threadmgr::CThreadMgrBase
{
public:
	CModuleC (std::string name, uint8_t que_max)
		: CThreadMgrBase (name, que_max)
	{
		threadmgr::SEQ_BASE_t seqs[static_cast<int>(CModuleC_extern::seq::max)] = {
			{[&](threadmgr::CThreadMgrIf *p_if){startup(p_if);}, "startup"},
			{[&](threadmgr::CThreadMgrIf *p_if){test_reg_notify(p_if);}, "test_reg_notify"},
			{[&](threadmgr::CThreadMgrIf *p_if){test_unreg_noitfy(p_if);}, "test_unreg_notify"},
			{[&](threadmgr::CThreadMgrIf *p_if){test_notify(p_if);}, "test_notify"},
			{[&](threadmgr::CThreadMgrIf *p_if){echo00(p_if);}, "echo00"},
			{[&](threadmgr::CThreadMgrIf *p_if){echo01(p_if);}, "echo01"},
			{[&](threadmgr::CThreadMgrIf *p_if){echo02(p_if);}, "echo02"},
		};
		set_seqs (seqs, static_cast<int>(CModuleC_extern::seq::max));
	}

	virtual ~CModuleC (void) {
	}

private:
	void startup (threadmgr::CThreadMgrIf *p_if) {
		uint8_t sect_id;
		EN_THM_ACT act;
		enum {
			SECTID_ENTRY = THM_SECT_ID_INIT,
			SECTID_REQ_TEST_NOTIFY,
			SECTID_WAIT_TEST_NOTIFY,
			SECTID_END,
			SECTID_ERR_END,
		};

		sect_id = p_if->get_sect_id();
		THM_LOG_I ("%s sect_id %d\n", __PRETTY_FUNCTION__, sect_id);

		switch (sect_id) {
		case SECTID_ENTRY:
			sect_id = SECTID_REQ_TEST_NOTIFY;
			act = EN_THM_ACT_CONTINUE;
			break;

		case SECTID_REQ_TEST_NOTIFY:
			request_async (static_cast<int>(module::module_c), static_cast<int>(CModuleC_extern::seq::test_notify));
			sect_id = SECTID_WAIT_TEST_NOTIFY;
			act = EN_THM_ACT_WAIT;
			break;

		case SECTID_WAIT_TEST_NOTIFY: {
			EN_THM_RSLT enRslt = p_if->get_source()->enRslt;
			if (enRslt == EN_THM_RSLT_SUCCESS) {
				sect_id = SECTID_END;
				act = EN_THM_ACT_CONTINUE;
			} else {
				sect_id = SECTID_ERR_END;
				act = EN_THM_ACT_CONTINUE;
			}

			}
			break;

		case SECTID_END: {
			const char *msg = "ModuleC startup end.";
			p_if->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, strlen(msg)+1);
			sect_id = THM_SECT_ID_INIT;
			act = EN_THM_ACT_DONE;
			} break;

		case SECTID_ERR_END:
			p_if->reply (EN_THM_RSLT_ERROR);
			sect_id = THM_SECT_ID_INIT;
			act = EN_THM_ACT_DONE;
			break;

		default:
			break;
		}

		p_if->set_sect_id (sect_id, act);
	}
	
	void test_reg_notify (threadmgr::CThreadMgrIf *p_if) {
		uint8_t sect_id;
		EN_THM_ACT act;
		enum {
			SECTID_ENTRY = THM_SECT_ID_INIT,
			SECTID_END,
		};

		sect_id = p_if->get_sect_id();
		THM_LOG_I ("%s sect_id %d\n", __PRETTY_FUNCTION__, sect_id);

		uint8_t client_id;
		bool rslt = p_if->reg_notify (static_cast<int>(CModuleC_extern::notify_cat::cat_1), &client_id);

		EN_THM_RSLT enRslt;
		if (rslt) {
			enRslt = EN_THM_RSLT_SUCCESS;
		} else {
			enRslt = EN_THM_RSLT_ERROR;
		}

		// client_idをmsgで返す
		p_if->reply (enRslt, (uint8_t*)&client_id, sizeof(client_id));

		sect_id = THM_SECT_ID_INIT;
		act = EN_THM_ACT_DONE;
		p_if->set_sect_id (sect_id, act);
	}
	
	void test_unreg_noitfy (threadmgr::CThreadMgrIf *p_if) {
		uint8_t sect_id;
		EN_THM_ACT act;
		enum {
			SECTID_ENTRY = THM_SECT_ID_INIT,
			SECTID_END,
		};

		sect_id = p_if->get_sect_id();
		THM_LOG_I ("%s sect_id %d\n", __PRETTY_FUNCTION__, sect_id);

		EN_THM_RSLT enRslt;

		// msgからidを取得
		uint8_t id = *(p_if->get_source()->msg.pMsg);
		bool rslt = p_if->unreg_notify (static_cast<int>(CModuleC_extern::notify_cat::cat_1), id);
		if (rslt) {
			enRslt = EN_THM_RSLT_SUCCESS;
		} else {
			enRslt = EN_THM_RSLT_ERROR;
		}

		p_if->reply (enRslt);

		sect_id = THM_SECT_ID_INIT;
		act = EN_THM_ACT_DONE;
		p_if->set_sect_id (sect_id, act);
	}
	
	void test_notify (threadmgr::CThreadMgrIf *p_if) {
		uint8_t sect_id;
		EN_THM_ACT act;
		enum {
			SECTID_ENTRY = THM_SECT_ID_INIT,
			SECTID_CYCLE,
			SECTID_SEND_NOTIFY,
			SECTID_END,
		};

		sect_id = p_if->get_sect_id();
		THM_LOG_I ("%s sect_id %d\n", __PRETTY_FUNCTION__, sect_id);

		switch (sect_id) {
		case SECTID_ENTRY:
			// 先にreplyしておく
			p_if->reply (EN_THM_RSLT_SUCCESS);
	
			sect_id = SECTID_CYCLE;
			act = EN_THM_ACT_CONTINUE;
			break;
		case SECTID_CYCLE:
			p_if->clear_timeout ();
			p_if->set_timeout (10000);
			sect_id = SECTID_SEND_NOTIFY;
			act = EN_THM_ACT_WAIT;
			break;
		case SECTID_SEND_NOTIFY: {
			const char *msg = "this is notify message...";
			p_if->notify (static_cast<int>(CModuleC_extern::notify_cat::cat_1), (uint8_t*)msg, strlen(msg)+1);
	
			sect_id = SECTID_CYCLE;
			act = EN_THM_ACT_CONTINUE;
			} break;
		case SECTID_END:
			break;
		default:
			break;
		}

		p_if->set_sect_id (sect_id, act);
	}
	
	void echo00 (threadmgr::CThreadMgrIf *p_if) {
		uint8_t sect_id;
		EN_THM_ACT act;
		enum {
			SECTID_ENTRY = 0,
			SECTID_END,
		};

		sect_id = p_if->get_sect_id();
		THM_LOG_I ("%s sect_id %d\n", __PRETTY_FUNCTION__, sect_id);

		size_t msglen = p_if->get_source()->msg.size;
		if (msglen == 0) {
			p_if->reply (EN_THM_RSLT_SUCCESS);
		} else {
			char *msg = (char*)p_if->get_source()->msg.pMsg;
			p_if->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, msglen);
		}

		sect_id = 0;
		act = EN_THM_ACT_DONE;
		p_if->set_sect_id (sect_id, act);
	}
	
	void echo01 (threadmgr::CThreadMgrIf *p_if) {
		uint8_t sect_id;
		EN_THM_ACT act;
		enum {
			SECTID_ENTRY = 0,
			SECTID_END,
		};

		sect_id = p_if->get_sect_id();
		THM_LOG_I ("%s sect_id %d\n", __PRETTY_FUNCTION__, sect_id);

		THM_LOG_I ("execute. sleep 1sec.\n");
		sleep (1);

		size_t msglen = p_if->get_source()->msg.size;
		if (msglen == 0) {
			p_if->reply (EN_THM_RSLT_SUCCESS);
		} else {
			char *msg = (char*)p_if->get_source()->msg.pMsg;
			p_if->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, msglen);
		}

		sect_id = 0;
		act = EN_THM_ACT_DONE;
		p_if->set_sect_id (sect_id, act);
	}
	
	void echo02 (threadmgr::CThreadMgrIf *p_if) {
		uint8_t sect_id;
		EN_THM_ACT act;
		enum {
			SECTID_ENTRY = 0,
			SECTID_END,
		};

		sect_id = p_if->get_sect_id();
		THM_LOG_I ("%s sect_id %d\n", __PRETTY_FUNCTION__, sect_id);

		THM_LOG_I ("execute. sleep 2sec.\n");
		sleep (2);

		size_t msglen = p_if->get_source()->msg.size;
		if (msglen == 0) {
			p_if->reply (EN_THM_RSLT_SUCCESS);
		} else {
			char *msg = (char*)p_if->get_source()->msg.pMsg;
			p_if->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, msglen);
		}

		sect_id = 0;
		act = EN_THM_ACT_DONE;
		p_if->set_sect_id (sect_id, act);
	}

};

#endif
