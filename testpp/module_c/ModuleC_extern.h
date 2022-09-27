#ifndef _MODULEC_EXTERN_H_
#define _MODULEC_EXTERN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrpp.h"
#include "modules.h"


class CModuleC_extern : public threadmgr::CThreadMgrExternalIf
{
public:
	enum class seq : int {
		startup = 0,
		test_reg_notify,
		test_unreg_notify,
		test_notify,
		test_echo00,
		test_echo01,
		test_echo02,

		max
	};

	enum class notify_cat : int {
		cat_1 = 0,
		cat_2,
		cat_3,
	};

	explicit CModuleC_extern (CThreadMgrExternalIf *p_if)
		: CThreadMgrExternalIf (p_if)
		, m_module_id(static_cast<int>(module::module_c))
	{
	}

	virtual ~CModuleC_extern (void) {
	};


	bool req_startup (void) {
		int seq = static_cast<int>(seq::startup);
		return request_async (m_module_id, seq);
	};

	bool req_test_reg_notify (void) {
		int seq = static_cast<int>(seq::test_reg_notify);
		return request_async (m_module_id, seq);
	};

	bool req_test_unreg_notify (uint8_t client_id) {
		int seq = static_cast<int>(seq::test_unreg_notify);
		return request_async (m_module_id, seq, &client_id, sizeof(uint8_t));
	};

	bool req_echo00 (uint32_t *pOutReqId=NULL) {
		int seq = static_cast<int>(seq::test_echo00);
		return request_async (m_module_id, seq, pOutReqId);
	};

	bool req_echo00_sync (void) {
		int seq = static_cast<int>(seq::test_echo00);
		return request_sync (m_module_id, seq);
	};

	bool req_echo01 (uint32_t *pOutReqId=NULL) {
		int seq = static_cast<int>(seq::test_echo01);
		return request_async (m_module_id, seq, pOutReqId);
	};

	bool req_echo01_sync (void) {
		int seq = static_cast<int>(seq::test_echo01);
		return request_sync (m_module_id, seq);
	};

	bool req_echo02 (uint32_t *pOutReqId=NULL) {
		int seq = static_cast<int>(seq::test_echo02);
		return request_async (m_module_id, seq, pOutReqId);
	};

	bool req_echo02_sync (void) {
		int seq = static_cast<int>(seq::test_echo02);
		return request_sync (m_module_id, seq);
	};

private:
	int m_module_id ;
};

#endif
