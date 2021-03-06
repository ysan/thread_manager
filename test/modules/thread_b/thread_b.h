#ifndef _THREAD_B_H_
#define _THREAD_B_H_


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
extern const ST_THM_SEQ gstSeqsThreadB [];
extern void recvNotifyThreadB (ST_THM_IF *pIf);
extern void reqStartupThreadB (uint32_t *pOutReqId);
extern void reqFunc00ThreadB (uint32_t *pOutReqId);
extern void func01ThreadB (void);
extern void reqFunc02ThreadB (uint32_t *pOutReqId);

#endif
