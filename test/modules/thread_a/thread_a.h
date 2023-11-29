#ifndef _THREAD_A_H_
#define _THREAD_A_H_

#include "threadmgr_if.h"

/*
 * Constant define
 */
enum {
	EN_A_STARTUP = 0,
	EN_A_FUNC_00,
	EN_A_FUNC_01,
	EN_A_FUNC_02,
	EN_A_FUNC_03,
	EN_A_SEQ_NUM,
};

/*
 * Type define
 */


/*
 * External
 */
extern const threadmgr_seq_t gst_seqs_thread_a[];
extern void recv_notify_thread_a (threadmgr_if_t *p_if);
extern void req_startup_thread_a (uint32_t *p_out_req_id);
extern void req_func00_thread_a (char *psz_msg, uint32_t *p_out_req_id);
extern void req_func01_thread_a (uint32_t *p_out_req_id);
extern void req_func02_thread_a (char *psz_msg, uint32_t *p_out_req_id);
extern void req_func03_thread_a (uint32_t *p_out_req_id);

#endif
