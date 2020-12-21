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

class CModuleA : public ThreadManager::CThreadMgrBase
{
public:
	CModuleA (std::string name, uint8_t nQueNum) : CThreadMgrBase (name.c_str(), nQueNum) {
		std::vector<ThreadManager::SEQ_BASE_t> seqs;
		seqs.push_back ({(ThreadManager::PFN_SEQ_BASE)&CModuleA::sequence1, "sequence1"}); // enum _SEQ_1
		seqs.push_back ({(ThreadManager::PFN_SEQ_BASE)&CModuleA::sequence2, "sequence2"}); // enum _SEQ_2
		// register your sequences (maximum of registration: 64)
		setSeqs (seqs);
	}
	virtual ~CModuleA (void) {}

private:
	// implements your sequences (member functions)

	// 1-shot sequence (simple-echo)
	void sequence1 (ThreadManager::CThreadMgrIf *pIf) {
		// get request message.
		char *msg = (char*)(pIf->getSrcInfo()->msg.pMsg);
		size_t msglen = pIf->getSrcInfo()->msg.size;
		
		// send reply (maximum of message size: 256bytes)
		pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, msglen);
		
	 	// at the end of sequence, 
		// set THM_SECT_ID_INIT, EN_THM_ACT_DONE with setSectId().
	 	pIf->setSectId (THM_SECT_ID_INIT, EN_THM_ACT_DONE);
	}

	// separated section sequence
	void sequence2 (ThreadManager::CThreadMgrIf *pIf) {
		enum {
			SECTID_REQ_MODB_SEQ2 = THM_SECT_ID_INIT,
			SECTID_WAIT_MODB_SEQ2,
			SECTID_END,
		};

		EN_THM_ACT action;
		uint8_t section_id = pIf->getSectId();
		switch (section_id) {
		case SECTID_REQ_MODB_SEQ2: {

			// request to CModuleB::sequence2
			requestAsync(_MODULE_B, _SEQ_2);
	
			std::cout << __PRETTY_FUNCTION__ << " request CModuleB::sequence1" << std::endl;

			// set next section_id and action
			section_id = SECTID_WAIT_MODB_SEQ2;
			// wait for reply
			// while waiting this module can execute other sequences.
			action = EN_THM_ACT_WAIT;

			// If you don't want to execute other sequences, call lock() in advance.
			//pIf->lock();
			}
			break;

		case SECTID_WAIT_MODB_SEQ2: {
			EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;

			std::cout << __PRETTY_FUNCTION__ << " reply CModuleB::sequence1 [" << enRslt << "]" << std::endl; // "[1]" --> EN_THM_RSLT_SUCCESS

			// set next section_id and action
			section_id = SECTID_END;
			action = EN_THM_ACT_CONTINUE;

			// don't forget to unlock() when you call lock()
			//pIf->unlock();
			}
			break;

		case SECTID_END:
			// send reply
			pIf->reply (EN_THM_RSLT_SUCCESS);

			// at the end of sequence, 
			// set THM_SECT_ID_INIT, EN_THM_ACT_DONE with setSectId().
			section_id = THM_SECT_ID_INIT;
			action = EN_THM_ACT_DONE;
			break;

		default:
			break;
		}

		pIf->setSectId (section_id, action);
	}

};

class CModuleB : public ThreadManager::CThreadMgrBase
{
public:
	CModuleB (std::string name, uint8_t nQueNum) : CThreadMgrBase (name.c_str(), nQueNum) {
		std::vector<ThreadManager::SEQ_BASE_t> seqs;
		seqs.push_back ({(ThreadManager::PFN_SEQ_BASE)&CModuleB::sequence1, "sequence1"}); // enum _SEQ_1
		seqs.push_back ({(ThreadManager::PFN_SEQ_BASE)&CModuleB::sequence2, "sequence2"}); // enum _SEQ_2
		// register your sequences (maximum of registration: 64)
		setSeqs (seqs);
	}
	virtual ~CModuleB (void) {}

private:
	// implements your sequences (member functions)

	// 1-shot sequence
	void sequence1 (ThreadManager::CThreadMgrIf *pIf) {

		// implement something
		sleep (1);
		std::cout << __PRETTY_FUNCTION__ << " done." << std::endl;

		// send reply
		pIf->reply (EN_THM_RSLT_SUCCESS);
		//pIf->reply (EN_THM_RSLT_ERROR);
		
	 	// at the end of sequence, set THM_SECT_ID_INIT, EN_THM_ACT_DONE with setSectId().
	 	pIf->setSectId (THM_SECT_ID_INIT, EN_THM_ACT_DONE);
	}

	// separated section sequence
	void sequence2 (ThreadManager::CThreadMgrIf *pIf) {
		enum {
			SECTID_REQ_MODC_SEQ4 = THM_SECT_ID_INIT,
			SECTID_WAIT_MODC_SEQ4,
			SECTID_END,
		};

		EN_THM_ACT action;
		uint8_t section_id = pIf->getSectId();
		switch (section_id) {
		case SECTID_REQ_MODC_SEQ4: {

			// request to CModuleC::sequence4
			requestAsync(_MODULE_C, _SEQ_4);
	
			std::cout << __PRETTY_FUNCTION__ << " request CModuleC::sequence3" << std::endl;

			// set next section_id and action
			section_id = SECTID_WAIT_MODC_SEQ4;
			// wait for reply
			// while waiting this module can execute other sequences.
			action = EN_THM_ACT_WAIT;

			// If you don't want to execute other sequences, call lock() in advance.
			//pIf->lock();
			}
			break;

		case SECTID_WAIT_MODC_SEQ4: {
			EN_THM_RSLT enRslt = pIf->getSrcInfo()->enRslt;

			std::cout << __PRETTY_FUNCTION__ << " reply CModuleC::sequence4 [" << enRslt << "]" << std::endl; // "[1]" --> EN_THM_RSLT_SUCCESS

			// set next section_id and action
			section_id = SECTID_END;
			action = EN_THM_ACT_CONTINUE;

			// don't forget to unlock() when you call lock()
			//pIf->unlock();
			}
			break;

		case SECTID_END:
			// send reply
			pIf->reply (EN_THM_RSLT_SUCCESS);

			// at the end of sequence, 
			// set THM_SECT_ID_INIT, EN_THM_ACT_DONE with setSectId().
			section_id = THM_SECT_ID_INIT;
			action = EN_THM_ACT_DONE;
			break;

		default:
			break;
		}

		pIf->setSectId (section_id, action);
	}
};

class CModuleC : public ThreadManager::CThreadMgrBase
{
public:
	CModuleC (std::string name, uint8_t nQueNum) : CThreadMgrBase (name.c_str(), nQueNum) {
		std::vector<ThreadManager::SEQ_BASE_t> seqs;
		seqs.push_back ({(ThreadManager::PFN_SEQ_BASE)&CModuleC::sequence1, "sequence1"}); // enum _SEQ_1
		seqs.push_back ({(ThreadManager::PFN_SEQ_BASE)&CModuleC::sequence2, "sequence2"}); // enum _SEQ_2
		seqs.push_back ({(ThreadManager::PFN_SEQ_BASE)&CModuleC::sequence3, "sequence3"}); // enum _SEQ_3
		seqs.push_back ({(ThreadManager::PFN_SEQ_BASE)&CModuleC::sequence4, "sequence4"}); // enum _SEQ_4
		// register your sequences (maximum of registration: 64)
		setSeqs (seqs);
	}
	virtual ~CModuleC (void) {}

private:
	enum {
		_NOTIFY_CATEGORY_1 = 0,
	};

	// implements your sequences (member functions)
	
	// loop sequence
	void sequence1 (ThreadManager::CThreadMgrIf *pIf) {
		uint8_t nSectId;
		EN_THM_ACT enAct;
		enum {
			SECTID_ENTRY = THM_SECT_ID_INIT,
			SECTID_LOOP,
			SECTID_SEND_NOTIFY,
		};

		nSectId = pIf->getSectId();
		switch (nSectId) {
		case SECTID_ENTRY:
			// since SECTID_LOOP loops infinitely, it is one way to reply first.
			pIf->reply (EN_THM_RSLT_SUCCESS);

			nSectId = SECTID_LOOP;
			enAct = EN_THM_ACT_CONTINUE;
			break;

		case SECTID_LOOP:
			// wait for a second
			// while waiting this module can execute other sequences.
			pIf->setTimeout (1000);
			nSectId = SECTID_SEND_NOTIFY;
			enAct = EN_THM_ACT_WAIT;
			break;

		case SECTID_SEND_NOTIFY: {
			// wakeup after a second
			
			std::string msg = "this is notify message...";
			// send notify
			// distribute to all registered clients by regNotify()
			pIf->notify (_NOTIFY_CATEGORY_1, (uint8_t*)msg.c_str(), msg.length());

			nSectId = SECTID_LOOP;
			enAct = EN_THM_ACT_CONTINUE;
			}
			break;
		}

		pIf->setSectId (nSectId, enAct);
	}

	// register notify
	void sequence2 (ThreadManager::CThreadMgrIf *pIf) {
		// call regNotify() to register requester, also getiing client_id
		uint8_t client_id;
		bool rslt = pIf->regNotify (_NOTIFY_CATEGORY_1, &client_id);
		if (rslt) {
			// client_id in reply-message
			pIf->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)&client_id, sizeof(client_id));
		} else {
			pIf->reply (EN_THM_RSLT_ERROR);
		}

		pIf->setSectId (THM_SECT_ID_INIT, EN_THM_ACT_DONE);
	}

	// unregister notify
	void sequence3 (ThreadManager::CThreadMgrIf *pIf) {
		// client_id in request-message
		uint8_t client_id = *(pIf->getSrcInfo()->msg.pMsg);
		// call unregNotify() to unregister requester
		bool rslt = pIf->unregNotify (_NOTIFY_CATEGORY_1, client_id);
		if (rslt) {
			pIf->reply (EN_THM_RSLT_SUCCESS);
		} else {
			pIf->reply (EN_THM_RSLT_ERROR);
		}

		pIf->setSectId (THM_SECT_ID_INIT, EN_THM_ACT_DONE);

	}

	// 1-shot sequence
	void sequence4 (ThreadManager::CThreadMgrIf *pIf) {

		// implement something
		sleep (1);
		std::cout << __PRETTY_FUNCTION__ << " done." << std::endl;

		// send reply
		pIf->reply (EN_THM_RSLT_SUCCESS);
		//pIf->reply (EN_THM_RSLT_ERROR);
		
	 	// at the end of sequence, set THM_SECT_ID_INIT, EN_THM_ACT_DONE with setSectId().
	 	pIf->setSectId (THM_SECT_ID_INIT, EN_THM_ACT_DONE);
	}

};

int main (void)
{
	initLogStdout();

	ThreadManager::CThreadMgr *p_mgr = ThreadManager::CThreadMgr::getInstance();

	// create your class instances (maximum of queue buffer size: 100)
	CModuleA *p_moduleA = new CModuleA("ModuleA", 10); // (name, queue buffer size)
	CModuleB *p_moduleB = new CModuleB("ModuleB", 10);
	CModuleC *p_moduleC = new CModuleC("ModuleC", 10);
	std::vector<ThreadManager::CThreadMgrBase*> threads;
	threads.push_back(p_moduleA); // enum _MODULE_A
	threads.push_back(p_moduleB); // enum _MODULE_B
	threads.push_back(p_moduleC); // enum _MODULE_C

	// register your instances (maximum of registration: 32)
	if (!p_mgr->setup (threads)) {
		THM_LOG_E ("setup failure.");
		exit (EXIT_FAILURE);
	}
	// ThreadManager-framework was ready.


	// If you want to make a request from an external thread, please execute createExternalCp().
	p_mgr->getExternalIf()->createExternalCp();

	{
		std::cout << "request CModuleA::sequence1" << std::endl;
		std::string msg = "test-message";
		// syncronized request to simple-echo of CModuleA.
		p_mgr->getExternalIf()->requestSync(_MODULE_A, _SEQ_1, (uint8_t*)msg.c_str(), msg.length());
		// main-thread wait for reply.
		ST_THM_SRC_INFO* r = p_mgr->getExternalIf()-> receiveExternal();
		std::cout << "reply CModuleA::sequence1 [" << r->enRslt << "]" << std::endl;          // "[1]" --> EN_THM_RSLT_SUCCESS
		std::cout << "reply CModuleA::sequence1 [" << (char*)r->msg.pMsg << "]" << std::endl; // "[test-message]"
	}


	{
		std::cout << "request CModuleA::sequence2" << std::endl;
		std::string msg = "test-message";
		// syncronized request to CModuleA::sequence2.
		p_mgr->getExternalIf()->requestSync(_MODULE_A, _SEQ_2);
		// main-thread wait for reply.
		ST_THM_SRC_INFO* r = p_mgr->getExternalIf()-> receiveExternal();
		std::cout << "reply CModuleA::sequence2 [" << r->enRslt << "]" << std::endl;          // "[1]" --> EN_THM_RSLT_SUCCESS
	}



	// threads return wait, as daemon process.
	p_mgr->wait ();


	p_mgr->getExternalIf()->destroyExternalCp();
	p_mgr->teardown();

	finalizLog();

	for (const auto &th : threads)
		delete th;

	exit (EXIT_SUCCESS);
}
