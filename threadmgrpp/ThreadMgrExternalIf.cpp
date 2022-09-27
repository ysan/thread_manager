#include "ThreadMgrExternalIf.h"

namespace threadmgr {

CThreadMgrExternalIf::CThreadMgrExternalIf (ST_THM_EXTERNAL_IF *p_ext_if)
	:mp_ext_if (NULL)
{
	if (p_ext_if) {
		mp_ext_if = p_ext_if;
	}
}

CThreadMgrExternalIf::CThreadMgrExternalIf (CThreadMgrExternalIf *p_ext_if)
	:mp_ext_if (NULL)
{
	if (p_ext_if) {
		if (p_ext_if->mp_ext_if) {
			mp_ext_if = p_ext_if->mp_ext_if;
		}
	}
}

CThreadMgrExternalIf::~CThreadMgrExternalIf (void)
{
}


bool CThreadMgrExternalIf::request_sync (uint8_t thread_idx, uint8_t seq_idx)
{
	if (mp_ext_if) {
		return mp_ext_if->pfnRequestSync (thread_idx, seq_idx, NULL, 0);
	} else {
		return false;
	}
}

bool CThreadMgrExternalIf::request_sync (uint8_t thread_idx, uint8_t seq_idx, uint8_t *pszMsg, size_t msgSize)
{
	if (mp_ext_if) {
		return mp_ext_if->pfnRequestSync (thread_idx, seq_idx, pszMsg, msgSize);
	} else {
		return false;
	}
}

bool CThreadMgrExternalIf::request_async (uint8_t thread_idx, uint8_t seq_idx)
{
	if (mp_ext_if) {
		return mp_ext_if->pfnRequestAsync (thread_idx, seq_idx, NULL, 0, NULL);
	} else {
		return false;
	}
}

bool CThreadMgrExternalIf::request_async (uint8_t thread_idx, uint8_t seq_idx, uint32_t *pOutReqId)
{
	if (mp_ext_if) {
		return mp_ext_if->pfnRequestAsync (thread_idx, seq_idx, NULL, 0, pOutReqId);
	} else {
		return false;
	}
}

bool CThreadMgrExternalIf::request_async (uint8_t thread_idx, uint8_t seq_idx, uint8_t *pszMsg, size_t msgSize)
{
	if (mp_ext_if) {
		return mp_ext_if->pfnRequestAsync (thread_idx, seq_idx, pszMsg, msgSize, NULL);
	} else {
		return false;
	}
}

bool CThreadMgrExternalIf::request_async (uint8_t thread_idx, uint8_t seq_idx, uint8_t *pszMsg, size_t msgSize, uint32_t *pOutReqId)
{
	if (mp_ext_if) {
		return mp_ext_if->pfnRequestAsync (thread_idx, seq_idx, pszMsg, msgSize, pOutReqId);
	} else {
		return false;
	}
}

void CThreadMgrExternalIf::set_request_option (uint32_t option)
{
	if (mp_ext_if) {
		mp_ext_if->pfnSetRequestOption (option);
	}
}

uint32_t CThreadMgrExternalIf::get_request_option (void) const
{
	if (mp_ext_if) {
		return mp_ext_if->pfnGetRequestOption ();
	} else {
		return 0;
	}
}

bool CThreadMgrExternalIf::create_external_cp (void)
{
	if (mp_ext_if) {
		return mp_ext_if->pfnCreateExternalCp ();
	} else {
		return false;
	}
}

void CThreadMgrExternalIf::destroy_external_cp (void)
{
	if (mp_ext_if) {
		return mp_ext_if->pfnDestroyExternalCp ();
	}
}

ST_THM_SRC_INFO* CThreadMgrExternalIf::receive_external (void)
{
	if (mp_ext_if) {
		return mp_ext_if->pfnReceiveExternal ();
	} else {
		return NULL;
	}
}

} // namespace threadmgr
