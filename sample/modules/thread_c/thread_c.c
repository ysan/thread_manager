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
void reqStartupThreadC (uint32_t *pnReqId); // extern
void reqRegNotifyThreadC (uint32_t *pnReqId); // extern
void reqUnRegNotifyThreadC (uint8_t nClientId, uint32_t *pnReqId); // extern
void reqFunc00ThreadC (uint32_t *pnReqId); // extern

/*
 * Variables
 */
const P_THM_SEQ gpSeqsThreadC[ EN_C_SEQ_NUM ] = {
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
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = 0,
		SECTID_END,
	};

	nSectId = pIf->pGetSectId();
	THM_LOG_I ("nSectId %d\n", nSectId);

	gpIf->pRequestAsync (EN_THREAD_C, EN_C_CYCLE_FUNC, NULL, NULL);
	// このrequestはWAITしてないの自身ではタイムアウトになる

	pIf->pReply (EN_THM_RSLT_SUCCESS, (uint8_t*)"threadC startup end.");

	nSectId = 0;
	enAct = EN_THM_ACT_DONE;
	pIf->pSetSectId (nSectId, enAct);
}

static void regNotify (ST_THM_IF *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = 0,
		SECTID_END,
	};

	nSectId = pIf->pGetSectId();
	THM_LOG_I ("nSectId %d\n", nSectId);

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

	nSectId = 0;
	enAct = EN_THM_ACT_DONE;
	pIf->pSetSectId (nSectId, enAct);
}

static void unregNotify (ST_THM_IF *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = 0,
		SECTID_END,
	};

	nSectId = pIf->pGetSectId();
	THM_LOG_I ("nSectId %d\n", nSectId);

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

	nSectId = 0;
	enAct = EN_THM_ACT_DONE;
	pIf->pSetSectId (nSectId, enAct);
}

static void cycleFunc (ST_THM_IF *pIf)
{
	uint8_t nSectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = 0,
		SECTID_CYCLE,
		SECTID_SEND_NOTIFY,
		SECTID_END,
	};

	nSectId = pIf->pGetSectId();
	THM_LOG_I ("nSectId %d\n", nSectId);

	switch (nSectId) {
	case SECTID_ENTRY:
		// 先にreplyしておく
		pIf->pReply (EN_THM_RSLT_SUCCESS, NULL);

		nSectId = SECTID_CYCLE;
		enAct = EN_THM_ACT_CONTINUE;
		break;
	case SECTID_CYCLE:
		pIf->pSetTimeout (15000);
		nSectId = SECTID_SEND_NOTIFY;
//		enAct = EN_THM_ACT_TIMEOUT;
		enAct = EN_THM_ACT_WAIT;
		break;
	case SECTID_SEND_NOTIFY:
		pIf->pNotify (gnClientId, (uint8_t*)"this is notify message...");

		nSectId = SECTID_CYCLE;
		enAct = EN_THM_ACT_CONTINUE;
		break;
	case SECTID_END:
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

	sleep (60);
	THM_LOG_I ("reply");
	pIf->pReply (EN_THM_RSLT_SUCCESS, NULL);

	nSectId = 0;
	enAct = EN_THM_ACT_DONE;
	pIf->pSetSectId (nSectId, enAct);
}


// 以下公開用

void reqStartupThreadC (uint32_t *pnReqId)
{
	gpIf->pRequestAsync (EN_THREAD_C, EN_C_STARTUP, NULL, pnReqId);
}

void reqRegNotifyThreadC (uint32_t *pnReqId)
{
	gpIf->pRequestAsync (EN_THREAD_C, EN_C_REG_NOTIFY, NULL, pnReqId);
}

void reqUnRegNotifyThreadC (uint8_t nClientId, uint32_t *pnReqId)
{
	gpIf->pRequestAsync (EN_THREAD_C, EN_C_UNREG_NOTIFY, (uint8_t*)&nClientId, pnReqId);
}

void reqFunc00ThreadC (uint32_t *pnReqId)
{
	gpIf->pRequestAsync (EN_THREAD_C, EN_C_FUNC_00, NULL, pnReqId);
}
