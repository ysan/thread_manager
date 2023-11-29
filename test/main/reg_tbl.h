#ifndef _REG_THREADMGR_TBL_H_
#define _REG_THREADMGR_TBL_H_

#include "threadmgr_if.h"

/*
 * Constant define
 */
enum {
	EN_THREAD_A = 0,
	EN_THREAD_B,
	EN_THREAD_C,
	EN_THREAD_MAX
};

/*
 * External
 */
extern const threadmgr_reg_tbl_t gst_reg_threadmgr_tbl [EN_THREAD_MAX];


#endif
