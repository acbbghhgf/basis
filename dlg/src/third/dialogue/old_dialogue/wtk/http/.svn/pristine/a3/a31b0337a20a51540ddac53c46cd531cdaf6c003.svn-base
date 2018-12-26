#include "wtk_event.h"


/*
void wtk_event_reset_sig(wtk_event_t *e)
{
	e->in_queue=e->read=e->write=e->eof=e->reof=e->error=e->writepending=e->writeepolled=e->readepolled=e->errepolled=0;
}
*/

void wtk_event_print(wtk_event_t *e)
{
	printf("============ evt ==============\n");
	printf("want read:\t%d\n",e->want_read);
	printf("want write:\t%d\n",e->want_write);
	printf("read:\t\t%d\n",e->read);
	printf("write:\t\t%d\n",e->write);
	printf("reof:\t\t%d\n",e->reof);
	printf("eof:\t\t%d\n",e->eof);
	printf("error:\t\t%d\n",e->error);
	printf("readpolled:\t%d\n",e->readepolled);
    printf("writeepolled:\t%d\n",e->writeepolled);
    printf("errpolled:\t%d\n",e->errepolled);
    printf("writepending:\t%d\n",e->writepending);
}
