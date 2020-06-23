#ifndef _MODULEA_H_
#define _MODULEA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrpp.h"

#include "ModuleA_extern.h"


using namespace ThreadManager;


class CModuleA : public CThreadMgrBase
{
public:
	CModuleA (std::string name, uint8_t nQueNum);
	virtual ~CModuleA (void);


private:
	void startUp (CThreadMgrIf *pIf);
	void func00 (CThreadMgrIf *pIf);
	void func01 (CThreadMgrIf *pIf);

	void onReceiveNotify (CThreadMgrIf *pIf);

	uint32_t mTmpReqId;
	uint8_t mClientId;

};

#endif
