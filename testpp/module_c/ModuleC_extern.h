#ifndef _MODULEC_EXTERN_H_
#define _MODULEC_EXTERN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrpp.h"
#include "modules.h"


class CModuleC_extern : public ThreadManager::CThreadMgrExternalIf
{
public:
	enum {
		EN_SEQ_STARTUP = 0,
		EN_SEQ_TEST_REG_NOTIFY,
		EN_SEQ_TEST_UNREG_NOTIFY,
		EN_SEQ_TEST_NOTIFY,
		EN_SEQ_ECHO00,
		EN_SEQ_ECHO01,
		EN_SEQ_ECHO02,
		EN_SEQ_NUM,
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

	bool reqTestRegNotify (void) {
		return requestAsync (EN_MODULE_C, EN_SEQ_TEST_REG_NOTIFY);
	};

	bool reqTestUnregNotify (uint8_t client_id) {
		return requestAsync (EN_MODULE_C, EN_SEQ_TEST_UNREG_NOTIFY, &client_id, sizeof(uint8_t));
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
