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


threadmgr_external_if_t *gp_if;


int main( void )
{
	set_log_fileptr(stdout);

	THM_LOG_E ("start");

	threadmgr_src_info_t *res = NULL;

	if (!(gp_if = setup_threadmgr(gst_reg_threadmgr_tbl, (uint32_t)EN_THREAD_MAX))) {
		THM_LOG_E ("setup_thread_mgr() is failure.");
		exit (EXIT_FAILURE);
	}

	gp_if->create_external_cp();


	req_startup_thread_a(NULL);
	res = gp_if-> receive_external();
	if (res) {
		THM_LOG_I ("dddddddddddddddddd res [%d][%s]", res->result, res->msg.msg);
	} else {
		THM_LOG_E ("dddddddddddddddddd res null");
	}

	req_startup_thread_b(NULL);
	res = gp_if-> receive_external();
	if (res) {
		THM_LOG_I ("dddddddddddddddddd res [%d][%s]\n", res->result, res->msg.msg);
	} else {
		THM_LOG_E ("dddddddddddddddddd res null");
	}

	req_startup_thread_c(NULL);
	res = gp_if-> receive_external();
	if (res) {
		THM_LOG_I ("dddddddddddddddddd res [%d][%s]\n", res->result, res->msg.msg);
	} else {
		THM_LOG_E ("dddddddddddddddddd res null");
	}


	char sz_in[1024];
	memset (sz_in, 0x00, sizeof(sz_in));

	while (1) {
		THM_LOG_I ("wait stdin...");

		if (read (STDIN_FILENO, (uint8_t*)sz_in, sizeof(sz_in) -1) < 0) {
			THM_PERROR ("read()");
			continue;
		}

		delete_LF (sz_in);

		if (
			(strlen(sz_in) == strlen("a0")) &&
			(!strncmp(sz_in, "a0", strlen("a0")))
		) {
			req_func00_thread_a (sz_in, NULL);

		} else if (
			(strlen(sz_in) == strlen("a2")) &&
			(!strncmp(sz_in, "a2", strlen("a2")))
		) {
			req_func02_thread_a (sz_in, NULL);

		} else if (
			(strlen(sz_in) == strlen("a3")) &&
			(!strncmp(sz_in, "a3", strlen("a3")))
		) {
			req_func03_thread_a (NULL);

		} else if (
			(strlen(sz_in) == strlen("b2")) &&
			(!strncmp(sz_in, "b2", strlen("b2")))
		) {
			req_func02_thread_b (NULL);

		} else if (
			(strlen(sz_in) == strlen("quit")) &&
			(!strncmp(sz_in, "quit", strlen("quit")))
		) {
			break;
		}

		memset (sz_in, 0x00, sizeof(sz_in));

	}


	gp_if->destroy_external_cp();

	teardown_threadmgr();

	exit (EXIT_SUCCESS);
}
