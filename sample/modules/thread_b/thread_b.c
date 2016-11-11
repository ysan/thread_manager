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
const P_THM_SEQ gpSeqsThreadB[ EN_B_SEQ_NUM ] = {
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
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = 0,
		SECTID_REQ_REG_NOTIFY_THC,
		SECTID_WAIT_REG_NOTIFY_THC,
		SECTID_END,
		SECTID_ERR_END,
	};

	nSectId = pIf->pGetSectId();
	THM_LOG_I ("nSectId %d\n", nSectId);

	switch (nSectId) {
	case SECTID_ENTRY:
		nSectId = SECTID_REQ_REG_NOTIFY_THC;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_REG_NOTIFY_THC:
		reqAsyncThreadCregNotify();
		nSectId = SECTID_WAIT_REG_NOTIFY_THC;
		enAct = EN_THM_ACT_WAIT;
		break;

	case SECTID_WAIT_REG_NOTIFY_THC: {
		EN_THM_RSLT enRslt = pIf->pstSrcInfo->enRslt;
		gnClientId = *(pIf->pstSrcInfo->pszMsg);
		THM_LOG_I ("return reqAsyncThreadCregNotify [%d] gnClientId:[%d]\n", enRslt, gnClientId);

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

static void func00 (ST_THM_IF *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = 0,
		SECTID_END,
	};

	nSectId = pIf->pGetSectId();
	THM_LOG_I ("nSectId %d\n", nSectId);

	THM_LOG_I ("execute. sleep 2sec.\n");
	sleep (2);

	pIf->pReply (EN_THM_RSLT_SUCCESS, (uint8_t*)"thread_b func00 end.");

	nSectId = 0;
	enAct = EN_THM_ACT_DONE;
	pIf->pSetSectId (nSectId, enAct);
}

static void func01 (ST_THM_IF *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = 0,
		SECTID_END,
		SECTID_ERR_END,
	};

	nSectId = pIf->pGetSectId();
	THM_LOG_I ("nSectId %d\n", nSectId);

	pIf->pReply (EN_THM_RSLT_SUCCESS, (uint8_t*)"thread_b func01 end. -- sync --");

	nSectId = 0;
	enAct = EN_THM_ACT_DONE;
	pIf->pSetSectId (nSectId, enAct);
}

static void func02 (ST_THM_IF *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = 0,
		SECTID_REQ_THREAD_C_FUNC00,
		SECTID_WAIT_THREAD_C_FUNC00,
		SECTID_END,
		SECTID_ERR_END,
	};

	nSectId = pIf->pGetSectId();
	THM_LOG_I ("nSectId %d\n", nSectId);

	switch (nSectId) {
	case SECTID_ENTRY:
		nSectId = SECTID_REQ_THREAD_C_FUNC00;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_THREAD_C_FUNC00:
		reqAsyncThreadCfunc00();
		nSectId = SECTID_WAIT_THREAD_C_FUNC00;
		enAct = EN_THM_ACT_WAIT;
		break;

	case SECTID_WAIT_THREAD_C_FUNC00: {
		EN_THM_RSLT enRslt = pIf->pstSrcInfo->enRslt;
		THM_LOG_I ("return reqAsyncThreadCfunc00 [%d] msg:[%s]\n", enRslt, (char*)pIf->pstSrcInfo->pszMsg);

		if (enRslt == EN_THM_RSLT_SUCCESS) {
			nSectId = SECTID_END;
			enAct = EN_THM_ACT_CONTINUE;
		} else {
			if (enRslt == EN_THM_RSLT_REQ_TIMEOUT) {
				THM_LOG_W ("timeout");
			}
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
