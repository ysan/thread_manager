#ifndef _MODULEA_EXTERN_H_
#define _MODULEA_EXTERN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ThreadMgrpp.h"
#include "modules.h"


class CModuleA_extern : public ThreadManager::CThreadMgrExternalIf
{
public:
	enum {
		EN_SEQ_STARTUP = 0,
		EN_SEQ_TEST_REQREP,
		EN_SEQ_TEST_REQREPNOTIFY,
		EN_SEQ_TEST_LOCK,
		EN_SEQ_TEST_LOCK_INTR,
		EN_SEQ_TEST_OVERWRITE,
		EN_SEQ_TEST_DESTROY,
	};

	explicit CModuleA_extern (CThreadMgrExternalIf *pIf) : CThreadMgrExternalIf (pIf) {
	};

	virtual ~CModuleA_extern (void) {
	};


	bool reqStartUp (void) {
		return requestAsync (EN_MODULE_A, EN_SEQ_STARTUP);
	};

	bool reqTestReqRep (void) {
		return requestAsync (EN_MODULE_A, EN_SEQ_TEST_REQREP);
	};

	bool reqTestReqRepNotify (void) {
		return requestAsync (EN_MODULE_A, EN_SEQ_TEST_REQREPNOTIFY);
	};

	bool reqTestLock (bool is_enable_lock) {
		return requestAsync (EN_MODULE_A, EN_SEQ_TEST_LOCK, (uint8_t*)&is_enable_lock, sizeof(is_enable_lock));
	};

	bool reqTestLockIntr (void) {
		return requestAsync (EN_MODULE_A, EN_SEQ_TEST_LOCK_INTR);
	};

	bool reqTestOverwrite (void) {
		return requestAsync (EN_MODULE_A, EN_SEQ_TEST_OVERWRITE);
	};

	bool reqTestDestroy (void) {
		return requestAsync (EN_MODULE_A, EN_SEQ_TEST_DESTROY);
	};

};

#endif
