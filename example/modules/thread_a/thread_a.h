#ifndef _THREAD_A_H_
#define _THREAD_A_H_


/*
 * Constant define
 */
enum {
	EN_A_STARTUP = 0,
	EN_A_FUNC_00,
	EN_A_FUNC_01,
	EN_A_FUNC_02,
	EN_A_SEQ_NUM,
};

/*
 * Type define
 */


/*
 * External
 */
extern const P_THM_SEQ gpSeqsThreadA[];
extern void recvNotifyThreadA (ST_THM_IF *pIf);
extern void reqStartupThreadA (uint32_t *pnReqId);
extern void reqFunc00ThreadA (char *pszMsg, uint32_t *pnReqId);
extern void reqFunc01ThreadA (uint32_t *pnReqId);
extern void reqFunc02ThreadA (char *pszMsg, uint32_t *pnReqId);

#endif