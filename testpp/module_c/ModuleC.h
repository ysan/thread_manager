#ifndef _MODULEC_H_
#define _MODULEC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrpp.h"

#include "ModuleC_extern.h"


using namespace ThreadManager;



class CModuleC : public CThreadMgrBase
{
public:
	CModuleC (char *pszName, uint8_t nQueNum);
	virtual ~CModuleC (void);


	void startUp (CThreadMgrIf *pIf) ;


private:
	ST_SEQ_BASE mSeqs [EN_SEQ_MODULE_C_NUM]; // entity

};

#endif
