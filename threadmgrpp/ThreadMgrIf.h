#ifndef _THREADMGR_IF_HH_
#define _THREADMGR_IF_HH_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "threadmgr_if.h"


namespace threadmgr {

class CThreadMgrIf 
{
public:
	explicit CThreadMgrIf (ST_THM_IF *p_if);
	explicit CThreadMgrIf (CThreadMgrIf *p_if);
	virtual ~CThreadMgrIf (void);


	ST_THM_SRC_INFO * get_source (void) const;

	bool reply (EN_THM_RSLT rslt);
	bool reply (EN_THM_RSLT rslt, uint8_t *msg, size_t msg_size);

	bool reg_notify (uint8_t category, uint8_t *p_client_id);
	bool unreg_notify (uint8_t category, uint8_t client_id);

	bool notify (uint8_t category);
	bool notify (uint8_t category, uint8_t *msg, size_t msg_size);

	void set_sect_id (uint8_t sect_id, EN_THM_ACT act);
	uint8_t get_sect_id (void) const;

	void set_timeout (uint32_t timeout_msec);
	void clear_timeout (void);

	void enable_overwrite (void);
	void disable_overwrite (void);

	void lock (void);
	void unlock (void);

	uint8_t get_seq_idx (void) const;
	const char* get_seq_name (void) const;


private:
	ST_THM_IF *mp_if;
};

} // namespace threadmgr

#endif
