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
	CModuleB (char *pszName, uint8_t nQueNum);
	virtual ~CModuleB (void);


	void startUp (CThreadMgrIf *pIf) ;


private:
	ST_SEQ_BASE mSeqs [EN_SEQ_MODULE_B_NUM]; // entity

};

#endif
