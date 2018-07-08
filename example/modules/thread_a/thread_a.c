#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "threadmgr_if.h"
#include "threadmgr_util.h"

#include "main.h"
#include "reg_tbl.h"

#include "thread_a.h"
#include "thread_b.h"
#include "thread_c.h"


/*
 * Prototypes
 */
void recvNotifyThreadA (ST_THM_IF *pIf); // extern
static void startup (ST_THM_IF *pIf);
static void func00 (ST_THM_IF *pIf);
static void func01 (ST_THM_IF *pIf);
static void func02 (ST_THM_IF *pIf);
void reqStartupThreadA (uint32_t *pnReqId); // extern
void reqFunc00ThreadA (char *pszMsg, uint32_t *pnReqId); // extern
void reqFunc01ThreadA (uint32_t *pnReqId); // extern
void reqFunc02ThreadA (char *pszMsg, uint32_t *pnReqId); // extern

/*
 * Variables
 */
const P_THM_SEQ gpSeqsThreadA[ EN_A_SEQ_NUM ] = {
	startup,
	func00,
	func01,
	func02,
};
static uint8_t gnClientId;



void recvNotifyThreadA (ST_THM_IF *pIf)
{
}

static void startup (ST_THM_IF *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = 0,
		SECTID_END,
	};

	nSectId = pIf->pGetSectId();
	THM_LOG_I ("nSectId %d\n", nSectId);

	pIf->pReply (EN_THM_RSLT_SUCCESS, (uint8_t*)"threadA startup end.");

	nSectId = 0;
	enAct = EN_THM_ACT_DONE;
	pIf->pSetSectId (nSectId, enAct);
}

static void func00 (ST_THM_IF *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = 0,
		SECTID_REQ_THREAD_A_FUNC01,
		SECTID_WAIT_THREAD_A_FUNC01,
		SECTID_END,
		SECTID_ERR_END,
	};

	nSectId = pIf->pGetSectId();
	THM_LOG_I ("nSectId %d\n", nSectId);

	switch (nSectId) {
	case SECTID_ENTRY:
		THM_LOG_I ("msg [%s]\n", pIf->pstSrcInfo->pszMsg);

		nSectId = SECTID_REQ_THREAD_A_FUNC01;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_THREAD_A_FUNC01:

		// 自スレのfunc01にリクエスト
		reqFunc01ThreadA(NULL);

		nSectId = SECTID_WAIT_THREAD_A_FUNC01;
		enAct = EN_THM_ACT_WAIT;
		break;

	case SECTID_WAIT_THREAD_A_FUNC01: {
		EN_THM_RSLT enRslt = pIf->pstSrcInfo->enRslt;
		THM_LOG_I ("reqFunc01ThreadA return [%d] msg:[%s]\n", enRslt, pIf->pstSrcInfo->pszMsg);

		if (enRslt == EN_THM_RSLT_SUCCESS) {
			nSectId = SECTID_END;
			enAct = EN_THM_ACT_CONTINUE;
		} else {
			nSectId = SECTID_ERR_END;
			enAct = EN_THM_ACT_CONTINUE;
		}

		} break;

	case SECTID_END:
		pIf->pReply (EN_THM_RSLT_SUCCESS, NULL);
		nSectId = 0;
		enAct = EN_THM_ACT_DONE;
		break;

	case SECTID_ERR_END:
		pIf->pReply (EN_THM_RSLT_ERROR, NULL);
		nSectId = 0;
		enAct = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}

	pIf->pSetSectId (nSectId, enAct);
}

static uint32_t gnTmpReqId;
static void func01 (ST_THM_IF *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = 0,
		SECTID_REQ_THREAD_B_FUNC00,
		SECTID_WAIT_THREAD_B_FUNC00,
		SECTID_END,
		SECTID_ERR_END,
	};

	nSectId = pIf->pGetSectId();
	THM_LOG_I ("nSectId %d\n", nSectId);

	switch (nSectId) {
	case SECTID_ENTRY:
		THM_LOG_I ("msg [%s]\n", pIf->pstSrcInfo->pszMsg);

		nSectId = SECTID_REQ_THREAD_B_FUNC00;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_THREAD_B_FUNC00:

		// スレッドBのfunc00にリクエスト
		reqFunc00ThreadB (NULL);
		reqFunc00ThreadB (NULL);
		reqFunc00ThreadB (NULL);
		reqFunc00ThreadB (NULL);
		reqFunc00ThreadB (&gnTmpReqId);
		THM_LOG_I ("reqFunc00ThreadB reqid:[%d]\n", gnTmpReqId);

		nSectId = SECTID_WAIT_THREAD_B_FUNC00;
		enAct = EN_THM_ACT_WAIT;
		break;

	case SECTID_WAIT_THREAD_B_FUNC00: {
		if (pIf->pstSrcInfo->nReqId != gnTmpReqId) {
			THM_LOG_W ("different reqid %d\n", pIf->pstSrcInfo->nReqId);
			nSectId = SECTID_WAIT_THREAD_B_FUNC00;
			enAct = EN_THM_ACT_WAIT;

		} else {
			EN_THM_RSLT enRslt = pIf->pstSrcInfo->enRslt;
			THM_LOG_I ("reqFunc00ThreadB return [%d] msg:[%s]\n", enRslt, pIf->pstSrcInfo->pszMsg);

			if (enRslt == EN_THM_RSLT_SUCCESS) {
				nSectId = SECTID_END;
				enAct = EN_THM_ACT_CONTINUE;
			} else {
				nSectId = SECTID_ERR_END;
				enAct = EN_THM_ACT_CONTINUE;
			}
		}

		} break;

	case SECTID_END:
		pIf->pReply (EN_THM_RSLT_SUCCESS, (uint8_t*)"aaaaaaaaaaaaaaaaa");
		nSectId = 0;
		enAct = EN_THM_ACT_DONE;
		break;

	case SECTID_ERR_END:
		pIf->pReply (EN_THM_RSLT_ERROR, NULL);
		nSectId = 0;
		enAct = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}

	pIf->pSetSectId (nSectId, enAct);
}

static void func02 (ST_THM_IF *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = 0,
		SECTID_REQ_THREAD_B_FUNC00,
		SECTID_WAIT_THREAD_B_FUNC00,
		SECTID_B,
		SECTID_C,
		SECTID_REQ_THREAD_B_FUNC01,
		SECTID_END,
	};

	nSectId = pIf->pGetSectId();
	THM_LOG_I ("nSectId %d\n", nSectId);

	switch (nSectId) {
	case SECTID_ENTRY: {
		char *pszMsg = (char*)(pIf->pstSrcInfo->pszMsg);
		THM_LOG_I ("msg:[%s]\n", pszMsg);

		nSectId = SECTID_REQ_THREAD_B_FUNC00;
		enAct = EN_THM_ACT_CONTINUE;

		} break;

	case SECTID_REQ_THREAD_B_FUNC00:

		THM_LOG_I ("timeout 7sec\n");
		pIf->pSetTimeout (7000);

		// スレッドBのfunc00にリクエスト
		reqFunc00ThreadB(NULL);

		nSectId = SECTID_WAIT_THREAD_B_FUNC00;
		enAct = EN_THM_ACT_WAIT;
		break;

	case SECTID_WAIT_THREAD_B_FUNC00: {
		EN_THM_RSLT enRslt = pIf->pstSrcInfo->enRslt;
		switch ((int)enRslt) {
		case EN_THM_RSLT_SUCCESS:
			THM_LOG_I ("return success reqFunc00ThreadB [%d] msg:[%s]\n", enRslt, pIf->pstSrcInfo->pszMsg);
			THM_LOG_N ("clearTimeout\n");
			pIf->pClearTimeout();
			break;
		case EN_THM_RSLT_ERROR:
			THM_LOG_E ("return error reqFunc00ThreadB [%d] msg:[%s]\n", enRslt, pIf->pstSrcInfo->pszMsg);
			THM_LOG_N ("clearTimeout\n");
			pIf->pClearTimeout();
			break;
		case EN_THM_RSLT_REQ_TIMEOUT:
			THM_LOG_E ("reqtimeout reqFunc00ThreadB");
			THM_LOG_N ("clearTimeout\n");
			pIf->pClearTimeout();
			break;
		case EN_THM_RSLT_SEQ_TIMEOUT:
			THM_LOG_E ("seqtimeout reqFunc00ThreadB");
			break;
		}

		nSectId = SECTID_B;
		enAct = EN_THM_ACT_CONTINUE;
		} break;

	case SECTID_B:
		nSectId = SECTID_C;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_C:
		nSectId = SECTID_REQ_THREAD_B_FUNC01;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_THREAD_B_FUNC01: {

		// スレッドBのfunc01にリクエスト 同期なのでここでリプライを待ちます
		func01ThreadB();
		EN_THM_RSLT enRslt = pIf->pstSrcInfo->enRslt;
		THM_LOG_I ("func01ThreadB return [%d] msg:[%s]\n", enRslt, pIf->pstSrcInfo->pszMsg);

		nSectId = SECTID_END;
		enAct = EN_THM_ACT_CONTINUE;
		} break;

	case SECTID_END:

		pIf->pReply (EN_THM_RSLT_SUCCESS, NULL);

		nSectId = 0;
		enAct = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}

	pIf->pSetSectId (nSectId, enAct);
}



// 以下公開用

void reqStartupThreadA (uint32_t *pnReqId)
{
	gpIf->pRequestAsync (EN_THREAD_A, EN_A_STARTUP, NULL, pnReqId);
}

void reqFunc00ThreadA (char *pszMsg, uint32_t *pnReqId)
{
	gpIf->pRequestAsync (EN_THREAD_A, EN_A_FUNC_00, (uint8_t*)pszMsg, pnReqId);
}

void reqFunc01ThreadA (uint32_t *pnReqId)
{
	gpIf->pRequestAsync (EN_THREAD_A, EN_A_FUNC_01, NULL, pnReqId);
}

void reqFunc02ThreadA (char *pszMsg, uint32_t *pnReqId)
{
	gpIf->pRequestAsync (EN_THREAD_A, EN_A_FUNC_02, (uint8_t*)pszMsg, pnReqId);
}