#ifndef _MODULEB_EXTERN_H_
#define _MODULEB_EXTERN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrpp.h"
#include "modules.h"


using namespace ThreadManager;

class CModuleB_extern : public CThreadMgrExternalIf
{
public:
	enum {
		EN_SEQ_MODULE_B_STARTUP = 0,
		EN_SEQ_MODULE_B_FUNC00,
	};

	explicit CModuleB_extern (CThreadMgrExternalIf *pIf) : CThreadMgrExternalIf (pIf) {
	};

	virtual ~CModuleB_extern (void) {
	};


	bool reqStartUp (void) {
		return requestAsync (EN_MODULE_B, EN_SEQ_MODULE_B_STARTUP);
	};

	bool syncReqFunc00 (void) {
		return requestSync (EN_MODULE_B, EN_SEQ_MODULE_B_FUNC00);
	};

	bool reqFunc00 (uint32_t *pOutReqId=NULL) {
		return requestAsync (EN_MODULE_B, EN_SEQ_MODULE_B_FUNC00, pOutReqId);
	};

};

#endif
