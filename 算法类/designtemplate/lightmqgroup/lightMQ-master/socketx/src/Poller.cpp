#include "Poller.hpp"

namespace socketx{

    Poller::Poller():
        timeout_(INFTIM)//默认一直阻塞
    {

    }

    Poller::~Poller()
    {
         
    }

    /*Wrapper function of Linux/Unix poll
    * Return a vector of active events
    */
    std::vector<std::shared_ptr<Event>> Poller::poll()
    {
        int num = ::poll(&*pollfdList.begin(),pollfdList.size(),timeout_);//poll是轮询机制
        
        /*Construct active events vector*/
        std::vector<std::shared_ptr<Event>> vec;
        for(auto it=pollfdList.begin();it!=pollfdList.end();++it)//遍历监听的数组
        {
            if(it->revents>0)//如果数组中某一个fd的revents》0就代表有读写事件，返回的事件掩码
            {
                auto events = eventsMap[it->fd];//fd为key
                events->setRevents(it->revents); //设置事件的掩码
                vec.push_back(events);//vec里添加一个event的指针
            }
        }
        return vec;
    }

    /*Update or delete an event from eventList*/
    void Poller::updateEvent(std::shared_ptr<Event> event)//这个event是异步共享的所以已经有了保活就不用担心了
    {//存在就更新
        int eventfd = event->getFD();
        if(eventsMap.count(eventfd))
        {
            /*Update pollfdList*/
            auto it = std::find_if(pollfdList.begin(),pollfdList.end(),
                                    [eventfd](struct pollfd &pollfd_){return eventfd==pollfd_.fd;});
            it->events = event->getEvents();
            it->revents = event->getRevents();
        }
        else
        {//不存在就增加
            /*Update eventsMap*/
            eventsMap[eventfd] = event;
            /*Update eventsList*/
            //eventsList.push_back(event);
            /*Update pollfdList*/
            struct pollfd pollfd_; 
            pollfd_.fd = eventfd;
            pollfd_.events = event->getEvents();
            pollfd_.revents = event->getRevents();
            pollfdList.push_back(pollfd_);
        }
    }
    
    void Poller::deleteEvent(std::shared_ptr<Event> event)
    {
        int eventfd = event->getFD();
        if(eventsMap.count(eventfd))//根据fd查到就删除
        {
            auto it = std::find_if(pollfdList.begin(),pollfdList.end(),
                                    [eventfd](struct pollfd &pollfd_){return eventfd==pollfd_.fd;});
            pollfdList.erase(it);
            eventsMap.erase(eventfd);
        }
        else
        {
            printf("Poller::deleteEvent error! No such an event...\n");
        }
    }
}
