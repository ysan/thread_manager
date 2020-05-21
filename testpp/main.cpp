#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <signal.h>

#include "ThreadMgrpp.h"

#include "ModuleA.h"
#include "ModuleA_extern.h"
#include "ModuleB.h"
#include "ModuleB_extern.h"
#include "ModuleC.h"
#include "ModuleC_extern.h"

#include "mgr_reg_tbl.h"


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

	CThreadMgr *pMgr = CThreadMgr::getInstance();

	if (!pMgr->setup (gp_threads, EN_MODULE_NUM)) {
		exit (EXIT_FAILURE);
	}

	CModuleA_extern *p_mod_a_extern = new CModuleA_extern (pMgr->getExternalIf());
	CModuleB_extern *p_mod_b_extern = new CModuleB_extern (pMgr->getExternalIf());
	CModuleC_extern *p_mod_c_extern = new CModuleC_extern (pMgr->getExternalIf());


	pMgr->getExternalIf()->createExternalCp();


	p_mod_a_extern-> reqStartUp ();
	ST_THM_SRC_INFO* r = pMgr->getExternalIf()-> receiveExternal();
	if (r) {
		THM_LOG_I ("dddddddddddddddddd r [%d][%s]", r->enRslt, r->msg.pMsg);
	} else {
		THM_LOG_E ("dddddddddddddddddd r null");
	}

	p_mod_b_extern-> reqStartUp ();
	r = pMgr->getExternalIf()-> receiveExternal();
	if (r) {
		THM_LOG_I ("dddddddddddddddddd r [%d][%s]", r->enRslt, r->msg.pMsg);
	} else {
		THM_LOG_E ("dddddddddddddddddd r null");
	}

	p_mod_c_extern-> reqStartUp ();
	r = pMgr->getExternalIf()-> receiveExternal();
	if (r) {
		THM_LOG_I ("dddddddddddddddddd r [%d][%s]", r->enRslt, r->msg.pMsg);
	} else {
		THM_LOG_E ("dddddddddddddddddd r null");
	}


	const char *msg = "test request";
	p_mod_a_extern->reqFunc00 ((const char*)msg, strlen(msg));


	kill (getpid(), SIGQUIT);


//	pMgr->wait ();
sleep(3);

	pMgr->teardown();

	putsBackTrace();

	finalizSyslog();
	finalizLog();

	exit (EXIT_SUCCESS);
}
