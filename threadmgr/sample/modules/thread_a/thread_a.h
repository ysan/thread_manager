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
	EN_A_FUNC_NUM,
};

/*
 * Type define
 */


/*
 * External
 */
extern const P_THM_FUNC gpThreadAfuncs[];
extern void recvNotifyThreadA (ST_THM_IF *pIf);
extern void reqAsyncThreadAstartup (void);
extern void reqAsyncThreadAfunc00 (char *pszMsg);
extern void reqAsyncThreadAfunc01 (void);
extern void reqAsyncThreadAfunc02 (char *pszMsg);

#endif
