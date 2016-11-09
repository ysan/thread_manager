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
void reqAsyncThreadAfunc00 (char *pszMsg); // extern
void reqAsyncThreadAfunc01 (void); // extern
void reqAsyncThreadAfunc02 (char *pszMsg); // extern

/*
 * Variables
 */
const P_THM_FUNC gpThreadAfuncs[ EN_A_FUNC_NUM ] = {
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
	uint8_t nSeqId;
	EN_THM_SEQ enSeq;
	enum {
		SEQID_ENTRY = 0,
		SEQID_END,
	};

	nSeqId = pIf->pGetSeqId();
	THM_LOG_I ("nSeqId %d\n", nSeqId);

	pIf->pReply (EN_THM_RSLT_SUCCESS, (uint8_t*)"threadA startup end.");

	nSeqId = 0;
	enSeq = EN_THM_SEQ_DONE;
	pIf->pSetSeqId (nSeqId, enSeq);
}

static void func00 (ST_THM_IF *pIf)
{
	uint8_t nSeqId;
	EN_THM_SEQ enSeq;
	enum {
		SEQID_ENTRY = 0,
		SEQID_REQ_THREAD_A_FUNC01,
		SEQID_WAIT_THREAD_A_FUNC01,
		SEQID_END,
		SEQID_ERR_END,
	};

	nSeqId = pIf->pGetSeqId();
	THM_LOG_I ("nSeqId %d\n", nSeqId);

	switch (nSeqId) {
	case SEQID_ENTRY:
		THM_LOG_I ("msg [%s]\n", pIf->pstSrcInfo->pszMsg);

		nSeqId = SEQID_REQ_THREAD_A_FUNC01;
		enSeq = EN_THM_SEQ_CONTINUE;
		break;

	case SEQID_REQ_THREAD_A_FUNC01:

		// 自スレのfunc01にリクエスト
		reqAsyncThreadAfunc01();

		nSeqId = SEQID_WAIT_THREAD_A_FUNC01;
		enSeq = EN_THM_SEQ_WAIT;
		break;

	case SEQID_WAIT_THREAD_A_FUNC01: {
		EN_THM_RSLT enRslt = pIf->pstSrcInfo->enRslt;
		THM_LOG_I ("reqAsyncThreadAfunc01 return [%d] msg:[%s]\n", enRslt, pIf->pstSrcInfo->pszMsg);

		if (enRslt == EN_THM_RSLT_SUCCESS) {
			nSeqId = SEQID_END;
			enSeq = EN_THM_SEQ_CONTINUE;
		} else {
			nSeqId = SEQID_ERR_END;
			enSeq = EN_THM_SEQ_CONTINUE;
		}

		} break;

	case SEQID_END:
		pIf->pReply (EN_THM_RSLT_SUCCESS, NULL);
		nSeqId = 0;
		enSeq = EN_THM_SEQ_DONE;
		break;

	case SEQID_ERR_END:
		pIf->pReply (EN_THM_RSLT_ERROR, NULL);
		nSeqId = 0;
		enSeq = EN_THM_SEQ_DONE;
		break;

	default:
		break;
	}

	pIf->pSetSeqId (nSeqId, enSeq);
}

static void func01 (ST_THM_IF *pIf)
{
	uint8_t nSeqId;
	EN_THM_SEQ enSeq;
	enum {
		SEQID_ENTRY = 0,
		SEQID_REQ_THREAD_B_FUNC00,
		SEQID_WAIT_THREAD_B_FUNC00,
		SEQID_END,
		SEQID_ERR_END,
	};

	nSeqId = pIf->pGetSeqId();
	THM_LOG_I ("nSeqId %d\n", nSeqId);

	switch (nSeqId) {
	case SEQID_ENTRY:
		THM_LOG_I ("msg [%s]\n", pIf->pstSrcInfo->pszMsg);

		nSeqId = SEQID_REQ_THREAD_B_FUNC00;
		enSeq = EN_THM_SEQ_CONTINUE;
		break;

	case SEQID_REQ_THREAD_B_FUNC00:

		// スレッドBのfunc00にリクエスト
		reqAsyncThreadBfunc00();

		nSeqId = SEQID_WAIT_THREAD_B_FUNC00;
		enSeq = EN_THM_SEQ_WAIT;
		break;

	case SEQID_WAIT_THREAD_B_FUNC00: {
		EN_THM_RSLT enRslt = pIf->pstSrcInfo->enRslt;
		THM_LOG_I ("reqAsyncThreadBfunc00 return [%d] msg:[%s]\n", enRslt, pIf->pstSrcInfo->pszMsg);

		if (enRslt == EN_THM_RSLT_SUCCESS) {
			nSeqId = SEQID_END;
			enSeq = EN_THM_SEQ_CONTINUE;
		} else {
			nSeqId = SEQID_ERR_END;
			enSeq = EN_THM_SEQ_CONTINUE;
		}

		} break;

	case SEQID_END:
		pIf->pReply (EN_THM_RSLT_SUCCESS, (uint8_t*)"aaaaaaaaaaaaaaaaa");
		nSeqId = 0;
		enSeq = EN_THM_SEQ_DONE;
		break;

	case SEQID_ERR_END:
		pIf->pReply (EN_THM_RSLT_ERROR, NULL);
		nSeqId = 0;
		enSeq = EN_THM_SEQ_DONE;
		break;

	default:
		break;
	}

	pIf->pSetSeqId (nSeqId, enSeq);
}

static void func02 (ST_THM_IF *pIf)
{
	uint8_t nSeqId;
	EN_THM_SEQ enSeq;
	enum {
		SEQID_ENTRY = 0,
		SEQID_A,
		SEQID_B,
		SEQID_C,
		SEQID_REQ_THREAD_B_FUNC01,
		SEQID_END,
	};

	nSeqId = pIf->pGetSeqId();
	THM_LOG_I ("nSeqId %d\n", nSeqId);

	switch (nSeqId) {
	case SEQID_ENTRY: {
		char *pszMsg = (char*)(pIf->pstSrcInfo->pszMsg);
		THM_LOG_I ("msg:[%s]\n", pszMsg);

		nSeqId = SEQID_A;
		enSeq = EN_THM_SEQ_CONTINUE;

		} break;

	case SEQID_A:
		nSeqId = SEQID_B;
		enSeq = EN_THM_SEQ_CONTINUE;
		break;

	case SEQID_B:
		nSeqId = SEQID_C;
		enSeq = EN_THM_SEQ_CONTINUE;
		break;

	case SEQID_C:
		nSeqId = SEQID_REQ_THREAD_B_FUNC01;
		enSeq = EN_THM_SEQ_CONTINUE;
		break;

	case SEQID_REQ_THREAD_B_FUNC01: {

		// スレッドBのfunc01にリクエスト 同期なのでここでリプライを待ちます
		reqSyncThreadBfunc01();
		EN_THM_RSLT enRslt = pIf->pstSrcInfo->enRslt;
		THM_LOG_I ("reqSyncThreadBfunc01 return [%d] msg:[%s]\n", enRslt, pIf->pstSrcInfo->pszMsg);

		nSeqId = SEQID_END;
		enSeq = EN_THM_SEQ_CONTINUE;
		} break;

	case SEQID_END:

		pIf->pReply (EN_THM_RSLT_SUCCESS, NULL);

		nSeqId = 0;
		enSeq = EN_THM_SEQ_DONE;
		break;

	default:
		break;
	}

	pIf->pSetSeqId (nSeqId, enSeq);
}



// 以下公開用

void reqAsyncThreadAstartup (void)
{
	gpIf->pRequestAsync (EN_THREAD_A, EN_A_STARTUP, NULL);
}

void reqAsyncThreadAfunc00 (char *pszMsg)
{
	gpIf->pRequestAsync (EN_THREAD_A, EN_A_FUNC_00, (uint8_t*)pszMsg);
}

void reqAsyncThreadAfunc01 (void)
{
	gpIf->pRequestAsync (EN_THREAD_A, EN_A_FUNC_01, NULL);
}

void reqAsyncThreadAfunc02 (char *pszMsg)
{
	gpIf->pRequestAsync (EN_THREAD_A, EN_A_FUNC_02, (uint8_t*)pszMsg);
}
