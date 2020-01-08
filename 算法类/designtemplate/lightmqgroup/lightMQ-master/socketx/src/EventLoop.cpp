#include "EventLoop.hpp"

namespace socketx
{
    EventLoop::EventLoop():
        poller(std::make_shared<Poller>()),stop(false)
    {
        IgnoreSIGPIPE();//忽略SIGPIPE信号
    }

    EventLoop::~EventLoop()
    {

    }

    /* Main Loop
    *  The Loop will stop when stop flag is set to true
    */
    void EventLoop::loop()
    {
        printf("EventLoop starts......\n");
        while(!stop)
        {
            activeEvents.clear();//清空一下容器
            activeEvents = poller->poll();//一直监听，底层使用poll方式，返回的是活跃事件的数组
            for(auto it=activeEvents.begin(); it!=activeEvents.end(); ++it)
            {
                (*it)->handleEvent();//依次调用事件的处理函数，会调用每一个事件的处理函数回调函数
            }
        }
        printf("EventLoop ends......\n");
    }

    /*Update events by invoke poller's function*/
    void EventLoop::updateEvent(std::shared_ptr<Event> event)
    {
        poller->updateEvent(event);
    }

    void EventLoop::deleteEvent(std::shared_ptr<Event> event)
    {
        poller->deleteEvent(event);
    }

}
