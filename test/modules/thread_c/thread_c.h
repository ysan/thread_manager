#ifndef _THREAD_C_H_
#define _THREAD_C_H_

#include "threadmgr_if.h"

/*
 * Constant define
 */
enum {
	EN_C_STARTUP = 0,
	EN_C_REG_NOTIFY,
	EN_C_UNREG_NOTIFY,
	EN_C_CYCLE_FUNC,
	EN_C_FUNC_00,
	EN_C_SEQ_NUM,
};

/*
 * Type define
 */


/*
 * External
 */
extern const threadmgr_seq_t gst_seqs_thread_c [];
extern void recv_notify_thread_c(threadmgr_if_t *p_if);
extern void req_startup_thread_c (uint32_t *p_out_req_id);
extern void req_reg_notify_thread_c(uint32_t *p_out_req_id);
extern void req_un_reg_notify_thread_c (uint8_t n_client_id, uint32_t *p_out_req_id);
extern void req_func00_thread_c (uint32_t *p_out_req_id);

#endif
