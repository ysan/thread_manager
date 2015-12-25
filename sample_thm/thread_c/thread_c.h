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
	EN_C_FUNC_NUM,
};

/*
 * Type define
 */


/*
 * External
 */
extern const P_THM_FUNC gpThreadCfuncs[];
extern void recvNotifyThreadC(ST_THM_IF *pIf);
extern void reqAsyncThreadCstartup (void);
extern void reqAsyncThreadCregNotify(void);
extern void reqAsyncThreadCunRegNotify (uint8_t nClientId);
extern void reqAsyncThreadCfunc00 (void);

#endif
