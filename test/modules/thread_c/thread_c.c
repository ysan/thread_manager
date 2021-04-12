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

#define NOTIFY_CATEGORY		(0)

/*
 * Prototypes
 */
void recvNotifyThreadC (ST_THM_IF *pIf); // extern
static void startup (ST_THM_IF *pIf);
static void regNotify (ST_THM_IF *pIf);
static void unregNotify (ST_THM_IF *pIf);
static void cycleFunc (ST_THM_IF *pIf);
static void func00 (ST_THM_IF *pIf);
void reqStartupThreadC (uint32_t *pnReqId); // extern
void reqRegNotifyThreadC (uint32_t *pnReqId); // extern
void reqUnRegNotifyThreadC (uint8_t nClientId, uint32_t *pnReqId); // extern
void reqFunc00ThreadC (uint32_t *pnReqId); // extern

/*
 * Variables
 */
const ST_THM_SEQ gstSeqsThreadC[ EN_C_SEQ_NUM ] = {
	{startup, "startup"},
	{regNotify, "regNotify"},
	{unregNotify, "unregNotify"},
	{cycleFunc, "cycleFunc"},
	{func00, "func00"},
};
static uint8_t gnClientId;



void recvNotifyThreadC (ST_THM_IF *pIf)
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

	nSectId = pIf->pfnGetSectId();
	THM_LOG_I ("nSectId %d\n", nSectId);

	gpIf->pfnRequestAsync (EN_THREAD_C, EN_C_CYCLE_FUNC, NULL, 0, NULL);
	// このrequestはWAITしてないのでreply queはdropする

	char *msg = "threadC startup end.";
	pIf->pfnReply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, strlen(msg));

	nSectId = 0;
	enAct = EN_THM_ACT_DONE;
	pIf->pfnSetSectId (nSectId, enAct);
}

static void regNotify (ST_THM_IF *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = 0,
		SECTID_END,
	};

	nSectId = pIf->pfnGetSectId();
	THM_LOG_I ("nSectId %d\n", nSectId);

	// 複数クライアントがいるときはちゃんとid管理すること
	bool rslt = pIf->pfnRegNotify (NOTIFY_CATEGORY, &gnClientId);

	EN_THM_RSLT enRslt;
	if (rslt) {
		enRslt = EN_THM_RSLT_SUCCESS;
	} else {
		enRslt = EN_THM_RSLT_ERROR;
	}

	// clientIdをmsgで返す
	pIf->pfnReply (enRslt, (uint8_t*)&gnClientId, sizeof(gnClientId));

	nSectId = 0;
	enAct = EN_THM_ACT_DONE;
	pIf->pfnSetSectId (nSectId, enAct);
}

static void unregNotify (ST_THM_IF *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = 0,
		SECTID_END,
	};

	nSectId = pIf->pfnGetSectId();
	THM_LOG_I ("nSectId %d\n", nSectId);

	EN_THM_RSLT enRslt;
	// msgからidを取得
	uint8_t id = *(pIf->pstSrcInfo->msg.pMsg);
	if (id != gnClientId) {
		// ここではidの一致だけ確認 本来はちゃんと管理すべき
		THM_LOG_E ("clientId is not match.");
		pIf->pfnReply (EN_THM_RSLT_ERROR, NULL, 0);
		enRslt = EN_THM_RSLT_ERROR;
	} else {
		bool rslt = pIf->pfnUnRegNotify (NOTIFY_CATEGORY, id);
		if (rslt) {
			enRslt = EN_THM_RSLT_SUCCESS;
		} else {
			enRslt = EN_THM_RSLT_ERROR;
		}
	}

	pIf->pfnReply (enRslt, NULL, 0);

	nSectId = 0;
	enAct = EN_THM_ACT_DONE;
	pIf->pfnSetSectId (nSectId, enAct);
}

static void cycleFunc (ST_THM_IF *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct = EN_THM_ACT_INIT;
	enum {
		SECTID_ENTRY = 0,
		SECTID_CYCLE,
		SECTID_SEND_NOTIFY,
		SECTID_END,
	};

	nSectId = pIf->pfnGetSectId();
	THM_LOG_I ("nSectId %d\n", nSectId);

	switch (nSectId) {
	case SECTID_ENTRY:
		// 先にreplyしておく
		pIf->pfnReply (EN_THM_RSLT_SUCCESS, NULL, 0);

		nSectId = SECTID_CYCLE;
		enAct = EN_THM_ACT_CONTINUE;
		break;
	case SECTID_CYCLE:
		pIf->pfnSetTimeout (15000);
		nSectId = SECTID_SEND_NOTIFY;
//		enAct = EN_THM_ACT_TIMEOUT;
		enAct = EN_THM_ACT_WAIT;
		break;
	case SECTID_SEND_NOTIFY: {
		char *msg = "this is notify message...";
		pIf->pfnNotify (NOTIFY_CATEGORY, (uint8_t*)msg, strlen(msg));

		nSectId = SECTID_CYCLE;
		enAct = EN_THM_ACT_CONTINUE;
		} break;
	case SECTID_END:
		break;
	default:
		break;
	}

	pIf->pfnSetSectId (nSectId, enAct);
}

static void func00 (ST_THM_IF *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = 0,
		SECTID_END,
	};

	nSectId = pIf->pfnGetSectId();
	THM_LOG_I ("nSectId %d\n", nSectId);

	sleep (60);
	THM_LOG_I ("reply");
	pIf->pfnReply (EN_THM_RSLT_SUCCESS, NULL, 0);

	nSectId = 0;
	enAct = EN_THM_ACT_DONE;
	pIf->pfnSetSectId (nSectId, enAct);
}


// 以下公開用

void reqStartupThreadC (uint32_t *pOutReqId)
{
	gpIf->pfnRequestAsync (EN_THREAD_C, EN_C_STARTUP, NULL, 0, pOutReqId);
}

void reqRegNotifyThreadC (uint32_t *pOutReqId)
{
	gpIf->pfnRequestAsync (EN_THREAD_C, EN_C_REG_NOTIFY, NULL, 0, pOutReqId);
}

void reqUnRegNotifyThreadC (uint8_t nClientId, uint32_t *pOutReqId)
{
	gpIf->pfnRequestAsync (EN_THREAD_C, EN_C_UNREG_NOTIFY, (uint8_t*)&nClientId, sizeof(uint8_t), pOutReqId);
}

void reqFunc00ThreadC (uint32_t *pOutReqId)
{
	gpIf->pfnRequestAsync (EN_THREAD_C, EN_C_FUNC_00, NULL, 0, pOutReqId);
}
