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
		EN_SEQ_STARTUP = 0,
		EN_SEQ_REG_NOTIFY,
		EN_SEQ_UNREG_NOTIFY,
		EN_SEQ_CYCLE_FUNC,
		EN_SEQ_FUNC00,
	};

	enum {
		EN_NOTIFY_CAT_1 = 0,
		EN_NOTIFY_CAT_2,
		EN_NOTIFY_CAT_3,
	};

	explicit CModuleC_extern (CThreadMgrExternalIf *pIf) : CThreadMgrExternalIf (pIf) {
	};

	virtual ~CModuleC_extern (void) {
	};


	bool reqStartUp (void) {
		return requestAsync (EN_MODULE_C, EN_SEQ_STARTUP);
	};

	bool reqRegNotify (void) {
		return requestAsync (EN_MODULE_C, EN_SEQ_REG_NOTIFY);
	};

	bool reqUnregNotify (void) {
		return requestAsync (EN_MODULE_C, EN_SEQ_UNREG_NOTIFY);
	};

	bool reqFunc00 (void) {
		return requestAsync (EN_MODULE_C, EN_SEQ_FUNC00);
	};

};

#endif
