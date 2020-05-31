#ifndef _MODULEC_EXTERN_H_
#define _MODULEC_EXTERN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrpp.h"
#include "modules.h"


using namespace ThreadManager;

class CModuleC_extern : public CThreadMgrExternalIf
{
public:
	enum {
		EN_SEQ_MODULE_C_STARTUP = 0,
	};

	explicit CModuleC_extern (CThreadMgrExternalIf *pIf) : CThreadMgrExternalIf (pIf) {
	};

	virtual ~CModuleC_extern (void) {
	};


	bool reqStartUp (void) {
		return requestAsync (EN_MODULE_C, EN_SEQ_MODULE_C_STARTUP);
	};

private:

};

#endif
