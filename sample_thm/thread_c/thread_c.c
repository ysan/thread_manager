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
void recvNotifyThreadC (ST_THM_IF *pIf); // extern
static void startup (ST_THM_IF *pIf);
static void regNotify (ST_THM_IF *pIf);
static void unregNotify (ST_THM_IF *pIf);
static void cycleFunc (ST_THM_IF *pIf);
static void func00 (ST_THM_IF *pIf);
void reqAsyncThreadCstartup (void); // extern
void reqAsyncThreadCregNotify (void); // extern
void reqAsyncThreadCunRegNotify (uint8_t nClientId); // extern
void reqAsyncThreadCfunc00 (void); // extern

/*
 * Variables
 */
const P_THM_FUNC gpThreadCfuncs[ EN_C_FUNC_NUM ] = {
	startup,
	regNotify,
	unregNotify,
	cycleFunc,
	func00,
};
static uint8_t gnClientId;



void recvNotifyThreadC (ST_THM_IF *pIf)
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

	gpIf->pRequestAsync (EN_THREAD_C, EN_C_CYCLE_FUNC, NULL);

	pIf->pReply (EN_THM_RSLT_SUCCESS, (uint8_t*)"threadC startup end.");

	nSeqId = 0;
	enSeq = EN_THM_SEQ_DONE;
	pIf->pSetSeqId (nSeqId, enSeq);
}

static void regNotify (ST_THM_IF *pIf)
{
	uint8_t nSeqId;
	EN_THM_SEQ enSeq;
	enum {
		SEQID_ENTRY = 0,
		SEQID_END,
	};

	nSeqId = pIf->pGetSeqId();
	THM_LOG_I ("nSeqId %d\n", nSeqId);

	// 複数クライアントがいるときはちゃんとid管理すること
	bool rslt = pIf->pRegNotify (&gnClientId);

	EN_THM_RSLT enRslt;
	if (rslt) {
		enRslt = EN_THM_RSLT_SUCCESS;
	} else {
		enRslt = EN_THM_RSLT_ERROR;
	}

	// clientIdをmsgで返す
	pIf->pReply (enRslt, (uint8_t*)&gnClientId);

	nSeqId = 0;
	enSeq = EN_THM_SEQ_DONE;
	pIf->pSetSeqId (nSeqId, enSeq);
}

static void unregNotify (ST_THM_IF *pIf)
{
	uint8_t nSeqId;
	EN_THM_SEQ enSeq;
	enum {
		SEQID_ENTRY = 0,
		SEQID_END,
	};

	nSeqId = pIf->pGetSeqId();
	THM_LOG_I ("nSeqId %d\n", nSeqId);

	EN_THM_RSLT enRslt;
	// msgからidを取得
	uint8_t id = *(pIf->pstSrcInfo->pszMsg);
	if (id != gnClientId) {
		// ここではidの一致だけ確認 本来はちゃんと管理すべき
		THM_LOG_E ("clientId is not match.");
		pIf->pReply (EN_THM_RSLT_ERROR, NULL);
		enRslt = EN_THM_RSLT_ERROR;
	} else {
		bool rslt = pIf->pUnRegNotify (id);
		if (rslt) {
			enRslt = EN_THM_RSLT_SUCCESS;
		} else {
			enRslt = EN_THM_RSLT_ERROR;
		}
	}

	pIf->pReply (enRslt, NULL);

	nSeqId = 0;
	enSeq = EN_THM_SEQ_DONE;
	pIf->pSetSeqId (nSeqId, enSeq);
}

static void cycleFunc (ST_THM_IF *pIf)
{
	uint8_t nSeqId;
	EN_THM_SEQ enSeq;
	enum {
		SEQID_ENTRY = 0,
		SEQID_CYCLE,
		SEQID_SEND_NOTIFY,
		SEQID_END,
	};

	nSeqId = pIf->pGetSeqId();
	THM_LOG_I ("nSeqId %d\n", nSeqId);

	switch (nSeqId) {
	case SEQID_ENTRY:
		nSeqId = SEQID_CYCLE;
		enSeq = EN_THM_SEQ_CONTINUE;
		break;
	case SEQID_CYCLE:
		pIf->pSetTimeout (60000);
		nSeqId = SEQID_SEND_NOTIFY;
		enSeq = EN_THM_SEQ_TIMEOUT;
		break;
	case SEQID_SEND_NOTIFY:
		pIf->pNotify (gnClientId, (uint8_t*)"this is notify message...");

		nSeqId = SEQID_CYCLE;
		enSeq = EN_THM_SEQ_CONTINUE;
		break;
	case SEQID_END:
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

	sleep (60);
	THM_LOG_I ("reply");
	pIf->pReply (EN_THM_RSLT_SUCCESS, NULL);

	nSeqId = 0;
	enSeq = EN_THM_SEQ_DONE;
	pIf->pSetSeqId (nSeqId, enSeq);
}


// 以下公開用

void reqAsyncThreadCstartup (void )
{
	gpIf->pRequestAsync (EN_THREAD_C, EN_C_STARTUP, NULL);
}

void reqAsyncThreadCregNotify (void)
{
	gpIf->pRequestAsync (EN_THREAD_C, EN_C_REG_NOTIFY, NULL);
}

void reqAsyncThreadCunRegNotify (uint8_t nClientId)
{
	gpIf->pRequestAsync (EN_THREAD_C, EN_C_UNREG_NOTIFY, (uint8_t*)&nClientId);
}

void reqAsyncThreadCfunc00 (void)
{
	gpIf->pRequestAsync (EN_THREAD_C, EN_C_FUNC_00, NULL);
}
