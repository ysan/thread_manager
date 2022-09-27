#ifndef _THREADMGR_EXTERNAL_IF_HH_
#define _THREADMGR_EXTERNAL_IF_HH_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "threadmgr_if.h"

namespace threadmgr {

class CThreadMgrExternalIf
{
public:
	explicit CThreadMgrExternalIf (ST_THM_EXTERNAL_IF *p_ext_if);
	explicit CThreadMgrExternalIf (CThreadMgrExternalIf *p_ext_if);
	virtual ~CThreadMgrExternalIf (void);


	bool request_sync (uint8_t thread_idx, uint8_t seq_idx);
	bool request_sync (uint8_t thread_idx, uint8_t seq_idx, uint8_t *msg, size_t msg_size);

	bool request_async (uint8_t thread_idx, uint8_t seq_idx);
	bool request_async (uint8_t thread_idx, uint8_t seq_idx, uint32_t *p_out_req_id);
	bool request_async (uint8_t thread_idx, uint8_t seq_idx, uint8_t *pMsg, size_t msg_size);
	bool request_async (uint8_t thread_idx, uint8_t seq_idx, uint8_t *pMsg, size_t msg_s, uint32_t *p_out_req_id);

	void set_request_option (uint32_t option);
	uint32_t get_request_option (void) const ;

	bool create_external_cp (void);
	void destroy_external_cp (void);

	ST_THM_SRC_INFO* receive_external (void);


private:
	ST_THM_EXTERNAL_IF *mp_ext_if;

};

} // namespace threadmgr

#endif
