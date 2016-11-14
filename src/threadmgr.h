#ifndef _THREADMGR_H_
#define _THREADMGR_H_

#include <stdbool.h>

#include "threadmgr_if.h"


/*
 * Constant define
 */


/*
 * Type define
 */


/*
 * External
 */
bool setup (const ST_THM_REG_TBL *pTbl, uint8_t nTblMax);
bool requestSync (uint8_t nThreadIdx, uint8_t nSeqIdx, uint8_t *pszMsg);
bool requestAsync (uint8_t nThreadIdx, uint8_t nSeqIdx, uint8_t *pszMsg, uint32_t *pReqId);
bool createExternalCp (void);
void destroyExternalCp (void);
ST_THM_SRC_INFO *receiveExternal (void);
void finalize (void);

#endif
