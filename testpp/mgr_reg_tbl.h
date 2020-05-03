#ifndef _MGR_REG_TBL_H_
#define _MGR_REG_TBL_H_


#include "ThreadMgrpp.h"


using namespace ThreadManager;

enum {
	EN_MODULE_A = 0,
	EN_MODULE_B,
	EN_MODULE_C,

	EN_MODULE_NUM
};


extern CThreadMgrBase *gp_threads [];


#endif
