#ifndef _MODULEA_EXTERN_H_
#define _MODULEA_EXTERN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "mgr_reg_tbl.h"


using namespace ThreadManager;

enum {
	EN_SEQ_STARTUP = 0,
	EN_SEQ_FUNC00,
	EN_SEQ_FUNC01,

	EN_SEQ_NUM,
};


class CModuleA_extern : public CThreadMgrExternalIf
{
public:
	explicit CModuleA_extern (CThreadMgrExternalIf *pIf) : CThreadMgrExternalIf (pIf) {
	};

	virtual ~CModuleA_extern (void) {
	};


	bool reqStartUp (void) {
		return requestAsync (EN_MODULE_A, EN_SEQ_STARTUP);
	};

	bool reqFunc00 (const char *pMsg, size_t len) {
		return requestAsync (EN_MODULE_A, EN_SEQ_FUNC00, (uint8_t*)pMsg, len);
	};

	bool reqFunc01 (const char *pMsg, size_t len) {
		return requestAsync (EN_MODULE_A, EN_SEQ_FUNC01, (uint8_t*)pMsg, len);
	};


private:

};

#endif
