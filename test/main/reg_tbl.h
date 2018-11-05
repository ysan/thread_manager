#ifndef _REG_THREADMGR_TBL_H_
#define _REG_THREADMGR_TBL_H_


/*
 * Constant define
 */
enum {
	EN_THREAD_A = 0,
	EN_THREAD_B,
	EN_THREAD_C,
	EN_THREAD_MAX
};

/*
 * External
 */
extern const ST_THM_REG_TBL gstRegThreadMgrTbl [EN_THREAD_MAX];


#endif
