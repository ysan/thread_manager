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
void recv_notify_thread_a (threadmgr_if_t *p_if); // extern
static void startup (threadmgr_if_t *p_if);
static void func00 (threadmgr_if_t *p_if);
static void func01 (threadmgr_if_t *p_if);
static void func02 (threadmgr_if_t *p_if);
static void func03 (threadmgr_if_t *p_if);
void req_startup_thread_a (uint32_t *preq_id); // extern
void req_func00_thread_a (char *psz_msg, uint32_t *preq_id); // extern
void req_func01_thread_a (uint32_t *preq_id); // extern
void req_func02_thread_a (char *psz_msg, uint32_t *preq_id); // extern
void req_func03_thread_a (uint32_t *preq_id); // extern

/*
 * Variables
 */
const threadmgr_seq_t gst_seqs_thread_a [EN_A_SEQ_NUM] = {
	{startup, "startup"},
	{func00, "func00"},
	{func01, "func01"},
	{func02, "func02"},
	{func03, "func03"},
};
static uint8_t gn_client_id;



void recv_notify_thread_a (threadmgr_if_t *p_if)
{
}

static void startup (threadmgr_if_t *p_if)
{
	uint8_t n_sectid;
	EN_THM_ACT en_act;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_END,
	};

	n_sectid = p_if->get_sectid();
	THM_LOG_I ("n_sectid %d\n", n_sectid);

	char *msg = "thread_a startup end.";
	p_if->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, strlen(msg));

	n_sectid = THM_SECT_ID_INIT;
	en_act = EN_THM_ACT_DONE;
	p_if->set_sectid (n_sectid, en_act);
}

static void func00 (threadmgr_if_t *p_if)
{
	uint8_t n_sectid;
	EN_THM_ACT en_act = EN_THM_ACT_INIT;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_REQ_THREAD_A_FUNC01,
		SECTID_WAIT_THREAD_A_FUNC01,
		SECTID_END,
		SECTID_ERR_END,
	};

	n_sectid = p_if->get_sectid();
	THM_LOG_I ("n_sectid %d\n", n_sectid);

	switch (n_sectid) {
	case SECTID_ENTRY:
		THM_LOG_I ("msg [%s]\n", (char*)p_if->src_info->msg.msg);

		n_sectid = SECTID_REQ_THREAD_A_FUNC01;
		en_act = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_THREAD_A_FUNC01:

		// 自スレのfunc01にリクエスト
		req_func01_thread_a(NULL);

		n_sectid = SECTID_WAIT_THREAD_A_FUNC01;
		en_act = EN_THM_ACT_WAIT;
		break;

	case SECTID_WAIT_THREAD_A_FUNC01: {
		EN_THM_RSLT result = p_if->src_info->result;
		THM_LOG_I ("req_func01_thread_a return [%d] msg:[%s]\n", result, (char*)p_if->src_info->msg.msg);

		if (result == EN_THM_RSLT_SUCCESS) {
			n_sectid = SECTID_END;
			en_act = EN_THM_ACT_CONTINUE;
		} else {
			n_sectid = SECTID_ERR_END;
			en_act = EN_THM_ACT_CONTINUE;
		}

		} break;

	case SECTID_END:
		p_if->reply (EN_THM_RSLT_SUCCESS, NULL, 0);
		n_sectid = THM_SECT_ID_INIT;
		en_act = EN_THM_ACT_DONE;
		break;

	case SECTID_ERR_END:
		p_if->reply (EN_THM_RSLT_ERROR, NULL, 0);
		n_sectid = THM_SECT_ID_INIT;
		en_act = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}

	p_if->set_sectid (n_sectid, en_act);
}

static uint32_t gn_tmp_req_id;
static void func01 (threadmgr_if_t *p_if)
{
	uint8_t n_sectid;
	EN_THM_ACT en_act = EN_THM_ACT_INIT;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_REQ_THREAD_B_FUNC00,
		SECTID_WAIT_THREAD_B_FUNC00,
		SECTID_END,
		SECTID_ERR_END,
	};

	n_sectid = p_if->get_sectid();
	THM_LOG_I ("n_sectid %d\n", n_sectid);

	switch (n_sectid) {
	case SECTID_ENTRY:
		THM_LOG_I ("msg [%s]\n", (char*)p_if->src_info->msg.msg);

		n_sectid = SECTID_REQ_THREAD_B_FUNC00;
		en_act = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_THREAD_B_FUNC00:

		// スレッドBのfunc00にリクエスト
		req_func00_thread_b (NULL);
		req_func00_thread_b (NULL);
		req_func00_thread_b (NULL);
		req_func00_thread_b (NULL);
		req_func00_thread_b (&gn_tmp_req_id);
		THM_LOG_I ("req_func00_thread_b reqid:[%d]\n", gn_tmp_req_id);

		n_sectid = SECTID_WAIT_THREAD_B_FUNC00;
		en_act = EN_THM_ACT_WAIT;
		break;

	case SECTID_WAIT_THREAD_B_FUNC00: {
		if (p_if->src_info->req_id != gn_tmp_req_id) {
			THM_LOG_W ("different reqid %d\n", p_if->src_info->req_id);
			n_sectid = SECTID_WAIT_THREAD_B_FUNC00;
			en_act = EN_THM_ACT_WAIT;

		} else {
			EN_THM_RSLT result = p_if->src_info->result;
			THM_LOG_I ("req_func00_thread_b return [%d] msg:[%s]\n", result, (char*)p_if->src_info->msg.msg);

			if (result == EN_THM_RSLT_SUCCESS) {
				n_sectid = SECTID_END;
				en_act = EN_THM_ACT_CONTINUE;
			} else {
				n_sectid = SECTID_ERR_END;
				en_act = EN_THM_ACT_CONTINUE;
			}
		}

		} break;

	case SECTID_END: {
		char *msg = "aaaaaaaaaaaaaaaaa";
		p_if->reply (EN_THM_RSLT_SUCCESS, (uint8_t*)msg, strlen(msg));
		n_sectid = THM_SECT_ID_INIT;
		en_act = EN_THM_ACT_DONE;
		} break;

	case SECTID_ERR_END:
		p_if->reply (EN_THM_RSLT_ERROR, NULL, 0);
		n_sectid = THM_SECT_ID_INIT;
		en_act = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}

	p_if->set_sectid (n_sectid, en_act);
}

static void func02 (threadmgr_if_t *p_if)
{
	uint8_t n_sectid;
	EN_THM_ACT en_act;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_REQ_THREAD_B_FUNC00,
		SECTID_WAIT_THREAD_B_FUNC00,
		SECTID_B,
		SECTID_C,
		SECTID_REQ_THREAD_B_FUNC01,
		SECTID_END,
	};

	n_sectid = p_if->get_sectid();
	THM_LOG_I ("n_sectid %d\n", n_sectid);

	switch (n_sectid) {
	case SECTID_ENTRY: {
		char *psz_msg = (char*)(p_if->src_info->msg.msg);
		THM_LOG_I ("msg:[%s]\n", psz_msg);

		n_sectid = SECTID_REQ_THREAD_B_FUNC00;
		en_act = EN_THM_ACT_CONTINUE;

		} break;

	case SECTID_REQ_THREAD_B_FUNC00:

		THM_LOG_I ("timeout 7sec\n");
		p_if->set_timeout (7000);

		// スレッドBのfunc00にリクエスト
		req_func00_thread_b(NULL);

		n_sectid = SECTID_WAIT_THREAD_B_FUNC00;
		en_act = EN_THM_ACT_WAIT;
		break;

	case SECTID_WAIT_THREAD_B_FUNC00: {
		EN_THM_RSLT result = p_if->src_info->result;
		switch ((int)result) {
		case EN_THM_RSLT_SUCCESS:
			THM_LOG_I ("return success req_func00_thread_b [%d]; msg:[%s]\n", result, (char*)p_if->src_info->msg.msg);
			THM_LOG_I ("clear_timeout\n");
			p_if->clear_timeout();
			break;
		case EN_THM_RSLT_ERROR:
			THM_LOG_E ("return error req_func00_thread_b [%d] msg:[%s]\n", result, (char*)p_if->src_info->msg.msg);
			THM_LOG_I ("clear_timeout\n");
			p_if->clear_timeout();
			break;
		case EN_THM_RSLT_REQ_TIMEOUT:
			THM_LOG_E ("reqtimeout req_func00_thread_b");
			THM_LOG_I ("clear_timeout\n");
			p_if->clear_timeout();
			break;
		case EN_THM_RSLT_SEQ_TIMEOUT:
			THM_LOG_E ("seqtimeout req_func00_thread_b");
			break;
		}

		n_sectid = SECTID_B;
		en_act = EN_THM_ACT_CONTINUE;
		} break;

	case SECTID_B:
		n_sectid = SECTID_C;
		en_act = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_C:
		n_sectid = SECTID_REQ_THREAD_B_FUNC01;
		en_act = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_THREAD_B_FUNC01: {

		// スレッドBのfunc01にリクエスト 同期なのでここでリプライを待ちます
		func01_thread_b();
		EN_THM_RSLT result = p_if->src_info->result;
		THM_LOG_I ("func01_thread_b return [%d] msg:[%s]\n", result, (char*)p_if->src_info->msg.msg);

		n_sectid = SECTID_END;
		en_act = EN_THM_ACT_CONTINUE;
		} break;

	case SECTID_END:

		p_if->reply (EN_THM_RSLT_SUCCESS, NULL, 0);

		n_sectid = THM_SECT_ID_INIT;
		en_act = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}

	p_if->set_sectid (n_sectid, en_act);
}

static void func03 (threadmgr_if_t *p_if)
{
	uint8_t n_sectid;
	EN_THM_ACT en_act = EN_THM_ACT_INIT;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_1,
		SECTID_2,
		SECTID_3,
		SECTID_4,
		SECTID_END,
	};

	n_sectid = p_if->get_sectid();
	THM_LOG_I ("n_sectid %d\n", n_sectid);

	switch (n_sectid) {
	case SECTID_ENTRY: {
		p_if->enable_overwrite ();
		n_sectid = SECTID_1;
		en_act = EN_THM_ACT_CONTINUE;
		} break;

	case SECTID_1:
		// スレッドBのfunc00にリクエスト
		req_func00_thread_b (&gn_tmp_req_id);
		THM_LOG_I ("req_func00Thread_b reqid:[%d]\n", gn_tmp_req_id);
		n_sectid = SECTID_2;
		en_act = EN_THM_ACT_WAIT;
		break;

	case SECTID_2: {
		if (p_if->src_info->req_id != gn_tmp_req_id) {
			THM_LOG_W ("different reqid %d\n", p_if->src_info->req_id);
			n_sectid = SECTID_2;
			en_act = EN_THM_ACT_WAIT;

		} else {
			EN_THM_RSLT result = p_if->src_info->result;
			THM_LOG_I ("req_func00Thread_b return [%d] msg:[%s]\n", result, (char*)p_if->src_info->msg.msg);
			p_if->set_timeout (3000);
			n_sectid = SECTID_3;
			en_act = EN_THM_ACT_WAIT;
		}
		} break;

	case SECTID_3:
		n_sectid = SECTID_END;
		en_act = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_END:

		p_if->reply (EN_THM_RSLT_SUCCESS, NULL, 0);

		n_sectid = THM_SECT_ID_INIT;
		en_act = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}

	p_if->set_sectid (n_sectid, en_act);
}


// 以下公開用

void req_startup_thread_a (uint32_t *p_out_req_id)
{
	gp_if->request_async (EN_THREAD_A, EN_A_STARTUP, NULL, 0, p_out_req_id);
}

void req_func00_thread_a (char *psz_msg, uint32_t *p_out_req_id)
{
	gp_if->request_async (EN_THREAD_A, EN_A_FUNC_00, (uint8_t*)psz_msg, strlen(psz_msg), p_out_req_id);
}

void req_func01_thread_a (uint32_t *p_out_req_id)
{
	gp_if->request_async (EN_THREAD_A, EN_A_FUNC_01, NULL, 0, p_out_req_id);
}

void req_func02_thread_a (char *psz_msg, uint32_t *p_out_req_id)
{
	gp_if->request_async (EN_THREAD_A, EN_A_FUNC_02, (uint8_t*)psz_msg, strlen(psz_msg), p_out_req_id);
}

void req_func03_thread_a (uint32_t *p_out_req_id)
{
	gp_if->request_async (EN_THREAD_A, EN_A_FUNC_03, NULL, 0, p_out_req_id);
}
