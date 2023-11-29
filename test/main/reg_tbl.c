#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "threadmgr_if.h"
#include "reg_tbl.h"

#include "thread_a.h"
#include "thread_b.h"
#include "thread_c.h"


const threadmgr_reg_tbl_t gst_reg_threadmgr_tbl [EN_THREAD_MAX] = {
	{
		"thread_A",
		NULL,
		NULL,
		5,
		gst_seqs_thread_a,
		EN_A_SEQ_NUM,
		recv_notify_thread_a,
	},
	{
		"thread_B",
		NULL,
		NULL,
		10,
		gst_seqs_thread_b,
		EN_B_SEQ_NUM,
		recv_notify_thread_b,
	},
	{
		"thread_C",
		NULL,
		NULL,
		15,
		gst_seqs_thread_c,
		EN_C_SEQ_NUM,
		recv_notify_thread_c,
	},
};
