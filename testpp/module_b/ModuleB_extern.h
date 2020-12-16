#ifndef _MODULEB_EXTERN_H_
#define _MODULEB_EXTERN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrpp.h"
#include "modules.h"


class CModuleB_extern : public ThreadManager::CThreadMgrExternalIf
{
public:
	enum {
		EN_SEQ_STARTUP = 0,
		EN_SEQ_ECHO00,
		EN_SEQ_ECHO01,
		EN_SEQ_ECHO02,
	};

	explicit CModuleB_extern (CThreadMgrExternalIf *pIf) : CThreadMgrExternalIf (pIf) {
	};

	virtual ~CModuleB_extern (void) {
	};


	bool reqStartUp (void) {
		return requestAsync (EN_MODULE_B, EN_SEQ_STARTUP);
	};

	bool reqEcho00 (uint32_t *pOutReqId=NULL) {
		return requestAsync (EN_MODULE_B, EN_SEQ_ECHO00, pOutReqId);
	};

	bool reqEcho00sync (void) {
		return requestSync (EN_MODULE_B, EN_SEQ_ECHO00);
	};

	bool reqEcho01 (uint32_t *pOutReqId=NULL) {
		return requestAsync (EN_MODULE_B, EN_SEQ_ECHO01, pOutReqId);
	};

	bool reqEcho01sync (void) {
		return requestSync (EN_MODULE_B, EN_SEQ_ECHO01);
	};

	bool reqEcho02 (uint32_t *pOutReqId=NULL) {
		return requestAsync (EN_MODULE_B, EN_SEQ_ECHO02, pOutReqId);
	};

	bool reqEcho02sync (void) {
		return requestSync (EN_MODULE_B, EN_SEQ_ECHO02);
	};

};

#endif
