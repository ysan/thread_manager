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
		gpThreadAfuncs,
		EN_A_FUNC_NUM,
		NULL,
		recvNotifyThreadA,
	},
	{
		"thread_B",
		NULL,
		10,
		gpThreadBfuncs,
		EN_B_FUNC_NUM,
		NULL,
		recvNotifyThreadB,
	},
	{
		"thread_C",
		NULL,
		15,
		gpThreadCfuncs,
		EN_C_FUNC_NUM,
		NULL,
		recvNotifyThreadC,
	},
};
