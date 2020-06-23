#ifndef _MODULEB_H_
#define _MODULEB_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrpp.h"

#include "ModuleB_extern.h"


using namespace ThreadManager;


class CModuleB : public CThreadMgrBase
{
public:
	CModuleB (std::string name, uint8_t nQueNum);
	virtual ~CModuleB (void);

private:
	void startUp (CThreadMgrIf *pIf);
	void func00 (CThreadMgrIf *pIf);

	uint8_t mClientId;
};

#endif
