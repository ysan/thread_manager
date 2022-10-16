#ifndef _MODULEB_EXTERN_H_
#define _MODULEB_EXTERN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrpp.h"
#include "modules.h"


class CModuleB_extern : public threadmgr::CThreadMgrExternalIf
{
public:
	enum class sequence : int {
		startup = 0,
		echo00,
		echo01,
		echo02,
	};

	explicit CModuleB_extern (CThreadMgrExternalIf *p_if)
		: CThreadMgrExternalIf (p_if)
		, m_module_id (static_cast<int>(module::module_b))
	{
	};

	virtual ~CModuleB_extern (void) {
	};


	bool req_startup (void) {
		int sequence = static_cast<int>(sequence::startup);
		return request_async (m_module_id, sequence);
	};

	bool req_echo00 (uint32_t *out_req_id=NULL) {
		int sequence = static_cast<int>(sequence::echo00);
		return request_async (m_module_id, sequence, out_req_id);
	};

	bool req_echo00_sync (void) {
		int sequence = static_cast<int>(sequence::echo00);
		return request_sync (m_module_id, sequence);
	};

	bool req_echo01 (uint32_t *out_req_id=NULL) {
		int sequence = static_cast<int>(sequence::echo01);
		return request_async (m_module_id, sequence, out_req_id);
	};

	bool req_echo01_sync (void) {
		int sequence = static_cast<int>(sequence::echo01);
		return request_sync (m_module_id, sequence);
	};

	bool req_echo02 (uint32_t *out_req_id=NULL) {
		int sequence = static_cast<int>(sequence::echo02);
		return request_async (m_module_id, sequence, out_req_id);
	};

	bool req_echo02_sync (void) {
		int sequence = static_cast<int>(sequence::echo02);
		return request_sync (m_module_id, sequence);
	};

private:
	int m_module_id;
};

#endif
