#ifndef _THREADMGR_IF_H_
#define _THREADMGR_IF_H_

#include <stdbool.h>

/*
 * Constant define
 */


/*
 * Type define
 */
#if !defined (_NO_TYPEDEF_uint8_t)
typedef unsigned char uint8_t;
#endif

#if !defined (_NO_TYPEDEF_uint16_t)
typedef unsigned short uint16_t;
#endif

#if !defined (_NO_TYPEDEF_uint32_t)
typedef unsigned int uint32_t;
#endif

#if !defined (_NO_TYPEDEF_uint64_t)
typedef unsigned long int uint64_t;
#endif

typedef enum {
	EN_THM_RSLT_IGNORE = 0,
	EN_THM_RSLT_SUCCESS,
	EN_THM_RSLT_ERROR,
	EN_THM_RSLT_REQ_TIMEOUT,
	EN_THM_RSLT_SEQ_TIMEOUT,
	EN_THM_RSLT_MAX,
} EN_THM_RSLT;

typedef enum {
	EN_THM_ACT_INIT = 0,
	EN_THM_ACT_CONTINUE,
	EN_THM_ACT_WAIT,
	EN_THM_ACT_DONE,
	EN_THM_ACT_MAX,
} EN_THM_ACT;


typedef struct threadmgr_src_info {
	uint32_t nReqId;
	EN_THM_RSLT enRslt;
	uint8_t nClientId;
	uint8_t *pszMsg;
} ST_THM_SRC_INFO;


/*--- threadmgr_external_if ---*/
typedef bool (*PFN_REQUEST_SYNC) (uint8_t nThreadIdx, uint8_t nSeqIdx, uint8_t *pszMsg);
typedef bool (*PFN_REQUEST_ASYNC) (uint8_t nThreadIdx, uint8_t nSeqIdx, uint8_t *pszMsg, uint32_t *pnReqId);
typedef bool (*PFN_CREATE_EXTERNAL_CP) (void);
typedef void (*PFN_DESTROY_EXTERNAL_CP) (void);
typedef ST_THM_SRC_INFO* (*PFN_RECEIVE_EXTERNAL) (void);
typedef void (*PFN_FINALIZE) (void);

typedef struct threadmgr_external_if {
	PFN_REQUEST_SYNC pfnRequestSync;
	PFN_REQUEST_ASYNC pfnRequestAsync;
	PFN_CREATE_EXTERNAL_CP pfnCreateExternalCp;
	PFN_DESTROY_EXTERNAL_CP pfnDestroyExternalCp;
	PFN_RECEIVE_EXTERNAL pfnReceiveExternal;
	PFN_FINALIZE pFinalize;
} ST_THM_EXTERNAL_IF;



/*--- threadmgr_if ---*/
typedef bool (*PFN_REPLY) (EN_THM_RSLT enRslt, uint8_t *pszMsg);
typedef bool (*PFN_REG_NOTIFY) (uint8_t *pnClientId);
typedef bool (*PFN_UNREG_NOTIFY) (uint8_t nClientId);
typedef bool (*PFN_NOTIFY) (uint8_t nClientId, uint8_t *pszMsg);
typedef void (*PFN_SET_SECTID) (uint8_t nSectId, EN_THM_ACT enAct);
typedef uint8_t (*PFN_GET_SECTID) (void);
typedef void (*PFN_SET_TIMEOUT) (uint32_t nTimeoutMsec);
typedef void (*PFN_CLEAR_TIMEOUT) (void);

typedef struct threadmgr_if {
	ST_THM_SRC_INFO *pstSrcInfo;

	PFN_REPLY pfnReply;

	PFN_REG_NOTIFY pfnRegNotify;
	PFN_UNREG_NOTIFY pfnUnRegNotify;
	PFN_NOTIFY pfnNotify;

	PFN_SET_SECTID pfnSetSectId;
	PFN_GET_SECTID pfnGetSectId;

	PFN_SET_TIMEOUT pfnSetTimeout;
	PFN_CLEAR_TIMEOUT pfnClearTimeout;

} ST_THM_IF;


/*--- threadmgr_reg_tbl ---*/
typedef void (*PCB_SETUP) (void);
typedef void (*PCB_THM_SEQ) (ST_THM_IF *pIf);
typedef void (*PCB_RECV_ASYNC_REPLY) (ST_THM_IF *pIf);
typedef void (*PCB_RECV_NOTIFY) (ST_THM_IF *pIf);

typedef struct threadmgr_reg_tbl {
	const char *pszName;
	const PCB_SETUP pcbSetup;
	uint8_t nQueNum;
	const PCB_THM_SEQ *pcbSeqArray; // double pointer
	uint8_t nSeqNum;
	const PCB_RECV_ASYNC_REPLY pcbRecvAsyncReply;
	const PCB_RECV_NOTIFY pcbRecvNotify;
} ST_THM_REG_TBL;


/*
 * External
 */
ST_THM_EXTERNAL_IF *setupThreadMgr (const ST_THM_REG_TBL *pTbl, uint8_t nTblMax);


#endif
