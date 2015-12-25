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
void recvNotifyThreadB (ST_THM_IF *pIf); // extern
static void startup (ST_THM_IF *pIf);
static void func00 (ST_THM_IF *pIf);
static void func01 (ST_THM_IF *pIf);
static void func02 (ST_THM_IF *pIf);
void reqAsyncThreadBstartup (void); // extern
void reqAsyncThreadBfunc00 (void); // extern
void reqSyncThreadBfunc01 (void); // extern
void reqAsyncThreadBfunc02 (void); // extern

/*
 * Variables
 */
const P_THM_FUNC gpThreadBfuncs[ EN_B_FUNC_NUM ] = {
	startup,
	func00,
	func01,
	func02,
};
static uint8_t gnClientId;


void recvNotifyThreadB (ST_THM_IF *pIf)
{
	if (pIf->pstSrcInfo->nClientId == gnClientId) {
		THM_LOG_I ("recv notify  id:[%d] msg:[%s]\n", pIf->pstSrcInfo->nClientId, pIf->pstSrcInfo->pszMsg);
	}
}

static void startup (ST_THM_IF *pIf)
{
	uint8_t nSeqId;
	EN_THM_SEQ enSeq;
	enum {
		SEQID_ENTRY = 0,
		SEQID_REQ_REG_NOTIFY_THC,
		SEQID_WAIT_REG_NOTIFY_THC,
		SEQID_END,
		SEQID_ERR_END,
	};

	nSeqId = pIf->pGetSeqId();
	THM_LOG_I ("nSeqId %d\n", nSeqId);

	switch (nSeqId) {
	case SEQID_ENTRY:
		nSeqId = SEQID_REQ_REG_NOTIFY_THC;
		enSeq = EN_THM_SEQ_CONTINUE;
		break;

	case SEQID_REQ_REG_NOTIFY_THC:
		reqAsyncThreadCregNotify();
		nSeqId = SEQID_WAIT_REG_NOTIFY_THC;
		enSeq = EN_THM_SEQ_WAIT;
		break;

	case SEQID_WAIT_REG_NOTIFY_THC: {
		EN_THM_RSLT enRslt = pIf->pstSrcInfo->enRslt;
		gnClientId = *(pIf->pstSrcInfo->pszMsg);
		THM_LOG_I ("return reqAsyncThreadCregNotify [%d] gnClientId:[%d]\n", enRslt, gnClientId);

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

static void func00 (ST_THM_IF *pIf)
{
	uint8_t nSeqId;
	EN_THM_SEQ enSeq;
	enum {
		SEQID_ENTRY = 0,
		SEQID_END,
	};

	nSeqId = pIf->pGetSeqId();
	THM_LOG_I ("nSeqId %d\n", nSeqId);

	THM_LOG_I ("execute. sleep 1sec.\n");
	sleep (1);

	pIf->pReply (EN_THM_RSLT_SUCCESS, (uint8_t*)"thread_b func00 end.");

	nSeqId = 0;
	enSeq = EN_THM_SEQ_DONE;
	pIf->pSetSeqId (nSeqId, enSeq);
}

static void func01 (ST_THM_IF *pIf)
{
	uint8_t nSeqId;
	EN_THM_SEQ enSeq;
	enum {
		SEQID_ENTRY = 0,
		SEQID_END,
		SEQID_ERR_END,
	};

	nSeqId = pIf->pGetSeqId();
	THM_LOG_I ("nSeqId %d\n", nSeqId);

	pIf->pReply (EN_THM_RSLT_SUCCESS, (uint8_t*)"thread_b func01 end. -- sync --");

	nSeqId = 0;
	enSeq = EN_THM_SEQ_DONE;
	pIf->pSetSeqId (nSeqId, enSeq);
}

static void func02 (ST_THM_IF *pIf)
{
	uint8_t nSeqId;
	EN_THM_SEQ enSeq;
	enum {
		SEQID_ENTRY = 0,
		SEQID_REQ_THREAD_C_FUNC00,
		SEQID_WAIT_THREAD_C_FUNC00,
		SEQID_END,
		SEQID_ERR_END,
	};

	nSeqId = pIf->pGetSeqId();
	THM_LOG_I ("nSeqId %d\n", nSeqId);

	switch (nSeqId) {
	case SEQID_ENTRY:
		nSeqId = SEQID_REQ_THREAD_C_FUNC00;
		enSeq = EN_THM_SEQ_CONTINUE;
		break;

	case SEQID_REQ_THREAD_C_FUNC00:
		reqAsyncThreadCfunc00();
		nSeqId = SEQID_WAIT_THREAD_C_FUNC00;
		enSeq = EN_THM_SEQ_WAIT;
		break;

	case SEQID_WAIT_THREAD_C_FUNC00: {
		EN_THM_RSLT enRslt = pIf->pstSrcInfo->enRslt;
		THM_LOG_I ("return reqAsyncThreadCfunc00 [%d] msg:[%s]\n", enRslt, (char*)pIf->pstSrcInfo->pszMsg);

		if (enRslt == EN_THM_RSLT_SUCCESS) {
			nSeqId = SEQID_END;
			enSeq = EN_THM_SEQ_CONTINUE;
		} else {
			if (enRslt == EN_THM_RSLT_TIMEOUT) {
				THM_LOG_W ("timeout");
			}
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



// 以下公開用

void reqAsyncThreadBstartup (void)
{
	gpIf->pRequestAsync (EN_THREAD_B, EN_B_STARTUP, NULL);
}

void reqAsyncThreadBfunc00 (void)
{
	gpIf->pRequestAsync (EN_THREAD_B, EN_B_FUNC_00, NULL);
}

void reqSyncThreadBfunc01 (void)
{
	gpIf->pRequestSync (EN_THREAD_B, EN_B_FUNC_01, NULL);
}

void reqAsyncThreadBfunc02 (void)
{
	gpIf->pRequestAsync (EN_THREAD_B, EN_B_FUNC_02, NULL);
}
