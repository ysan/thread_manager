#ifndef _THREAD_C_H_
#define _THREAD_C_H_


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
extern const ST_THM_SEQ gstSeqsThreadC [];
extern void recvNotifyThreadC(ST_THM_IF *pIf);
extern void reqStartupThreadC (uint32_t *pnReqId);
extern void reqRegNotifyThreadC(uint32_t *pnReqId);
extern void reqUnRegNotifyThreadC (uint8_t nClientId, uint32_t *pnReqId);
extern void reqFunc00ThreadC (uint32_t *pnReqId);

#endif
