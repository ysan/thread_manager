#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <signal.h>

#include <vector>
#include <string>
#include <cassert>
#include <iostream>

#include "ThreadMgrpp.h"

#include "ModuleA.h"
#include "ModuleA_extern.h"
#include "ModuleB.h"
#include "ModuleB_extern.h"
#include "ModuleC.h"
#include "ModuleC_extern.h"

#include "modules.h"
#include "threadmgr_if.h"
#include "threadmgr_util.h"


int main (void)
{
	setLogFileptr(stdout);

	THM_LOG_D ("test THM_LOG_D");
	THM_LOG_I ("test THM_LOG_I");
	THM_LOG_W ("test THM_LOG_W");
	THM_LOG_E ("test THM_LOG_E");
	THM_PERROR ("test THM_PERROR");


	ThreadManager::CThreadMgr *p_mgr = ThreadManager::CThreadMgr::getInstance();

	CModuleA *p_moduleA = new CModuleA("ModuleA", 10);
	CModuleB *p_moduleB = new CModuleB("ModuleB", 10);
	CModuleC *p_moduleC = new CModuleC("ModuleC", 10);
	std::vector<ThreadManager::CThreadMgrBase*> threads;
	threads.push_back(p_moduleA);
	threads.push_back(p_moduleB);
	threads.push_back(p_moduleC);

	if (!p_mgr->setup (threads)) {
		THM_LOG_E ("setup failure.");
		exit (EXIT_FAILURE);
	}


	CModuleA_extern *p_mod_a_extern = new CModuleA_extern (p_mgr->getExternalIf());
	CModuleB_extern *p_mod_b_extern = new CModuleB_extern (p_mgr->getExternalIf());
	CModuleC_extern *p_mod_c_extern = new CModuleC_extern (p_mgr->getExternalIf());


	p_mgr->getExternalIf()->createExternalCp();

	{
		p_mod_a_extern-> reqStartUp ();
		ST_THM_SRC_INFO* r = p_mgr->getExternalIf()-> receiveExternal();
		assert(r != NULL);
		assert (r->enRslt == EN_THM_RSLT_SUCCESS);
		std::string s = (char*)r->msg.pMsg;
		assert (s == std::string("ModuleA startup end."));
	}
	{
		p_mod_b_extern-> reqStartUp ();
		ST_THM_SRC_INFO* r = p_mgr->getExternalIf()-> receiveExternal();
		assert(r != NULL);
		assert (r->enRslt == EN_THM_RSLT_SUCCESS);
		std::string s = (char*)r->msg.pMsg;
		assert (s == std::string("ModuleB startup end."));
	}
	{
		p_mod_c_extern-> reqStartUp ();
		ST_THM_SRC_INFO* r = p_mgr->getExternalIf()-> receiveExternal();
		assert(r != NULL);
		assert (r->enRslt == EN_THM_RSLT_SUCCESS);
		std::string s = (char*)r->msg.pMsg;
		assert (s == std::string("ModuleC startup end."));
	}

	// test request-reply, req-timeout, seq-timeout, notify
	{
		p_mod_a_extern->reqTestReqRep ();
		ST_THM_SRC_INFO* r = p_mgr->getExternalIf()-> receiveExternal();
		assert(r != NULL);
		assert (r->enRslt == EN_THM_RSLT_SUCCESS);
	}

	// test lock sequence
	{
		// set without-reply
		uint32_t opt = p_mgr->getExternalIf()->getRequestOption ();
		opt |= REQUEST_OPTION__WITHOUT_REPLY;
		p_mgr->getExternalIf()->setRequestOption (opt);

		p_mod_a_extern->reqTestLockIntr ();  // "intr."
		p_mod_a_extern->reqTestLock (true);  // "checked."
		p_mod_a_extern->reqTestLockIntr ();  // "intr."

		// reset without-reply
		opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
		p_mgr->getExternalIf()->setRequestOption (opt);

		p_mod_a_extern->reqTestLockIntr ();  // "intr."
		ST_THM_SRC_INFO* r = p_mgr->getExternalIf()-> receiveExternal();
		assert(r != NULL);
		assert (r->enRslt == EN_THM_RSLT_SUCCESS);
		std::string s = (char*)r->msg.pMsg;
		assert (s == std::string("intr.checked.intr.intr."));
	}

	// test unlock sequence (normal)
	{
		// set without-reply
		uint32_t opt = p_mgr->getExternalIf()->getRequestOption ();
		opt |= REQUEST_OPTION__WITHOUT_REPLY;
		p_mgr->getExternalIf()->setRequestOption (opt);

		p_mod_a_extern->reqTestLock (false); // "checked."
		p_mod_a_extern->reqTestLockIntr ();  // "intr."
		p_mod_a_extern->reqTestLockIntr ();  // "intr."

		// reset without-reply
		opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
		p_mgr->getExternalIf()->setRequestOption (opt);

		sleep(3);
		p_mod_a_extern->reqTestLockIntr ();  // "intr."
		ST_THM_SRC_INFO* r = p_mgr->getExternalIf()-> receiveExternal();
		assert(r != NULL);
		assert (r->enRslt == EN_THM_RSLT_SUCCESS);
		std::string s = (char*)r->msg.pMsg;
		assert (s == std::string("intr.checked.intr.intr.intr.intr.checked.intr."));
	}

	// test overwrite
	{
		// set without-reply
		uint32_t opt = p_mgr->getExternalIf()->getRequestOption ();
		opt |= REQUEST_OPTION__WITHOUT_REPLY;
		p_mgr->getExternalIf()->setRequestOption (opt);

		p_mod_a_extern->reqTestOverwrite (); // "ch"
		p_mod_a_extern->reqTestOverwrite (); // "ch"
		p_mod_a_extern->reqTestOverwrite (); // "ch"

		// reset without-reply
		opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
		p_mgr->getExternalIf()->setRequestOption (opt);

		p_mod_a_extern->reqTestOverwrite ();  // "ch"
		ST_THM_SRC_INFO* r = p_mgr->getExternalIf()-> receiveExternal();
		assert(r != NULL);
		assert (r->enRslt == EN_THM_RSLT_SUCCESS);
		std::string s = (char*)r->msg.pMsg;
		assert (s == std::string("chchchchecked."));
	}

	// dump
	kill (getpid(), SIGQUIT);
	sleep(5);

	// test destroy
	{
		// set without-reply
		uint32_t opt = p_mgr->getExternalIf()->getRequestOption ();
		opt |= REQUEST_OPTION__WITHOUT_REPLY;
		p_mgr->getExternalIf()->setRequestOption (opt);

		p_mod_a_extern->reqTestDestroy ();

		// reset without-reply
		opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
		p_mgr->getExternalIf()->setRequestOption (opt);
	}


	p_mgr->wait ();


	p_mgr->getExternalIf()->destroyExternalCp();
	p_mgr->teardown();

	delete p_mod_a_extern;
	delete p_mod_b_extern;
	delete p_mod_c_extern;

	for (const auto &th : threads)
		delete th;

	exit (EXIT_SUCCESS);
}
