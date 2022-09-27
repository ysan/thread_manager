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
	enum class seq : int {
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
		int seq = static_cast<int>(seq::startup);
		return request_async (m_module_id, seq);
	};

	bool req_test_reqrep (void) {
		int seq = static_cast<int>(seq::test_reqrep);
		return request_async (m_module_id, seq);
	};

	bool req_test_reqrep_notify (void) {
		int seq = static_cast<int>(seq::test_reqrep_notify);
		return request_async (m_module_id, seq);
	};

	bool req_test_lock (bool is_enable_lock) {
		int seq = static_cast<int>(seq::test_lock);
		return request_async (m_module_id, seq, (uint8_t*)&is_enable_lock, sizeof(is_enable_lock));
	};

	bool req_test_lock_intr (void) {
		int seq = static_cast<int>(seq::test_lock_intr);
		return request_async (m_module_id, seq);
	};

	bool req_test_overwrite (void) {
		int seq = static_cast<int>(seq::test_overwrite);
		return request_async (m_module_id, seq);
	};

	bool req_test_destroy (void) {
		int seq = static_cast<int>(seq::test_destroy);
		return request_async (m_module_id, seq);
	};

private:
	int m_module_id ;
};

#endif
