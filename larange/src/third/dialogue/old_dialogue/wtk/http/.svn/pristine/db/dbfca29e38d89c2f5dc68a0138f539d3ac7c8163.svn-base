#ifdef WIN32
#include "wtk_select.h"

int wtk_epoll_create(wtk_epoll_t *epoll)
{
    return 0;
}

int wtk_epoll_init(wtk_epoll_t *epoll,int event_num)
{
    epoll->r_set=(fd_set*)wtk_calloc(1,sizeof(fd_set));
    epoll->w_set=(fd_set*)wtk_calloc(1,sizeof(fd_set));
    epoll->e_set=(fd_set*)wtk_calloc(1,sizeof(fd_set));
    epoll->tmpr_set=(fd_set*)wtk_calloc(1,sizeof(fd_set));
    epoll->tmpw_set=(fd_set*)wtk_calloc(1,sizeof(fd_set));
    epoll->tmpe_set=(fd_set*)wtk_calloc(1,sizeof(fd_set));
    FD_ZERO(epoll->r_set);
    FD_ZERO(epoll->w_set);
    FD_ZERO(epoll->e_set);
    epoll->montier_events=0;
    wtk_queue_init(&(epoll->event_queue));
    return 0;
}

int wtk_epoll_clean(wtk_epoll_t *epoll)
{
    if(epoll->r_set)
    {
        wtk_free(epoll->r_set);
        wtk_free(epoll->w_set);
        wtk_free(epoll->e_set);
        wtk_free(epoll->tmpw_set);
        wtk_free(epoll->tmpr_set);
        wtk_free(epoll->tmpe_set);
        epoll->r_set=0;
    }
    return 0;
}

int wtk_epoll_close(wtk_epoll_t *epoll)
{
    return 0;
}

int wtk_epoll_remove_event(wtk_epoll_t* epoll, int fd,wtk_event_t *event)
{
    if(event->errepolled)
    {
        FD_CLR(fd,epoll->e_set);
        event->errepolled=0;
    }
    if(event->readepolled || event->	want_read || event->want_accept )
    {
        FD_CLR(fd, (epoll->r_set));
        event->readepolled=0;
    }
    if(event->writeepolled || event->want_write)
    {
        FD_CLR(fd,(epoll->w_set));
        event->writeepolled=0;
    }
    if(event->in_queue)
    {
        wtk_queue_remove(&(epoll->event_queue),&(event->select_node_q));
        --epoll->montier_events;
        event->in_queue=0;
    }
    return 0;
}

void wtk_epoll_push_event(wtk_epoll_t* epoll,wtk_event_t *event)
{
    if(event->in_queue==0)
    {
        ++epoll->montier_events;
        wtk_queue_push(&(epoll->event_queue),&(event->select_node_q));
        event->in_queue=1;
    }
}

int wtk_epoll_event_add_read(wtk_epoll_t *epoll,int fd,wtk_event_t *event)
{
    if(event->readepolled==0)
    {
        if(event->in_queue==0)
        {
            wtk_epoll_push_event(epoll,event);
        }
        FD_SET(fd, (epoll->r_set));
        event->readepolled=1;
    }
    return 0;
}

int wtk_epoll_event_remove_read(wtk_epoll_t *epoll,int fd,wtk_event_t *event)
{
    if(event->readepolled)
    {
        FD_CLR(fd,epoll->r_set);
        event->readepolled=0;
    }
    return 0;
}

int wtk_epoll_event_add_write(wtk_epoll_t *epoll,int fd,wtk_event_t *event)
{
    if(event->writeepolled==0)
    {
        if(event->in_queue==0)
        {
            wtk_epoll_push_event(epoll,event);
        }
        FD_SET(fd,epoll->w_set);
        event->writeepolled=1;
    }
     return 0;
}

int wtk_epoll_event_remove_write(wtk_epoll_t *epoll,int fd,wtk_event_t *event)
{
    if(event->writeepolled)
    {
        FD_CLR(fd,epoll->w_set);
        event->writeepolled=0;
    }
    return 0;
}

int wtk_epoll_add_event(wtk_epoll_t *epoll,int fd,wtk_event_t* event)
{
    if(event->errepolled==0)
    {
        FD_SET(fd, (epoll->e_set));
        event->errepolled=1;
    }
    if(event->	want_read || event->want_accept)
    {
        FD_SET(fd, (epoll->r_set));
        event->readepolled=1;
    }
    if(event->want_write)
    {
        FD_SET(fd,(epoll->w_set));
        event->writeepolled=1;
    }
    wtk_epoll_push_event(epoll,event);
    return 0;
}

int wtk_epoll_remove_connection(wtk_epoll_t *epoll,wtk_connection_t *c)
{
    wtk_epoll_remove_event(epoll,c->fd,c->event);
    return 0;
}

int wtk_epoll_add_connection(wtk_epoll_t *epoll,wtk_connection_t *c)
{
    wtk_epoll_add_event(epoll,c->fd,(c->event));
    return 0;
}

void fd_set_print(fd_set* set)
{
    int i;

    for(i=0;i<set->fd_count;++i)
    {
        printf("%d,",set->fd_array[i]);
    }
}

int wtk_epoll_process(wtk_epoll_t *epoll,int timeout)
{
    wtk_queue_node_t *n,*t;
    fd_set *r_set,*w_set,*e_set;
    wtk_event_t *e;
    int ret;

    *epoll->tmpr_set=*epoll->r_set;
    *epoll->tmpw_set=*epoll->w_set;
    *epoll->tmpe_set=*epoll->e_set;
    r_set=epoll->tmpr_set;
    w_set=epoll->tmpw_set;
    e_set=epoll->tmpe_set;
    ret=select(0,r_set,w_set,e_set,0);
    if(ret>0)
    {
        for(n=epoll->event_queue.pop;n;n=t)
        {
            t=n->next;
            e=(wtk_event_t*)wtk_queue_node_data(n,wtk_event_t,select_node_q);
            e->read= FD_ISSET(e->fd,r_set);
            e->write=FD_ISSET(e->fd,w_set);
            e->error=FD_ISSET(e->fd,e_set);
            if(e->read ||  e->write || e->error)
            {
                ret-=e->read+e->write+e->error;
                e->handler(e->data,e);
                if(ret==0){break;}
            }
        }
    }
    /*
    if(ret!=0)
    {
        fd_set_print(r_set);
        fd_set_print(w_set);
        fd_set_print(e_set);
        wtk_debug("%d\n",WSAGetLastError());
    }*/
    return ret;
}
#endif
