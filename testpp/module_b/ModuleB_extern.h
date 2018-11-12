#ifndef _MODULEB_EXTERN_H_
#define _MODULEB_EXTERN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrExternalIf.h"
#include "mgr_reg_tbl.h"


using namespace ThreadManager;

enum {
	EN_SEQ_MODULE_B_STARTUP = 0,

	EN_SEQ_MODULE_B_NUM,
};


class CModuleB_extern : public CThreadMgrExternalIf
{
public:
	explicit CModuleB_extern (CThreadMgrExternalIf *pIf) : CThreadMgrExternalIf (pIf) {
	};

	virtual ~CModuleB_extern (void) {
	};


	bool reqStartUp (void) {
		return requestAsync (EN_MODULE_B, EN_SEQ_MODULE_B_STARTUP);
	};

private:

};

#endif
