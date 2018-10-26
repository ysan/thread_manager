#ifndef _THREAD_MGR_IF_H_
#define _THREAD_MGR_IF_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "threadmgr_if.h"

using namespace std;


class CThreadMgrIf 
{
public:
	CThreadMgrIf (ST_THM_IF *pIf);
	virtual ~CThreadMgrIf (void);


	ST_THM_SRC_INFO * getSrcInfo (void);

	bool reply (EN_THM_RSLT enRslt, uint8_t *pszMsg);
	bool regNotify (uint8_t *pnClientId);
	bool unregNotify (uint8_t nClientId);
	bool notify (uint8_t nClientId, uint8_t *pszMsg);
	void setSectId (uint8_t nSectId, EN_THM_ACT enAct);
	uint8_t getSectId (void);
	void setTimeout (uint32_t nTimeoutMsec);
	void clearTimeout (void);
	void enableOverwrite (void);
	void disableOverwrite (void);


private:
	ST_THM_IF *mpIf;
};


#endif
