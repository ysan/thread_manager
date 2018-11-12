#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "ModuleA.h"
#include "ModuleB.h"
#include "ModuleC.h"

#include "mgr_reg_tbl.h"


CModuleA g_moduleA ((char*)"ModuleA", 10);
CModuleB g_moduleB ((char*)"ModuleB", 10);
CModuleC g_moduleC ((char*)"ModuleC", 10);


CThreadMgrBase *gp_threads [] = {
	&g_moduleA,
	&g_moduleB,
	&g_moduleC,

};


