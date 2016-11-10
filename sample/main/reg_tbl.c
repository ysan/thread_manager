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


const ST_THM_REG_TBL gstRegThreadMgrTbl[ EN_THREAD_MAX ] = {
	{
		"thread_A",
		NULL,
		5,
		gpSeqsThreadA,
		EN_A_SEQ_NUM,
		NULL,
		recvNotifyThreadA,
	},
	{
		"thread_B",
		NULL,
		10,
		gpSeqsThreadB,
		EN_B_SEQ_NUM,
		NULL,
		recvNotifyThreadB,
	},
	{
		"thread_C",
		NULL,
		15,
		gpSeqsThreadC,
		EN_C_SEQ_NUM,
		NULL,
		recvNotifyThreadC,
	},
};
