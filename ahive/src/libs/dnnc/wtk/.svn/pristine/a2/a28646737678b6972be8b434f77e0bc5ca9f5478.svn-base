#include "wtk_poll_evt.h" 

void wtk_poll_evt_init(wtk_poll_evt_t *evt,int evt_type)
{
	//wtk_debug("evt_type=%x\n",evt_type);
	//memset(evt,0,sizeof(wtk_poll_evt_t));
	evt->ref=0;
	evt->polled=0;
	evt->want_read=evt_type&WTK_CONNECT_EVT_READ?1:0;
	evt->want_write=evt_type&WTK_CONNECT_EVT_WRITE?1:0;
	evt->want_accept=evt_type&WTK_CONNECT_EVT_ACCEPT?1:0;

	evt->readpolled=0;
	evt->writepolled=0;
	evt->writepending=0;

	evt->read=0;
	evt->write=0;
	evt->error=0;
	evt->eof=0;
	evt->reof=0;

	evt->want_close=0;

	evt->use_ext_handler=0;
}

void wtk_poll_evt_print(wtk_poll_evt_t *evt)
{
	wtk_debug("============== evt=%p ============\n",evt);
	printf("polled: %d\n",evt->polled);
	printf("want_read: %d\n",evt->want_read);
	printf("want_write: %d\n",evt->want_write);
	printf("want_accept: %d\n",evt->want_accept);

	printf("read: %d\n",evt->read);
	printf("write: %d\n",evt->write);
	printf("error: %d\n",evt->error);
	printf("eof: %d\n",evt->eof);
	printf("reof: %d\n",evt->reof);

	printf("ref: %d\n",evt->ref);
	printf("want_close: %d\n",evt->want_close);
}

