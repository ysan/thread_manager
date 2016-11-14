#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "threadmgr_if.h"
#include "threadmgr_util.h"

#include "reg_tbl.h"

#include "thread_a.h"
#include "thread_b.h"
#include "thread_c.h"


ST_THM_EXTERNAL_IF *gpIf;


int main( void )
{
	ST_THM_SRC_INFO *res = NULL;

	if (!(gpIf = setupThreadMgr(gstRegThreadMgrTbl, (uint32_t)EN_THREAD_MAX))) {
		THM_LOG_E ("setupThreadMgr() is failure.");
		exit (EXIT_FAILURE);
	}

	gpIf->pCreateExternalCp();


	reqStartupThreadA(NULL);
	res = gpIf-> pReceiveExternal();
	if (res) {
		THM_LOG_I ("dddddddddddddddddd res [%d][%s]", res->enRslt, res->pszMsg);
	} else {
		THM_LOG_E ("dddddddddddddddddd res null");
	}

	reqStartupThreadB(NULL);
	res = gpIf-> pReceiveExternal();
	if (res) {
		THM_LOG_I ("dddddddddddddddddd res [%d][%s]\n", res->enRslt, res->pszMsg);
	} else {
		THM_LOG_E ("dddddddddddddddddd res null");
	}

	reqStartupThreadC(NULL);
	res = gpIf-> pReceiveExternal();
	if (res) {
		THM_LOG_I ("dddddddddddddddddd res [%d][%s]\n", res->enRslt, res->pszMsg);
	} else {
		THM_LOG_E ("dddddddddddddddddd res null");
	}


	char szIn[1024];
	memset (szIn, 0x00, sizeof(szIn));

	while (1) {
		THM_LOG_I ("wait stdin...");

		if (read (STDIN_FILENO, (uint8_t*)szIn, sizeof(szIn)) < 0) {
			THM_PERROR ("read()");
			continue;
		}

		deleteLF (szIn);

		if (
			(strlen(szIn) == strlen("a0")) &&
			(!strncmp(szIn, "a0", strlen("a0")))
		) {
			reqFunc00ThreadA (szIn, NULL);

		} else if (
			(strlen(szIn) == strlen("a2")) &&
			(!strncmp(szIn, "a2", strlen("a2")))
		) {
			reqFunc02ThreadA (szIn, NULL);

		} else if (
			(strlen(szIn) == strlen("b2")) &&
			(!strncmp(szIn, "b2", strlen("b2")))
		) {
			reqFunc02ThreadB (NULL);
		}

		memset (szIn, 0x00, sizeof(szIn));

	}


	gpIf->pDestroyExternalCp();


	exit (EXIT_SUCCESS);
}
