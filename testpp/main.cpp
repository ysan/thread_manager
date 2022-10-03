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
#include <cstdlib>

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


	threadmgr::CThreadMgr *p_mgr = threadmgr::CThreadMgr::get_instance();

	auto module_a = std::make_shared <CModuleA>("ModuleA", 10);
	auto module_b = std::make_shared <CModuleB>("ModuleB", 10);
	auto module_c = std::make_shared <CModuleC>("ModuleC", 10);
	std::vector<std::shared_ptr<threadmgr::CThreadMgrBase>> threads;
	threads.push_back(module_a);
	threads.push_back(module_b);
	threads.push_back(module_c);

	if (!p_mgr->setup (threads)) {
		THM_LOG_E ("setup failure.");
		exit (EXIT_FAILURE);
	}


	auto mod_a_extern = std::make_shared <CModuleA_extern> (p_mgr->get_external_if());
	auto mod_b_extern = std::make_shared <CModuleB_extern> (p_mgr->get_external_if());
	auto mod_c_extern = std::make_shared <CModuleC_extern> (p_mgr->get_external_if());


	p_mgr->get_external_if()->create_external_cp();

	{
		mod_a_extern-> req_startup ();
		ST_THM_SRC_INFO* r = p_mgr->get_external_if()-> receive_external();
		assert(r != NULL);
		assert (r->enRslt == EN_THM_RSLT_SUCCESS);
		std::string s = (char*)r->msg.pMsg;
		assert (s == std::string("ModuleA startup end."));
	}
	{
		mod_b_extern-> req_startup ();
		ST_THM_SRC_INFO* r = p_mgr->get_external_if()-> receive_external();
		assert(r != NULL);
		assert (r->enRslt == EN_THM_RSLT_SUCCESS);
		std::string s = (char*)r->msg.pMsg;
		assert (s == std::string("ModuleB startup end."));
	}
	{
		mod_c_extern-> req_startup ();
		ST_THM_SRC_INFO* r = p_mgr->get_external_if()-> receive_external();
		assert(r != NULL);
		assert (r->enRslt == EN_THM_RSLT_SUCCESS);
		std::string s = (char*)r->msg.pMsg;
		assert (s == std::string("ModuleC startup end."));
	}

	// test request-reply, req-timeout, seq-timeout, notify
	{
		mod_a_extern->req_test_reqrep ();
		ST_THM_SRC_INFO* r = p_mgr->get_external_if()-> receive_external();
		assert(r != NULL);
		assert (r->enRslt == EN_THM_RSLT_SUCCESS);
	}

	// test lock sequence
	{
		// set without-reply
		uint32_t opt = p_mgr->get_external_if()->get_request_option ();
		opt |= REQUEST_OPTION__WITHOUT_REPLY;
		p_mgr->get_external_if()->set_request_option (opt);

		mod_a_extern->req_test_lock_intr ();  // "intr."
		mod_a_extern->req_test_lock (true);  // "checked."
		mod_a_extern->req_test_lock_intr ();  // "intr."

		// reset without-reply
		opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
		p_mgr->get_external_if()->set_request_option (opt);

		mod_a_extern->req_test_lock_intr ();  // "intr."
		ST_THM_SRC_INFO* r = p_mgr->get_external_if()-> receive_external();
		assert(r != NULL);
		assert (r->enRslt == EN_THM_RSLT_SUCCESS);
		std::string s = (char*)r->msg.pMsg;
		assert (s == std::string("intr.checked.intr.intr."));
	}

	// test unlock sequence (normal)
	{
		// set without-reply
		uint32_t opt = p_mgr->get_external_if()->get_request_option ();
		opt |= REQUEST_OPTION__WITHOUT_REPLY;
		p_mgr->get_external_if()->set_request_option (opt);

		mod_a_extern->req_test_lock (false); // "checked."
		mod_a_extern->req_test_lock_intr ();  // "intr."
		mod_a_extern->req_test_lock_intr ();  // "intr."

		// reset without-reply
		opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
		p_mgr->get_external_if()->set_request_option (opt);

		sleep(3);
		mod_a_extern->req_test_lock_intr ();  // "intr."
		ST_THM_SRC_INFO* r = p_mgr->get_external_if()-> receive_external();
		assert(r != NULL);
		assert (r->enRslt == EN_THM_RSLT_SUCCESS);
		std::string s = (char*)r->msg.pMsg;
		assert (s == std::string("intr.checked.intr.intr.intr.intr.checked.intr."));
	}

	// test overwrite
	{
		// set without-reply
		uint32_t opt = p_mgr->get_external_if()->get_request_option ();
		opt |= REQUEST_OPTION__WITHOUT_REPLY;
		p_mgr->get_external_if()->set_request_option (opt);

		mod_a_extern->req_test_overwrite (); // "ch"
		mod_a_extern->req_test_overwrite (); // "ch"
		mod_a_extern->req_test_overwrite (); // "ch"

		// reset without-reply
		opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
		p_mgr->get_external_if()->set_request_option (opt);

		mod_a_extern->req_test_overwrite ();  // "ch"
		ST_THM_SRC_INFO* r = p_mgr->get_external_if()-> receive_external();
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
		uint32_t opt = p_mgr->get_external_if()->get_request_option ();
		opt |= REQUEST_OPTION__WITHOUT_REPLY;
		p_mgr->get_external_if()->set_request_option (opt);

		mod_a_extern->req_test_destroy ();

		// reset without-reply
		opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
		p_mgr->get_external_if()->set_request_option (opt);
	}


	p_mgr->wait ();


	p_mgr->get_external_if()->destroy_external_cp();
	p_mgr->teardown();


	exit (EXIT_SUCCESS);
}
