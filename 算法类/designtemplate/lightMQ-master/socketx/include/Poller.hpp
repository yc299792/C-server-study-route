#ifndef POLLER_HPP
#define POLLER_HPP

#include "utilx.hpp"
#include "Event.hpp"


namespace socketx
{
    /*Forward declaration*/
    class EventLoop;

    class Poller
    {
        public:
            Poller();
            ~Poller();

            /*Wrapper function of Linux/Unix poll
            * Return a vector of active events
            */
            std::vector<std::shared_ptr<Event>> poll();//对poll的封装，会返回一个激活事件的指针容器

            /*Update or delete an event from eventList*/
            void updateEvent(std::shared_ptr<Event> event);
            void deleteEvent(std::shared_ptr<Event> event);

            void setTimeout(int timeout)
            {
                timeout_ = timeout;
            }

        private:
            //std::vector<Event *> eventsList;
            std::vector<struct pollfd> pollfdList;//这个是用来监听的fd文件描述符数组
            std::map<int, std::shared_ptr<Event>> eventsMap; //事件map

            int timeout_;//超时时间
    };
}
#endif
