#include "DispatchMsgService.h"


#include <stdio.h>
#include <algorithm>
#include <iostream> 
#include <sstream> 

#define MAX_ITEM_IN_MSG_QUEUE 65536

DispatchMsgService DispatchMsgService::instance_;

DispatchMsgService::DispatchMsgService() : msg_queue_( PosixQueue<iEvent>(65536) )
{
    //msg_queue_ = new PosixQueue<Message>(65536);
}

DispatchMsgService::~DispatchMsgService()
{
}

bool DispatchMsgService::init()
{
    /* create thread */
    int ret=pthread_create(&thread_hdl_, NULL, svc, this);
    if(ret !=0 )  
    {  
        printf("Create pthread error!\n");  
        return false;  
    }
    svr_exit_ = false;
    
    return true;
}

bool DispatchMsgService::uninit()
{
    //delete msg_queue_;
    svr_exit_ = true;
    pthread_join(thread_hdl_, NULL);
    return true;
}

bool DispatchMsgService::subscribe(u16 eid, iEventHandler* handler)
{
    if (NULL == handler) return false;
    
    T_EventHandlersMap::iterator iter = subscribers_.find(eid);
    if (iter != subscribers_.end())
    {
        T_EventHandlers::iterator hdl_iter = std::find(iter->second.begin(), iter->second.end(), handler);
        if ( hdl_iter == iter->second.end())
        {
           iter->second.push_back(handler);
        }
    }
    else
    {
        subscribers_[eid].push_back(handler);
    }
    
    return true;
}

bool DispatchMsgService::unsubscribe(u16 eid, iEventHandler* handler)
{
    if (NULL == handler) return false;
    
    T_EventHandlersMap::iterator iter = subscribers_.find(eid);
    if (iter != subscribers_.end())
    {
        T_EventHandlers::iterator hdl_iter = std::find(iter->second.begin(), iter->second.end(), handler);
        if ( hdl_iter != iter->second.end())
        {
           iter->second.erase(hdl_iter);
        }
    }
    
    return true;
}

bool DispatchMsgService::publish(iEvent* ev)
{
    if (NULL == ev)
    {
        return false;
    }

    return msg_queue_.enqueue(ev, 0);
}

bool DispatchMsgService::process(const iEvent* ev)
{
    if (NULL == ev)
    {
        return NULL;
    }
	
    u16 eid = ev->getEid();

    if (eid == EID_UNKOWN)
    {
        printf("event is invalid");
        return false;
    }

    T_EventHandlersMap::iterator handlers = subscribers_.find(eid);
    if (handlers == subscribers_.end())
    {
        printf("DispatchMsgService : no any event handler subscribed %d", eid);
        return false;
    }

    for (T_EventHandlers::iterator iter = handlers->second.begin();
         iter != handlers->second.end();
         iter++)
    {
        iEventHandler* handler = *iter;
        handler->handle(ev);
    }

    return true;
}

void* DispatchMsgService::svc(void* argv)
{
    printf("dms is runing\n");

    DispatchMsgService* dmsvr = (DispatchMsgService*) argv;
    if (argv == NULL)
    {
        printf("parameter of thread is invalid.\n");
        return NULL;
    }
    
    while(!dmsvr->svr_exit_)
    {
        iEvent* ev = NULL;
        
        /* wait only 1 ms to dequeue */
        if (-1 == dmsvr->msg_queue_.dequeue(ev, 1))
        {
            continue;
        }
        
        std::stringstream ss;
        ev->dump(ss);
        printf("dequeue an event(%s).\n", ss.str().c_str());

        dmsvr->process(ev);
        delete ev;
    }

    dmsvr->subscribers_.clear();
    
    printf("dms quit.\n");

    return NULL;
}


