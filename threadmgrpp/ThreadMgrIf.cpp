#include "ThreadMgrIf.h"


namespace threadmgr {

CThreadMgrIf::CThreadMgrIf (ST_THM_IF *p_if)
	:mp_if (NULL)
{
	if (p_if) {
		mp_if = p_if;
	}
}

CThreadMgrIf::CThreadMgrIf (CThreadMgrIf *p_if)
	:mp_if (NULL)
{
	if (p_if) {
		if (p_if->mp_if) {
			mp_if = p_if->mp_if;
		}
	}
}

CThreadMgrIf::~CThreadMgrIf (void)
{
}


ST_THM_SRC_INFO * CThreadMgrIf::get_source (void) const
{
	if (mp_if) {
		return mp_if->pstSrcInfo;
	} else {
		return NULL;
	}
}

bool CThreadMgrIf::reply (EN_THM_RSLT rslt)
{
	if (mp_if) {
		return mp_if->pfnReply (rslt, NULL, 0);
	} else {
		return false;
	}
}

bool CThreadMgrIf::reply (EN_THM_RSLT rslt, uint8_t *msg, size_t msg_size)
{
	if (mp_if) {
		return mp_if->pfnReply (rslt, msg, msg_size);
	} else {
		return false;
	}
}

bool CThreadMgrIf::reg_notify (uint8_t nCategory, uint8_t *pnClientId)
{
	if (mp_if) {
		return mp_if->pfnRegNotify (nCategory, pnClientId);
	} else {
		return false;
	}
}

bool CThreadMgrIf::unreg_notify (uint8_t nCategory, uint8_t nClientId)
{
	if (mp_if) {
		return mp_if->pfnUnRegNotify (nCategory, nClientId);
	} else {
		return false;
	}
}

bool CThreadMgrIf::notify (uint8_t nCategory)
{
	if (mp_if) {
		return mp_if->pfnNotify (nCategory, NULL, 0);
	} else {
		return false;
	}
}

bool CThreadMgrIf::notify (uint8_t nCategory, uint8_t *msg, size_t msg_size)
{
	if (mp_if) {
		return mp_if->pfnNotify (nCategory, msg, msg_size);
	} else {
		return false;
	}
}

void CThreadMgrIf::set_sect_id (uint8_t nSectId, EN_THM_ACT enAct)
{
	if (mp_if) {
		mp_if->pfnSetSectId (nSectId, enAct);
	}
}

uint8_t CThreadMgrIf::get_sect_id (void) const
{
	if (mp_if) {
		return mp_if->pfnGetSectId ();
	} else {
		return THM_SECT_ID_INIT;
	}
}

void CThreadMgrIf::set_timeout (uint32_t timeout_msec)
{
	if (mp_if) {
		mp_if->pfnSetTimeout (timeout_msec);
	}
}

void CThreadMgrIf::clear_timeout (void)
{
	if (mp_if) {
		mp_if->pfnClearTimeout ();
	}
}

void CThreadMgrIf::enable_overwrite (void)
{
	if (mp_if) {
		mp_if->pfnEnableOverwrite ();
	}
}

void CThreadMgrIf::disable_overwrite (void)
{
	if (mp_if) {
		mp_if->pfnDisableOverwrite ();
	}
}

void CThreadMgrIf::lock (void)
{
	if (mp_if) {
		mp_if->pfnLock ();
	}
}

void CThreadMgrIf::unlock (void)
{
	if (mp_if) {
		mp_if->pfnUnlock ();
	}
}

uint8_t CThreadMgrIf::get_seq_idx (void) const
{
	if (mp_if) {
		return mp_if->pfnGetSeqIdx ();
	} else {
//TODO SEQ_IDX_BLANK
		return 0x80;
	}
}

const char* CThreadMgrIf::get_seq_name (void) const
{
	if (mp_if) {
		return mp_if->pfnGetSeqName ();
	} else {
		return NULL;
	}
}

} // namespace threadmgr
