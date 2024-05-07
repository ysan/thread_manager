#ifndef _MODULE_C_EXTERN_H_
#define _MODULE_C_EXTERN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrpp.h"
#include "modules.h"


class module_c_extern : public threadmgr::CThreadMgrExternalIf
{
public:
	enum class sequence : int {
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

	explicit module_c_extern (CThreadMgrExternalIf *p_if)
		: CThreadMgrExternalIf (p_if)
		, m_module_id(static_cast<int>(module::module_c))
	{
	}

	virtual ~module_c_extern (void) {
	};


	bool req_startup_sync (void) {
		int sequence = static_cast<int>(sequence::startup);
		return request_sync (m_module_id, sequence);
	};

	bool req_test_reg_notify (void) {
		int sequence = static_cast<int>(sequence::test_reg_notify);
		return request_async (m_module_id, sequence);
	};

	bool req_test_unreg_notify (uint8_t client_id) {
		int sequence = static_cast<int>(sequence::test_unreg_notify);
		return request_async (m_module_id, sequence, &client_id, sizeof(uint8_t));
	};

	bool req_echo00 (uint32_t *p_out_req_id=NULL) {
		int sequence = static_cast<int>(sequence::test_echo00);
		return request_async (m_module_id, sequence, p_out_req_id);
	};

	bool req_echo00_sync (void) {
		int sequence = static_cast<int>(sequence::test_echo00);
		return request_sync (m_module_id, sequence);
	};

	bool req_echo01 (uint32_t *p_out_req_id=NULL) {
		int sequence = static_cast<int>(sequence::test_echo01);
		return request_async (m_module_id, sequence, p_out_req_id);
	};

	bool req_echo01_sync (void) {
		int sequence = static_cast<int>(sequence::test_echo01);
		return request_sync (m_module_id, sequence);
	};

	bool req_echo02 (uint32_t *p_out_req_id=NULL) {
		int sequence = static_cast<int>(sequence::test_echo02);
		return request_async (m_module_id, sequence, p_out_req_id);
	};

	bool req_echo02_sync (void) {
		int sequence = static_cast<int>(sequence::test_echo02);
		return request_sync (m_module_id, sequence);
	};

private:
	int m_module_id ;
};

#endif
