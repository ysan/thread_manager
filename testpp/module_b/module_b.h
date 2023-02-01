#ifndef _MODULE_B_H_
#define _MODULE_B_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrBase.h"
#include "ThreadMgrpp.h"

#include "module_b_extern.h"


class module_b : public threadmgr::CThreadMgrBase
{
public:
	module_b (std::string name, uint8_t que_max)
		: CThreadMgrBase (name, que_max)
	{
		std::vector<threadmgr::sequence_t> sequences;
		sequences.push_back ({[&](threadmgr::CThreadMgrIf *p_if){startup(p_if);}, std::move("startup")});
		sequences.push_back ({[&](threadmgr::CThreadMgrIf *p_if){echo00(p_if);}, std::move("echo00")});
		sequences.push_back ({[&](threadmgr::CThreadMgrIf *p_if){echo01(p_if);}, std::move("echo01")});
		sequences.push_back ({[&](threadmgr::CThreadMgrIf *p_if){echo02(p_if);}, std::move("echo02")});
		set_sequences (sequences);
	}

	virtual ~module_b (void) {
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

		const char *msg = "module_b startup end.";
		p_if->reply (threadmgr::result::success, reinterpret_cast<uint8_t*>(const_cast<char*>(msg)), strlen(msg)+1);

		section_id = threadmgr::section_id::init;
		act = threadmgr::action::done;
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

		size_t msglen = p_if->get_source().get_message().length();
		if (msglen == 0) {
			p_if->reply (threadmgr::result::success);
		} else {
			char *msg = reinterpret_cast<char*>(p_if->get_source().get_message().data());
			p_if->reply (threadmgr::result::success, reinterpret_cast<uint8_t*>(const_cast<char*>(msg)), msglen);
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

		size_t msglen = p_if->get_source().get_message().length();
		if (msglen == 0) {
			p_if->reply (threadmgr::result::success);
		} else {
			char *msg = reinterpret_cast<char*>(p_if->get_source().get_message().data());
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

		size_t msglen = p_if->get_source().get_message().length();
		if (msglen == 0) {
			p_if->reply (threadmgr::result::success);
		} else {
			char *msg = reinterpret_cast<char*>(p_if->get_source().get_message().data());
			p_if->reply (threadmgr::result::success, reinterpret_cast<uint8_t*>(msg), msglen);
		}

		section_id = 0;
		act = threadmgr::action::done;
		p_if->set_section_id (section_id, act);
	}

};

#endif
