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

#include "ThreadMgrIf.h"
#include "ThreadMgrpp.h"

#include "module_a.h"
#include "module_a_extern.h"
#include "module_b.h"
#include "module_b_extern.h"
#include "module_c.h"
#include "module_c_extern.h"

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

	auto mod_a = std::make_shared <module_a>(std::move("module_a"), 10);
	auto mod_b = std::make_shared <module_b>(std::move("module_b"), 10);
	auto mod_c = std::make_shared <module_c>(std::move("module_c"), 10);
	std::vector<std::shared_ptr<threadmgr::CThreadMgrBase>> threads;
	threads.push_back(mod_a);
	threads.push_back(mod_b);
	threads.push_back(mod_c);

	if (!p_mgr->setup (threads)) {
		THM_LOG_E ("setup failure.");
		exit (EXIT_FAILURE);
	}


	auto mod_a_extern = std::make_shared <module_a_extern> (p_mgr->get_external_if());
	auto mod_b_extern = std::make_shared <module_b_extern> (p_mgr->get_external_if());
	auto mod_c_extern = std::make_shared <module_c_extern> (p_mgr->get_external_if());


	p_mgr->get_external_if()->create_external_cp();

	{
		mod_a_extern-> req_startup ();
		threadmgr::CSource& r = p_mgr->get_external_if()-> receive_external();
		assert (r.get_result() == threadmgr::result::success);
		std::string s = reinterpret_cast<char*>(r.get_message().data());
		assert (s == std::string("module_a startup end."));
	}
	{
		mod_b_extern-> req_startup ();
		threadmgr::CSource& r = p_mgr->get_external_if()-> receive_external();
		assert (r.get_result() == threadmgr::result::success);
		std::string s = reinterpret_cast<char*>(r.get_message().data());
		assert (s == std::string("module_b startup end."));
	}
	{
		mod_c_extern-> req_startup ();
		threadmgr::CSource& r = p_mgr->get_external_if()-> receive_external();
		assert (r.get_result() == threadmgr::result::success);
		std::string s = reinterpret_cast<char*>(r.get_message().data());
		assert (s == std::string("module_c startup end."));
	}

	// test request-reply, req-timeout, seq-timeout, notify
	{
		mod_a_extern->req_test_reqrep ();
		threadmgr::CSource& r = p_mgr->get_external_if()-> receive_external();
		assert (r.get_result() == threadmgr::result::success);
	}

	// test lock sequence
	{
		// set without-reply
		threadmgr::request_option::type opt = p_mgr->get_external_if()->get_request_option ();
		opt |= threadmgr::request_option::without_reply;
		p_mgr->get_external_if()->set_request_option (opt);

		mod_a_extern->req_test_lock_intr ();  // "intr."
		mod_a_extern->req_test_lock (true);  // "checked."
		mod_a_extern->req_test_lock_intr ();  // "intr."

		// reset without-reply
		opt &= ~threadmgr::request_option::without_reply;
		p_mgr->get_external_if()->set_request_option (opt);

		mod_a_extern->req_test_lock_intr ();  // "intr."
		threadmgr::CSource& r = p_mgr->get_external_if()-> receive_external();
		assert (r.get_result() == threadmgr::result::success);
		std::string s = reinterpret_cast<char*>(r.get_message().data());
		assert (s == std::string("intr.checked.intr.intr."));
	}

	// test unlock sequence (normal)
	{
		// set without-reply
		threadmgr::request_option::type opt = p_mgr->get_external_if()->get_request_option ();
		opt |= threadmgr::request_option::without_reply;
		p_mgr->get_external_if()->set_request_option (opt);

		mod_a_extern->req_test_lock (false); // "checked."
		mod_a_extern->req_test_lock_intr ();  // "intr."
		mod_a_extern->req_test_lock_intr ();  // "intr."

		// reset without-reply
		opt &= ~threadmgr::request_option::without_reply;
		p_mgr->get_external_if()->set_request_option (opt);

		sleep(3);
		mod_a_extern->req_test_lock_intr ();  // "intr."
		threadmgr::CSource& r = p_mgr->get_external_if()-> receive_external();
		assert (r.get_result() == threadmgr::result::success);
		std::string s = reinterpret_cast<char*>(r.get_message().data());
		assert (s == std::string("intr.checked.intr.intr.intr.intr.checked.intr."));
	}

	// test overwrite
	{
		// set without-reply
		threadmgr::request_option::type opt = p_mgr->get_external_if()->get_request_option ();
		opt |= threadmgr::request_option::without_reply;
		p_mgr->get_external_if()->set_request_option (opt);

		mod_a_extern->req_test_overwrite (); // "ch"
		mod_a_extern->req_test_overwrite (); // "ch"
		mod_a_extern->req_test_overwrite (); // "ch"

		// reset without-reply
		opt &= ~threadmgr::request_option::without_reply;
		p_mgr->get_external_if()->set_request_option (opt);

		mod_a_extern->req_test_overwrite ();  // "ch"
		threadmgr::CSource& r = p_mgr->get_external_if()-> receive_external();
		assert (r.get_result() == threadmgr::result::success);
		std::string s = reinterpret_cast<char*>(r.get_message().data());
		assert (s == std::string("chchchchecked."));
	}

	// dump
	kill (getpid(), SIGQUIT);
	sleep(5);

	// test destroy
	{
		// set without-reply
		threadmgr::request_option::type opt = p_mgr->get_external_if()->get_request_option ();
		opt |= threadmgr::request_option::without_reply;
		p_mgr->get_external_if()->set_request_option (opt);

		mod_a_extern->req_test_destroy ();

		// reset without-reply
		opt &= ~threadmgr::request_option::without_reply;
		p_mgr->get_external_if()->set_request_option (opt);
	}


	p_mgr->wait ();


	p_mgr->get_external_if()->destroy_external_cp();
	p_mgr->teardown();

	mod_a = nullptr;
	mod_b = nullptr;
	mod_c = nullptr;

	exit (EXIT_SUCCESS);
}
