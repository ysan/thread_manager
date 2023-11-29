#ifndef _THREAD_B_H_
#define _THREAD_B_H_

#include "threadmgr_if.h"

/*
 * Constant define
 */
enum {
	EN_B_STARTUP = 0,
	EN_B_FUNC_00,
	EN_B_FUNC_01,
	EN_B_FUNC_02,
	EN_B_SEQ_NUM,
};

/*
 * Type define
 */


/*
 * External
 */
extern const threadmgr_seq_t gst_seqs_thread_b [];
extern void recv_notify_thread_b (threadmgr_if_t *p_if);
extern void req_startup_thread_b (uint32_t *p_out_req_id);
extern void req_func00_thread_b (uint32_t *p_out_req_id);
extern void func01_thread_b (void);
extern void req_func02_thread_b (uint32_t *p_out_req_id);

#endif
