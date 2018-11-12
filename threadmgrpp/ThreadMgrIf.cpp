#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrIf.h"


namespace ThreadManager {

CThreadMgrIf::CThreadMgrIf (ST_THM_IF *pIf)
{
	if (pIf) {
		mpIf = pIf;
	}
}

CThreadMgrIf::CThreadMgrIf (CThreadMgrIf *pIf)
{
	if (pIf) {
		if (pIf->mpIf) {
			mpIf = pIf->mpIf;
		}
	}
}

CThreadMgrIf::~CThreadMgrIf (void)
{
}


ST_THM_SRC_INFO * CThreadMgrIf::getSrcInfo (void)
{
	if (mpIf) {
		return mpIf->pstSrcInfo;
	} else {
		return NULL;
	}
}

bool CThreadMgrIf::reply (EN_THM_RSLT enRslt)
{
	if (mpIf) {
		return mpIf->pfnReply (enRslt, NULL);
	} else {
		return false;
	}
}

bool CThreadMgrIf::reply (EN_THM_RSLT enRslt, uint8_t *pszMsg)
{
	if (mpIf) {
		return mpIf->pfnReply (enRslt, pszMsg);
	} else {
		return false;
	}
}

bool CThreadMgrIf::regNotify (uint8_t *pnClientId)
{
	if (mpIf) {
		return mpIf->pfnRegNotify (pnClientId);
	} else {
		return false;
	}
}

bool CThreadMgrIf::unregNotify (uint8_t nClientId)
{
	if (mpIf) {
		return mpIf->pfnUnRegNotify (nClientId);
	} else {
		return false;
	}
}

bool CThreadMgrIf::notify (uint8_t nClientId)
{
	if (mpIf) {
		return mpIf->pfnNotify (nClientId, NULL);
	} else {
		return false;
	}
}

bool CThreadMgrIf::notify (uint8_t nClientId, uint8_t *pszMsg)
{
	if (mpIf) {
		return mpIf->pfnNotify (nClientId, pszMsg);
	} else {
		return false;
	}
}

void CThreadMgrIf::setSectId (uint8_t nSectId, EN_THM_ACT enAct)
{
	if (mpIf) {
		mpIf->pfnSetSectId (nSectId, enAct);
	}
}

uint8_t CThreadMgrIf::getSectId (void)
{
	if (mpIf) {
		return mpIf->pfnGetSectId ();
	}

	return THM_SECT_ID_INIT;
}

void CThreadMgrIf::setTimeout (uint32_t nTimeoutMsec)
{
	if (mpIf) {
		mpIf->pfnSetTimeout (nTimeoutMsec);
	}
}

void CThreadMgrIf::clearTimeout (void)
{
	if (mpIf) {
		mpIf->pfnClearTimeout ();
	}
}

void CThreadMgrIf::enableOverwrite (void)
{
	if (mpIf) {
		mpIf->pfnEnableOverwrite ();
	}
}

void CThreadMgrIf::disableOverwrite (void)
{
	if (mpIf) {
		mpIf->pfnDisableOverwrite ();
	}
}

} // namespace ThreadManager
