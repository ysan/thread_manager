#ifndef _MODULE_B_EXTERN_H_
#define _MODULE_B_EXTERN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrpp.h"
#include "modules.h"


class module_b_extern : public threadmgr::CThreadMgrExternalIf
{
public:
	enum class sequence : int {
		startup = 0,
		echo00,
		echo01,
		echo02,
	};

	explicit module_b_extern (CThreadMgrExternalIf *p_if)
		: CThreadMgrExternalIf (p_if)
		, m_module_id (static_cast<int>(module::module_b))
	{
	};

	virtual ~module_b_extern (void) {
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

	bool req_echo01 (std::string msg) {
		int sequence = static_cast<int>(sequence::echo01);
		if (msg.length() == 0) {
			return request_async (m_module_id, sequence);
		} else {
			char *p = const_cast<char*>(msg.c_str());
			return request_async (m_module_id, sequence, reinterpret_cast<uint8_t*>(p), msg.length());
		}
	};

	bool req_echo01_sync (void) {
		int sequence = static_cast<int>(sequence::echo01);
		return request_sync (m_module_id, sequence);
	};

	bool req_echo02 (uint32_t *out_req_id=NULL) {
		int sequence = static_cast<int>(sequence::echo02);
		return request_async (m_module_id, sequence, out_req_id);
	};

	bool req_echo02 (std::string msg) {
		int sequence = static_cast<int>(sequence::echo02);
		if (msg.length() == 0) {
			return request_async (m_module_id, sequence);
		} else {
			char *p = const_cast<char*>(msg.c_str());
			return request_async (m_module_id, sequence, reinterpret_cast<uint8_t*>(p), msg.length());
		}
	};

	bool req_echo02_sync (std::string msg="") {
		int sequence = static_cast<int>(sequence::echo02);
		if (msg.length() == 0) {
			return request_sync (m_module_id, sequence);
		} else {
			char *p = const_cast<char*>(msg.c_str());
			return request_sync (m_module_id, sequence, reinterpret_cast<uint8_t*>(p), msg.length());
		}
	};

private:
	int m_module_id;
};

#endif
