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
void reqStartupThreadB (uint32_t *pnReqId); // extern
void reqFunc00ThreadB (uint32_t *pnReqId); // extern
void func01ThreadB (void); // extern
void reqFunc02ThreadB (uint32_t *pnReqId); // extern

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
		reqRegNotifyThreadC (NULL);
		nSectId = SECTID_WAIT_REG_NOTIFY_THC;
		enAct = EN_THM_ACT_WAIT;
		break;

	case SECTID_WAIT_REG_NOTIFY_THC: {
		EN_THM_RSLT enRslt = pIf->pstSrcInfo->enRslt;
		gnClientId = *(pIf->pstSrcInfo->pszMsg);
		THM_LOG_I ("return reqRegNotifyThreadC [%d] gnClientId:[%d]\n", enRslt, gnClientId);

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
		SECTID_REQ_FUNC00_THREAD_C,
		SECTID_WAIT_FUNC00_THREAD_C,
		SECTID_END,
		SECTID_ERR_END,
	};

	nSectId = pIf->pGetSectId();
	THM_LOG_I ("nSectId %d\n", nSectId);

	switch (nSectId) {
	case SECTID_ENTRY:
		nSectId = SECTID_REQ_FUNC00_THREAD_C;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_FUNC00_THREAD_C:
		reqFunc00ThreadC (NULL);
		nSectId = SECTID_WAIT_FUNC00_THREAD_C;
		enAct = EN_THM_ACT_WAIT;
		break;

	case SECTID_WAIT_FUNC00_THREAD_C: {
		EN_THM_RSLT enRslt = pIf->pstSrcInfo->enRslt;
		switch ((int)enRslt) {
		case EN_THM_RSLT_SUCCESS:
			THM_LOG_I ("return success reqFunc00ThreadC [%d] msg:[%s]\n", enRslt, (char*)pIf->pstSrcInfo->pszMsg);
			nSectId = SECTID_END;
			enAct = EN_THM_ACT_CONTINUE;
			break;
		case EN_THM_RSLT_ERROR:
			THM_LOG_E ("return error reqFunc00ThreadC [%d] msg:[%s]\n", enRslt, (char*)pIf->pstSrcInfo->pszMsg);
			nSectId = SECTID_ERR_END;
			enAct = EN_THM_ACT_CONTINUE;
			break;
		case EN_THM_RSLT_REQ_TIMEOUT:
			THM_LOG_E ("timeout reqFunc00ThreadC");
			nSectId = SECTID_ERR_END;
			enAct = EN_THM_ACT_CONTINUE;
			break;
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

void reqStartupThreadB (uint32_t *pnReqId)
{
	gpIf->pRequestAsync (EN_THREAD_B, EN_B_STARTUP, NULL, pnReqId);
}

void reqFunc00ThreadB (uint32_t *pnReqId)
{
	gpIf->pRequestAsync (EN_THREAD_B, EN_B_FUNC_00, NULL, pnReqId);
}

void func01ThreadB (void)
{
	gpIf->pRequestSync (EN_THREAD_B, EN_B_FUNC_01, NULL);
}

void reqFunc02ThreadB (uint32_t *pnReqId)
{
	gpIf->pRequestAsync (EN_THREAD_B, EN_B_FUNC_02, NULL, pnReqId);
}
