#include "wtk_nk_event.h"

int wtk_nk_event_send_back(wtk_nk_event_t *nke)
{
    //wtk_debug("%.0f send back event ...\n",time_get_ms());
	return wtk_pipequeue_push(nke->pipe_queue,&(nke->pipe_n));
}
