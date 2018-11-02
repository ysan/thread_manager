#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrExternalIf.h"


namespace ThreadManager {

CThreadMgrExternalIf::CThreadMgrExternalIf (ST_THM_EXTERNAL_IF *pExtIf)
{
	if (pExtIf) {
		mpExtIf = pExtIf;
	}
}

CThreadMgrExternalIf::~CThreadMgrExternalIf (void)
{
}


bool CThreadMgrExternalIf::requestSync (uint8_t nThreadIdx, uint8_t nSeqIdx)
{
	if (mpExtIf) {
		return mpExtIf->pfnRequestSync (nThreadIdx, nSeqIdx, NULL);
	} else {
		return false;
	}
}

bool CThreadMgrExternalIf::requestSync (uint8_t nThreadIdx, uint8_t nSeqIdx, uint8_t *pszMsg)
{
	if (mpExtIf) {
		return mpExtIf->pfnRequestSync (nThreadIdx, nSeqIdx, pszMsg);
	} else {
		return false;
	}
}

bool CThreadMgrExternalIf::requestAsync (uint8_t nThreadIdx, uint8_t nSeqIdx)
{
	if (mpExtIf) {
		return mpExtIf->pfnRequestAsync (nThreadIdx, nSeqIdx, NULL, NULL);
	} else {
		return false;
	}
}

bool CThreadMgrExternalIf::requestAsync (uint8_t nThreadIdx, uint8_t nSeqIdx, uint32_t *pnReqId)
{
	if (mpExtIf) {
		return mpExtIf->pfnRequestAsync (nThreadIdx, nSeqIdx, NULL, pnReqId);
	} else {
		return false;
	}
}

bool CThreadMgrExternalIf::requestAsync (uint8_t nThreadIdx, uint8_t nSeqIdx, uint8_t *pszMsg)
{
	if (mpExtIf) {
		return mpExtIf->pfnRequestAsync (nThreadIdx, nSeqIdx, pszMsg, NULL);
	} else {
		return false;
	}
}

bool CThreadMgrExternalIf::requestAsync (uint8_t nThreadIdx, uint8_t nSeqIdx, uint8_t *pszMsg, uint32_t *pnReqId)
{
	if (mpExtIf) {
		return mpExtIf->pfnRequestAsync (nThreadIdx, nSeqIdx, pszMsg, pnReqId);
	} else {
		return false;
	}
}

bool CThreadMgrExternalIf::createExternalCp (void)
{
	if (mpExtIf) {
		return mpExtIf->pfnCreateExternalCp ();
	} else {
		return false;
	}
}

void CThreadMgrExternalIf::destroyExternalCp (void)
{
	if (mpExtIf) {
		return mpExtIf->pfnDestroyExternalCp ();
	}
}

ST_THM_SRC_INFO* CThreadMgrExternalIf::receiveExternal (void)
{
	if (mpExtIf) {
		return mpExtIf->pfnReceiveExternal ();
	} else {
		return NULL;
	}
}

} // namespace ThreadManager
