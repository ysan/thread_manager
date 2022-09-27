#ifndef _MODULEB_H_
#define _MODULEB_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrpp.h"

#include "ModuleB_extern.h"


class CModuleB : public threadmgr::CThreadMgrBase
{
public:
	CModuleB (std::string name, uint8_t que_max)
		: CThreadMgrBase (name, que_max)
	{
		std::vector<threadmgr::SEQ_BASE_t> seqs;
		seqs.push_back ({[&](threadmgr::CThreadMgrIf *p_if){startUp(p_if);}, "startUp"});
		seqs.push_back ({[&](threadmgr::CThreadMgrIf *p_if){echo00(p_if);}, "echo00"});
		seqs.push_back ({[&](threadmgr::CThreadMgrIf *p_if){echo01(p_if);}, "echo01"});
		seqs.push_back ({[&](threadmgr::CThreadMgrIf *p_if){echo02(p_if);}, "echo02"});
		set_seqs (seqs);
	}

	virtual ~CModuleB (void) {}

private:
	void startUp (threadmgr::CThreadMgrIf *p_if) {
		uint8_t sect_id;
		EN_THM_ACT act;
		enum {
			SECTID_ENTRY = THM_SECT_ID_INIT,
			SECTID_END,
		};

		sect_id = p_if->get_sect_id();
		THM_LOG_I ("%s sect_id %d\n", __PRETTY_FUNCTION__, sect_id);

		const char *msg = "ModuleB startup end.";
		p_if->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, strlen(msg)+1);

		sect_id = THM_SECT_ID_INIT;
		act = EN_THM_ACT_DONE;
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
