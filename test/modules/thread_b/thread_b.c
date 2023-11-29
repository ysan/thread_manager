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
void recv_notify_thread_b (threadmgr_if_t *p_if); // extern
static void startup (threadmgr_if_t *p_if);
static void func00 (threadmgr_if_t *p_if);
static void func01 (threadmgr_if_t *p_if);
static void func02 (threadmgr_if_t *p_if);
void req_startup_thread_b (uint32_t *p_req_id); // extern
void req_func00_thread_b (uint32_t *p_req_id); // extern
void func01_thread_b (void); // extern
void req_func02_thread_b (uint32_t *p_req_id); // extern

/*
 * Variables
 */
const threadmgr_seq_t gst_seqs_thread_b[ EN_B_SEQ_NUM ] = {
	{startup, "startup"},
	{func00, "func00"},
	{func01, "func01"},
	{func02, "func02"},
};
static uint8_t gn_client_id;


void recv_notify_thread_b (threadmgr_if_t *p_if)
{
	if (p_if->src_info->client_id == gn_client_id) {
		THM_LOG_I ("recv notify  id:[%d] msg:[%s]\n", p_if->src_info->client_id, (char*)p_if->src_info->msg.msg);
	}
}

static void startup (threadmgr_if_t *p_if)
{
	uint8_t sectid;
	EN_THM_ACT en_act = EN_THM_ACT_INIT;
	enum {
		SECTID_ENTRY = 0,
		SECTID_REQ_REG_NOTIFY_THC,
		SECTID_WAIT_REG_NOTIFY_THC,
		SECTID_END,
		SECTID_ERR_END,
	};

	sectid = p_if->pfn_get_sectid();
	THM_LOG_I ("sectid %d\n", sectid);

	switch (sectid) {
	case SECTID_ENTRY:
		sectid = SECTID_REQ_REG_NOTIFY_THC;
		en_act = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_REG_NOTIFY_THC:
		req_reg_notify_thread_c (NULL);
		sectid = SECTID_WAIT_REG_NOTIFY_THC;
		en_act = EN_THM_ACT_WAIT;
		break;

	case SECTID_WAIT_REG_NOTIFY_THC: {
		EN_THM_RSLT en_rslt = p_if->src_info->en_rslt;
		gn_client_id = *(p_if->src_info->msg.msg);
		THM_LOG_I ("return req_reg_notify_thread_c [%d] gn_client_id:[%d]\n", en_rslt, gn_client_id);

		if (en_rslt == EN_THM_RSLT_SUCCESS) {
			sectid = SECTID_END;
			en_act = EN_THM_ACT_CONTINUE;
		} else {
			sectid = SECTID_ERR_END;
			en_act = EN_THM_ACT_CONTINUE;
		}

		} break;

	case SECTID_END:
		p_if->pfn_reply (EN_THM_RSLT_SUCCESS, NULL, 0);
		sectid = 0;
		en_act = EN_THM_ACT_DONE;
		break;

	case SECTID_ERR_END:
		p_if->pfn_reply (EN_THM_RSLT_ERROR, NULL, 0);
		sectid = 0;
		en_act = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}

	p_if->pfn_set_sectid (sectid, en_act);
}

static void func00 (threadmgr_if_t *p_if)
{
	uint8_t sectid;
	EN_THM_ACT en_act;
	enum {
		SECTID_ENTRY = 0,
		SECTID_END,
	};

	sectid = p_if->pfn_get_sectid();
	THM_LOG_I ("sectid %d\n", sectid);

	THM_LOG_I ("execute. sleep 2sec.\n");
	sleep (2);

	char *msg = "thread_b func00 end.";
	p_if->pfn_reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, strlen(msg));

	sectid = 0;
	en_act = EN_THM_ACT_DONE;
	p_if->pfn_set_sectid (sectid, en_act);
}

static void func01 (threadmgr_if_t *p_if)
{
	uint8_t sectid;
	EN_THM_ACT en_act;
	enum {
		SECTID_ENTRY = 0,
		SECTID_END,
		SECTID_ERR_END,
	};

	sectid = p_if->pfn_get_sectid();
	THM_LOG_I ("sectid %d\n", sectid);

	char *msg = "thread_b func01 end. -- sync --";
	p_if->pfn_reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, strlen(msg));

	sectid = 0;
	en_act = EN_THM_ACT_DONE;
	p_if->pfn_set_sectid (sectid, en_act);
}

static void func02 (threadmgr_if_t *p_if)
{
	uint8_t sectid;
	EN_THM_ACT en_act = EN_THM_ACT_INIT;
	enum {
		SECTID_ENTRY = 0,
		SECTID_REQ_FUNC00_THREAD_C,
		SECTID_WAIT_FUNC00_THREAD_C,
		SECTID_END,
		SECTID_ERR_END,
	};

	sectid = p_if->pfn_get_sectid();
	THM_LOG_I ("sectid %d\n", sectid);

	switch (sectid) {
	case SECTID_ENTRY:
		sectid = SECTID_REQ_FUNC00_THREAD_C;
		en_act = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_FUNC00_THREAD_C:
		req_func00_thread_c (NULL);
		sectid = SECTID_WAIT_FUNC00_THREAD_C;
		en_act = EN_THM_ACT_WAIT;
		break;

	case SECTID_WAIT_FUNC00_THREAD_C: {
		EN_THM_RSLT en_rslt = p_if->src_info->en_rslt;
		switch ((int)en_rslt) {
		case EN_THM_RSLT_SUCCESS:
			THM_LOG_I ("return success req_func00_thread_c [%d] msg:[%s]\n", en_rslt, (char*)p_if->src_info->msg.msg);
			sectid = SECTID_END;
			en_act = EN_THM_ACT_CONTINUE;
			break;
		case EN_THM_RSLT_ERROR:
			THM_LOG_E ("return error req_func00_thread_c [%d] msg:[%s]\n", en_rslt, (char*)p_if->src_info->msg.msg);
			sectid = SECTID_ERR_END;
			en_act = EN_THM_ACT_CONTINUE;
			break;
		case EN_THM_RSLT_REQ_TIMEOUT:
			THM_LOG_E ("timeout req_func00_thread_c");
			sectid = SECTID_ERR_END;
			en_act = EN_THM_ACT_CONTINUE;
			break;
		}

		} break;

	case SECTID_END:
		p_if->pfn_reply (EN_THM_RSLT_SUCCESS, NULL, 0);
		sectid = 0;
		en_act = EN_THM_ACT_DONE;
		break;

	case SECTID_ERR_END:
		p_if->pfn_reply (EN_THM_RSLT_ERROR, NULL, 0);
		sectid = 0;
		en_act = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}

	p_if->pfn_set_sectid (sectid, en_act);
}



// 以下公開用

void req_startup_thread_b (uint32_t *p_out_req_id)
{
	gp_if->pfn_request_async (EN_THREAD_B, EN_B_STARTUP, NULL, 0, p_out_req_id);
}

void req_func00_thread_b (uint32_t *p_out_req_id)
{
	gp_if->pfn_request_async (EN_THREAD_B, EN_B_FUNC_00, NULL, 0, p_out_req_id);
}

void func01_thread_b (void)
{
	gp_if->pfn_request_sync (EN_THREAD_B, EN_B_FUNC_01, NULL, 0);
}

void req_func02_thread_b (uint32_t *p_out_req_id)
{
	gp_if->pfn_request_async (EN_THREAD_B, EN_B_FUNC_02, NULL, 0, p_out_req_id);
}
