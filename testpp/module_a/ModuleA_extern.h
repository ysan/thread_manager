#ifndef _MODULEA_EXTERN_H_
#define _MODULEA_EXTERN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrpp.h"
#include "modules.h"


using namespace ThreadManager;

class CModuleA_extern : public CThreadMgrExternalIf
{
public:
	enum {
		EN_SEQ_STARTUP = 0,
		EN_SEQ_FUNC00,
		EN_SEQ_FUNC01,
		EN_SEQ_FUNC02,
	};

	explicit CModuleA_extern (CThreadMgrExternalIf *pIf) : CThreadMgrExternalIf (pIf) {
	};

	virtual ~CModuleA_extern (void) {
	};


	bool reqStartUp (void) {
		return requestAsync (EN_MODULE_A, EN_SEQ_STARTUP);
	};

	bool reqFunc00 (void) {
		return requestAsync (EN_MODULE_A, EN_SEQ_FUNC00);
	};

};

#endif
