#ifndef _THREADMGR_IF_H_
#define _THREADMGR_IF_H_

#include <stdbool.h>

/*
 * Constant define
 */


/*
 * Type define
 */
#ifndef uint8_t
typedef unsigned char uint8_t;
#endif
#ifndef uint16_t
typedef unsigned short uint16_t;
#endif
#ifndef uint32_t
typedef unsigned int uint32_t;
#endif
#ifndef uint64_t
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
//	EN_THM_ACT_TIMEOUT,
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
typedef bool (*P_REQUEST_SYNC) (uint8_t nThreadIdx, uint8_t nSeqIdx, uint8_t *pszMsg);
typedef bool (*P_REQUEST_ASYNC) (uint8_t nThreadIdx, uint8_t nSeqIdx, uint8_t *pszMsg);
typedef bool (*P_CREATE_EXTERNAL_CP) (void);
typedef void (*P_DESTROY_EXTERNAL_CP) (void);
typedef ST_THM_SRC_INFO* (*P_RECEIVE_EXTERNAL) (void);
typedef void (*P_FINALIZE) (void);

typedef struct threadmgr_external_if {
	P_REQUEST_SYNC pRequestSync;
	P_REQUEST_ASYNC pRequestAsync;
	P_CREATE_EXTERNAL_CP pCreateExternalCp;
	P_DESTROY_EXTERNAL_CP pDestroyExternalCp;
	P_RECEIVE_EXTERNAL pReceiveExternal;
	P_FINALIZE pFinalize;
} ST_THM_EXTERNAL_IF;



/*--- threadmgr_if ---*/
typedef bool (*P_REPLY) (EN_THM_RSLT enRslt, uint8_t *pszMsg);
typedef bool (*P_REG_NOTIFY) (uint8_t *pnClientId);
typedef bool (*P_UNREG_NOTIFY) (uint8_t nClientId);
typedef bool (*P_NOTIFY) (uint8_t nClientId, uint8_t *pszMsg);
typedef void (*P_SET_SECTID) (uint8_t nSectId, EN_THM_ACT enAct);
typedef uint8_t (*P_GET_SECTID) (void);
typedef void (*P_SET_TIMEOUT) (uint32_t nTimeoutMsec);
typedef void (*P_CLEAR_TIMEOUT) (void);

typedef struct threadmgr_if {
	ST_THM_SRC_INFO *pstSrcInfo;

	P_REPLY pReply;

	P_REG_NOTIFY pRegNotify;
	P_UNREG_NOTIFY pUnRegNotify;
	P_NOTIFY pNotify;

	P_SET_SECTID pSetSectId;
	P_GET_SECTID pGetSectId;

	P_SET_TIMEOUT pSetTimeout;
	P_CLEAR_TIMEOUT pClearTimeout;

} ST_THM_IF;


/*--- threadmgr_reg_tbl ---*/
typedef void (*P_SETUP) (void);
typedef void (*P_THM_SEQ) (ST_THM_IF *pIf);
typedef void (*P_RECV_ASYNC_REPLY) (ST_THM_IF *pIf);
typedef void (*P_RECV_NOTIFY) (ST_THM_IF *pIf);

typedef struct threadmgr_reg_tbl {
	const char *pszName;
	const P_SETUP pSetup;
	uint8_t nQueNum;
	const P_THM_SEQ *pSeqArray; // double pointer
	uint8_t nSeqNum;
	const P_RECV_ASYNC_REPLY pRecvAsyncReply;
	const P_RECV_NOTIFY pRecvNotify;
} ST_THM_REG_TBL;


/*
 * External
 */
ST_THM_EXTERNAL_IF *setupThreadMgr (const ST_THM_REG_TBL *pTbl, uint8_t nTblMax);


#endif
