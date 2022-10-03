#include <iostream>
#include <ostream>
#include <vector>
#include <string>

// include library header
#include <threadmgr/ThreadMgrpp.h>

enum {
	_MODULE_A = 0,
	_MODULE_B,
	_MODULE_C,
};

enum {
	_SEQ_1 = 0,
	_SEQ_2,
	_SEQ_3,
	_SEQ_4,
};

class CModuleA : public threadmgr::CThreadMgrBase
{
public:
	CModuleA (std::string name, uint8_t que_max) : CThreadMgrBase (name.c_str(), que_max) {
		std::vector<threadmgr::SEQ_BASE_t> seqs;
		seqs.push_back ({[&](threadmgr::CThreadMgrIf *p_if){sequence1(p_if);}, "sequence1"});
		seqs.push_back ({[&](threadmgr::CThreadMgrIf *p_if){sequence2(p_if);}, "sequence2"});
		// register your sequences (maximum of registration: 64)
		set_seqs (seqs);
	}
	virtual ~CModuleA (void) {}

private:
	// implements your sequences (member functions)

	// 1-shot sequence (simple-echo)
	void sequence1 (threadmgr::CThreadMgrIf *p_if) {
		// get request message.
		char *msg = (char*)(p_if->get_source()->msg.pMsg);
		size_t msglen = p_if->get_source()->msg.size;
		
		// send reply (maximum of message size: 256bytes)
		p_if->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, msglen);
		
	 	// at the end of sequence, 
		// set THM_SECT_ID_INIT, EN_THM_ACT_DONE with set_sect_id().
	 	p_if->set_sect_id (THM_SECT_ID_INIT, EN_THM_ACT_DONE);
	}

	// separated section sequence
	void sequence2 (threadmgr::CThreadMgrIf *p_if) {
		enum {
			SECTID_REQ_MODB_SEQ2 = THM_SECT_ID_INIT,
			SECTID_WAIT_MODB_SEQ2,
			SECTID_END,
		};

		EN_THM_ACT action;
		uint8_t section_id = p_if->get_sect_id();
		switch (section_id) {
		case SECTID_REQ_MODB_SEQ2: {

			// request to CModuleB::sequence2
			request_async(_MODULE_B, _SEQ_2);
	
			std::cout << __PRETTY_FUNCTION__ << " request CModuleB::sequence1" << std::endl;

			// set next section_id and action
			section_id = SECTID_WAIT_MODB_SEQ2;
			// wait for reply
			// while waiting this module can execute other sequences.
			action = EN_THM_ACT_WAIT;

			// If you don't want to execute other sequences, call lock() in advance.
			//p_if->lock();
			}
			break;

		case SECTID_WAIT_MODB_SEQ2: {
			EN_THM_RSLT rslt = p_if->get_source()->enRslt;

			std::cout << __PRETTY_FUNCTION__ << " reply CModuleB::sequence1 [" << rslt << "]" << std::endl; // "[1]" --> EN_THM_RSLT_SUCCESS

			// set next section_id and action
			section_id = SECTID_END;
			action = EN_THM_ACT_CONTINUE;

			// don't forget to unlock() when you call lock()
			//p_if->unlock();
			}
			break;

		case SECTID_END:
			// send reply
			p_if->reply (EN_THM_RSLT_SUCCESS);

			// at the end of sequence, 
			// set THM_SECT_ID_INIT, EN_THM_ACT_DONE with set_sect_id().
			section_id = THM_SECT_ID_INIT;
			action = EN_THM_ACT_DONE;
			break;

		default:
			break;
		}

		p_if->set_sect_id (section_id, action);
	}

};

class CModuleB : public threadmgr::CThreadMgrBase
{
public:
	CModuleB (std::string name, uint8_t que_max) : CThreadMgrBase (name.c_str(), que_max) {
		std::vector<threadmgr::SEQ_BASE_t> seqs;
		seqs.push_back ({[&](threadmgr::CThreadMgrIf *p_if){sequence1(p_if);}, "sequence1"});
		seqs.push_back ({[&](threadmgr::CThreadMgrIf *p_if){sequence2(p_if);}, "sequence2"});
		// register your sequences (maximum of registration: 64)
		set_seqs (seqs);
	}
	virtual ~CModuleB (void) {}

private:
	// implements your sequences (member functions)

	// 1-shot sequence
	void sequence1 (threadmgr::CThreadMgrIf *p_if) {

		// implement something
		sleep (1);
		std::cout << __PRETTY_FUNCTION__ << " done." << std::endl;

		// send reply
		p_if->reply (EN_THM_RSLT_SUCCESS);
		//p_if->reply (EN_THM_RSLT_ERROR);
		
	 	// at the end of sequence, set THM_SECT_ID_INIT, EN_THM_ACT_DONE with set_sect_id().
	 	p_if->set_sect_id (THM_SECT_ID_INIT, EN_THM_ACT_DONE);
	}

	// separated section sequence
	void sequence2 (threadmgr::CThreadMgrIf *p_if) {
		enum {
			SECTID_REQ_MODC_SEQ4 = THM_SECT_ID_INIT,
			SECTID_WAIT_MODC_SEQ4,
			SECTID_END,
		};

		EN_THM_ACT action;
		uint8_t section_id = p_if->get_sect_id();
		switch (section_id) {
		case SECTID_REQ_MODC_SEQ4: {

			// request to CModuleC::sequence4
			request_async(_MODULE_C, _SEQ_4);
	
			std::cout << __PRETTY_FUNCTION__ << " request CModuleC::sequence3" << std::endl;

			// set next section_id and action
			section_id = SECTID_WAIT_MODC_SEQ4;
			// wait for reply
			// while waiting this module can execute other sequences.
			action = EN_THM_ACT_WAIT;

			// If you don't want to execute other sequences, call lock() in advance.
			//p_if->lock();
			}
			break;

		case SECTID_WAIT_MODC_SEQ4: {
			EN_THM_RSLT rslt = p_if->get_source()->enRslt;

			std::cout << __PRETTY_FUNCTION__ << " reply CModuleC::sequence4 [" << rslt << "]" << std::endl; // "[1]" --> EN_THM_RSLT_SUCCESS

			// set next section_id and action
			section_id = SECTID_END;
			action = EN_THM_ACT_CONTINUE;

			// don't forget to unlock() when you call lock()
			//p_if->unlock();
			}
			break;

		case SECTID_END:
			// send reply
			p_if->reply (EN_THM_RSLT_SUCCESS);

			// at the end of sequence, 
			// set THM_SECT_ID_INIT, EN_THM_ACT_DONE with set_sect_id().
			section_id = THM_SECT_ID_INIT;
			action = EN_THM_ACT_DONE;
			break;

		default:
			break;
		}

		p_if->set_sect_id (section_id, action);
	}
};

class CModuleC : public threadmgr::CThreadMgrBase
{
public:
	CModuleC (std::string name, uint8_t que_max) : CThreadMgrBase (name.c_str(), que_max) {
		std::vector<threadmgr::SEQ_BASE_t> seqs;
		seqs.push_back ({[&](threadmgr::CThreadMgrIf *p_if){sequence1(p_if);}, "sequence1"});
		seqs.push_back ({[&](threadmgr::CThreadMgrIf *p_if){sequence2(p_if);}, "sequence2"});
		seqs.push_back ({[&](threadmgr::CThreadMgrIf *p_if){sequence2(p_if);}, "sequence3"});
		seqs.push_back ({[&](threadmgr::CThreadMgrIf *p_if){sequence2(p_if);}, "sequence4"});
		// register your sequences (maximum of registration: 64)
		set_seqs (seqs);
	}
	virtual ~CModuleC (void) {}

private:
	enum {
		_NOTIFY_CATEGORY_1 = 0,
	};

	// implements your sequences (member functions)
	
	// loop sequence
	void sequence1 (threadmgr::CThreadMgrIf *p_if) {
		uint8_t nSectId;
		EN_THM_ACT enAct;
		enum {
			SECTID_ENTRY = THM_SECT_ID_INIT,
			SECTID_LOOP,
			SECTID_SEND_NOTIFY,
		};

		nSectId = p_if->get_sect_id();
		switch (nSectId) {
		case SECTID_ENTRY:
			// since SECTID_LOOP loops infinitely, it is one way to reply first.
			p_if->reply (EN_THM_RSLT_SUCCESS);

			nSectId = SECTID_LOOP;
			enAct = EN_THM_ACT_CONTINUE;
			break;

		case SECTID_LOOP:
			// wait for a second
			// while waiting this module can execute other sequences.
			p_if->set_timeout (1000);
			nSectId = SECTID_SEND_NOTIFY;
			enAct = EN_THM_ACT_WAIT;
			break;

		case SECTID_SEND_NOTIFY: {
			// wakeup after a second
			
			std::string msg = "this is notify message...";
			// send notify
			// distribute to all registered clients by regNotify()
			p_if->notify (_NOTIFY_CATEGORY_1, (uint8_t*)msg.c_str(), msg.length());

			nSectId = SECTID_LOOP;
			enAct = EN_THM_ACT_CONTINUE;
			}
			break;
		}

		p_if->set_sect_id (nSectId, enAct);
	}

	// register notify
	void sequence2 (threadmgr::CThreadMgrIf *p_if) {
		// call regNotify() to register requester, also getiing client_id
		uint8_t client_id;
		bool rslt = p_if->reg_notify (_NOTIFY_CATEGORY_1, &client_id);
		if (rslt) {
			// client_id in reply-message
			p_if->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)&client_id, sizeof(client_id));
		} else {
			p_if->reply (EN_THM_RSLT_ERROR);
		}

		p_if->set_sect_id (THM_SECT_ID_INIT, EN_THM_ACT_DONE);
	}

	// unregister notify
	void sequence3 (threadmgr::CThreadMgrIf *p_if) {
		// client_id in request-message
		uint8_t client_id = *(p_if->get_source()->msg.pMsg);
		// call unregNotify() to unregister requester
		bool rslt = p_if->unreg_notify (_NOTIFY_CATEGORY_1, client_id);
		if (rslt) {
			p_if->reply (EN_THM_RSLT_SUCCESS);
		} else {
			p_if->reply (EN_THM_RSLT_ERROR);
		}

		p_if->set_sect_id (THM_SECT_ID_INIT, EN_THM_ACT_DONE);

	}

	// 1-shot sequence
	void sequence4 (threadmgr::CThreadMgrIf *p_if) {

		// implement something
		sleep (1);
		std::cout << __PRETTY_FUNCTION__ << " done." << std::endl;

		// send reply
		p_if->reply (EN_THM_RSLT_SUCCESS);
		//p_if->reply (EN_THM_RSLT_ERROR);
		
	 	// at the end of sequence, set THM_SECT_ID_INIT, EN_THM_ACT_DONE with set_sect_id().
	 	p_if->set_sect_id (THM_SECT_ID_INIT, EN_THM_ACT_DONE);
	}

};

int main (void)
{
	setLogFileptr(stdout);

	threadmgr::CThreadMgr *p_mgr = threadmgr::CThreadMgr::get_instance();

	// create your class instances (maximum of queue buffer size: 100)
	auto module_a = std::make_shared <CModuleA>("ModuleA", 10); // (name, queue buffer size)
	auto module_b = std::make_shared <CModuleB>("ModuleB", 10);
	auto module_c = std::make_shared <CModuleC>("ModuleC", 10);
	std::vector<std::shared_ptr<threadmgr::CThreadMgrBase>> threads;
	threads.push_back(module_a); // enum _MODULE_A
	threads.push_back(module_b); // enum _MODULE_B
	threads.push_back(module_c); // enum _MODULE_C

	// register your instances (maximum of registration: 32)
	if (!p_mgr->setup (threads)) {
		THM_LOG_E ("setup failure.");
		exit (EXIT_FAILURE);
	}
	// threadmgr-framework was ready.


	// If you want to make a request from an external thread, please execute createExternalCp().
	p_mgr->get_external_if()->create_external_cp();

	{
		std::cout << "request CModuleA::sequence1" << std::endl;
		std::string msg = "test-message";
		// syncronized request to simple-echo of CModuleA.
		p_mgr->get_external_if()->request_sync(_MODULE_A, _SEQ_1, (uint8_t*)msg.c_str(), msg.length());
		// main-thread wait for reply.
		ST_THM_SRC_INFO* r = p_mgr->get_external_if()-> receive_external();
		std::cout << "reply CModuleA::sequence1 [" << r->enRslt << "]" << std::endl;          // "[1]" --> EN_THM_RSLT_SUCCESS
		std::cout << "reply CModuleA::sequence1 [" << (char*)r->msg.pMsg << "]" << std::endl; // "[test-message]"
	}


	{
		std::cout << "request CModuleA::sequence2" << std::endl;
		std::string msg = "test-message";
		// syncronized request to CModuleA::sequence2.
		p_mgr->get_external_if()->request_sync(_MODULE_A, _SEQ_2);
		// main-thread wait for reply.
		ST_THM_SRC_INFO* r = p_mgr->get_external_if()-> receive_external();
		std::cout << "reply CModuleA::sequence2 [" << r->enRslt << "]" << std::endl;          // "[1]" --> EN_THM_RSLT_SUCCESS
	}



	// threads return wait, as daemon process.
	p_mgr->wait ();


	p_mgr->get_external_if()->destroy_external_cp();
	p_mgr->teardown();


	exit (EXIT_SUCCESS);
}
