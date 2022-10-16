#ifndef _MODULEA_EXTERN_H_
#define _MODULEA_EXTERN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrpp.h"
#include "modules.h"


class CModuleA_extern : public threadmgr::CThreadMgrExternalIf
{
public:
	enum class sequence : int {
		startup = 0,
		test_reqrep,
		test_reqrep_notify,
		test_lock,
		test_lock_intr,
		test_overwrite,
		test_destroy,
	};

	explicit CModuleA_extern (CThreadMgrExternalIf *p_if)
		: CThreadMgrExternalIf (p_if)
		, m_module_id(static_cast<int>(module::module_a))
	{
	}

	virtual ~CModuleA_extern (void) {
	};


	bool req_startup (void) {
		int sequence = static_cast<int>(sequence::startup);
		return request_async (m_module_id, sequence);
	};

	bool req_test_reqrep (void) {
		int sequence = static_cast<int>(sequence::test_reqrep);
		return request_async (m_module_id, sequence);
	};

	bool req_test_reqrep_notify (void) {
		int sequence = static_cast<int>(sequence::test_reqrep_notify);
		return request_async (m_module_id, sequence);
	};

	bool req_test_lock (bool is_enable_lock) {
		int sequence = static_cast<int>(sequence::test_lock);
		return request_async (m_module_id, sequence, reinterpret_cast<uint8_t*>(&is_enable_lock), sizeof(is_enable_lock));
	};

	bool req_test_lock_intr (void) {
		int sequence = static_cast<int>(sequence::test_lock_intr);
		return request_async (m_module_id, sequence);
	};

	bool req_test_overwrite (void) {
		int sequence = static_cast<int>(sequence::test_overwrite);
		return request_async (m_module_id, sequence);
	};

	bool req_test_destroy (void) {
		int sequence = static_cast<int>(sequence::test_destroy);
		return request_async (m_module_id, sequence);
	};

private:
	int m_module_id ;
};

#endif
