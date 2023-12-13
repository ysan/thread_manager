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
void recv_notify_thread_c (threadmgr_if_t *p_if); // extern
static void startup (threadmgr_if_t *p_if);
static void reg_notify (threadmgr_if_t *p_if);
static void unreg_notify (threadmgr_if_t *p_if);
static void cycle_func (threadmgr_if_t *p_if);
static void func00 (threadmgr_if_t *p_if);
void req_startup_thread_c (uint32_t *pn_req_id); // extern
void req_reg_notify_thread_c (uint32_t *pn_req_id); // extern
void req_un_reg_notify_thread_c (uint8_t n_client_id, uint32_t *pn_req_id); // extern
void req_func00Thread_c (uint32_t *pn_req_id); // extern

/*
 * Variables
 */
const threadmgr_seq_t gst_seqs_thread_c[ EN_C_SEQ_NUM ] = {
	{startup, "startup"},
	{reg_notify, "reg_notify"},
	{unreg_notify, "unreg_notify"},
	{cycle_func, "cycle_func"},
	{func00, "func00"},
};
static uint8_t gn_client_id;



void recv_notify_thread_c (threadmgr_if_t *p_if)
{
}

static void startup (threadmgr_if_t *p_if)
{
	uint8_t sectid;
	EN_THM_ACT en_act;
	enum {
		SECTID_ENTRY = 0,
		SECTID_END,
	};

	sectid = p_if->get_sectid();
	THM_LOG_I ("sectid %d\n", sectid);

	gp_if->request_async (EN_THREAD_C, EN_C_CYCLE_FUNC, NULL, 0, NULL);
	// このrequestはWAITしてないのでreply queはdropする

	char *msg = "thread_c startup end.";
	p_if->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, strlen(msg));

	sectid = 0;
	en_act = EN_THM_ACT_DONE;
	p_if->set_sectid (sectid, en_act);
}

static void reg_notify (threadmgr_if_t *p_if)
{
	uint8_t sectid;
	EN_THM_ACT en_act;
	enum {
		SECTID_ENTRY = 0,
		SECTID_END,
	};

	sectid = p_if->get_sectid();
	THM_LOG_I ("sectid %d\n", sectid);

	// 複数クライアントがいるときはちゃんとid管理すること
	bool rslt = p_if->reg_notify (NOTIFY_CATEGORY, &gn_client_id);

	EN_THM_RSLT result;
	if (rslt) {
		result = EN_THM_RSLT_SUCCESS;
	} else {
		result = EN_THM_RSLT_ERROR;
	}

	// client_idをmsgで返す
	p_if->reply (result, (uint8_t*)&gn_client_id, sizeof(gn_client_id));

	sectid = 0;
	en_act = EN_THM_ACT_DONE;
	p_if->set_sectid (sectid, en_act);
}

static void unreg_notify (threadmgr_if_t *p_if)
{
	uint8_t sectid;
	EN_THM_ACT en_act;
	enum {
		SECTID_ENTRY = 0,
		SECTID_END,
	};

	sectid = p_if->get_sectid();
	THM_LOG_I ("sectid %d\n", sectid);

	EN_THM_RSLT result;
	// msgからidを取得
	uint8_t id = *(p_if->src_info->msg.msg);
	if (id != gn_client_id) {
		// ここではidの一致だけ確認 本来はちゃんと管理すべき
		THM_LOG_E ("client_id is not match.");
		p_if->reply (EN_THM_RSLT_ERROR, NULL, 0);
		result = EN_THM_RSLT_ERROR;
	} else {
		bool rslt = p_if->unreg_notify (NOTIFY_CATEGORY, id);
		if (rslt) {
			result = EN_THM_RSLT_SUCCESS;
		} else {
			result = EN_THM_RSLT_ERROR;
		}
	}

	p_if->reply (result, NULL, 0);

	sectid = 0;
	en_act = EN_THM_ACT_DONE;
	p_if->set_sectid (sectid, en_act);
}

static void cycle_func (threadmgr_if_t *p_if)
{
	uint8_t sectid;
	EN_THM_ACT en_act = EN_THM_ACT_INIT;
	enum {
		SECTID_ENTRY = 0,
		SECTID_CYCLE,
		SECTID_SEND_NOTIFY,
		SECTID_END,
	};

	sectid = p_if->get_sectid();
	THM_LOG_I ("sectid %d\n", sectid);

	switch (sectid) {
	case SECTID_ENTRY:
		// 先にreplyしておく
		p_if->reply (EN_THM_RSLT_SUCCESS, NULL, 0);

		sectid = SECTID_CYCLE;
		en_act = EN_THM_ACT_CONTINUE;
		break;
	case SECTID_CYCLE:
		p_if->set_timeout (15000);
		sectid = SECTID_SEND_NOTIFY;
//		en_act = EN_THM_ACT_TIMEOUT;
		en_act = EN_THM_ACT_WAIT;
		break;
	case SECTID_SEND_NOTIFY: {
		char *msg = "this is notify message...";
		p_if->notify (NOTIFY_CATEGORY, (uint8_t*)msg, strlen(msg));

		sectid = SECTID_CYCLE;
		en_act = EN_THM_ACT_CONTINUE;
		} break;
	case SECTID_END:
		break;
	default:
		break;
	}

	p_if->set_sectid (sectid, en_act);
}

static void func00 (threadmgr_if_t *p_if)
{
	uint8_t sectid;
	EN_THM_ACT en_act;
	enum {
		SECTID_ENTRY = 0,
		SECTID_END,
	};

	sectid = p_if->get_sectid();
	THM_LOG_I ("sectid %d\n", sectid);

	sleep (60);
	THM_LOG_I ("reply");
	p_if->reply (EN_THM_RSLT_SUCCESS, NULL, 0);

	sectid = 0;
	en_act = EN_THM_ACT_DONE;
	p_if->set_sectid (sectid, en_act);
}


// 以下公開用

void req_startup_thread_c (uint32_t *p_out_req_id)
{
	gp_if->request_async (EN_THREAD_C, EN_C_STARTUP, NULL, 0, p_out_req_id);
}

void req_reg_notify_thread_c (uint32_t *p_out_req_id)
{
	gp_if->request_async (EN_THREAD_C, EN_C_REG_NOTIFY, NULL, 0, p_out_req_id);
}

void req_un_reg_notify_thread_c (uint8_t n_client_id, uint32_t *p_out_req_id)
{
	gp_if->request_async (EN_THREAD_C, EN_C_UNREG_NOTIFY, (uint8_t*)&n_client_id, sizeof(uint8_t), p_out_req_id);
}

void req_func00_thread_c (uint32_t *p_out_req_id)
{
	gp_if->request_async (EN_THREAD_C, EN_C_FUNC_00, NULL, 0, p_out_req_id);
}
