#ifndef _MODULEA_H_
#define _MODULEA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "threadmgr_if.h"
#include "threadmgr_util.h"

#include "ThreadMgrIf.h"
#include "ThreadMgrExternalIf.h"

#include "ThreadMgrBase.h"
#include "ThreadMgr.h"

#include "ModuleA_extern.h"


using namespace ThreadManager;


class CModuleA : public CThreadMgrBase
{
public:
	CModuleA (char *pszName, uint8_t nQueNum);
	virtual ~CModuleA (void);


	void startUp (CThreadMgrIf *pIf);
	void func00 (CThreadMgrIf *pIf);
	void func01 (CThreadMgrIf *pIf);


private:
	ST_SEQ_BASE mSeqs [EN_SEQ_NUM]; // entity

	uint32_t mTmpReqId;

};

#endif