#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <signal.h>

#include <vector>

#include "ThreadMgrpp.h"

#include "ModuleA.h"
#include "ModuleA_extern.h"
#include "ModuleB.h"
#include "ModuleB_extern.h"
#include "ModuleC.h"
#include "ModuleC_extern.h"

#include "modules.h"


using namespace ThreadManager;

int main (void)
{
	initLogStdout();
	initSyslog();

	THM_LOG_D ("test THM_LOG_D");
	THM_LOG_I ("test THM_LOG_I");
	THM_LOG_W ("test THM_LOG_W");
	THM_LOG_E ("test THM_LOG_E");
	THM_PERROR ("test THM_PERROR");


	CThreadMgr *p_mgr = CThreadMgr::getInstance();

	CModuleA *p_moduleA = new CModuleA("ModuleA", 10);
	CModuleB *p_moduleB = new CModuleB("ModuleB", 10);
	CModuleC *p_moduleC = new CModuleC("ModuleC", 10);
	std::vector<CThreadMgrBase*> threads;
	threads.push_back(p_moduleA);
	threads.push_back(p_moduleB);
	threads.push_back(p_moduleC);

	if (!p_mgr->setup (threads)) {
		exit (EXIT_FAILURE);
	}

	CModuleA_extern *p_mod_a_extern = new CModuleA_extern (p_mgr->getExternalIf());
	CModuleB_extern *p_mod_b_extern = new CModuleB_extern (p_mgr->getExternalIf());
	CModuleC_extern *p_mod_c_extern = new CModuleC_extern (p_mgr->getExternalIf());


	p_mgr->getExternalIf()->createExternalCp();

	ST_THM_SRC_INFO* r = NULL;
	p_mod_a_extern-> reqStartUp ();
	r = p_mgr->getExternalIf()-> receiveExternal();
	if (r) {
		THM_LOG_I ("r [%d][%s]", r->enRslt, r->msg.pMsg);
	} else {
		THM_LOG_E ("r null");
	}

	p_mod_b_extern-> reqStartUp ();
	r = p_mgr->getExternalIf()-> receiveExternal();
	if (r) {
		THM_LOG_I ("r [%d][%s]", r->enRslt, r->msg.pMsg);
	} else {
		THM_LOG_E ("r null");
	}

	p_mod_c_extern-> reqStartUp ();
	r = p_mgr->getExternalIf()-> receiveExternal();
	if (r) {
		THM_LOG_I ("r [%d][%s]", r->enRslt, r->msg.pMsg);
	} else {
		THM_LOG_E ("r null");
	}


	const char *msg = "test request";
	p_mod_a_extern->reqFunc00 ((const char*)msg, strlen(msg));


	kill (getpid(), SIGQUIT);


//	p_mgr->wait ();
sleep(3);

	p_mgr->teardown();

	putsBackTrace();

	finalizSyslog();
	finalizLog();

	for (const auto &th : threads)
		delete th;

	exit (EXIT_SUCCESS);
}
