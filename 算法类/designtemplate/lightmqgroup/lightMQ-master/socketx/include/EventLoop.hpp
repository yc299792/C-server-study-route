#ifndef EVENTLOOP_HPP
#define EVENTLOOP_HPP

#include "utilx.hpp"

#include "Event.hpp"
#include "Poller.hpp"


namespace socketx
{
    class EventLoop//循环类
    {
        public:
            EventLoop();
            ~EventLoop();

            /* Main Loop
            *  The Loop will stop when stop flag is set to true
            */
            void loop();
            void quit() { stop = true; }

            /*Update events by invoke poller's function*/
            void updateEvent(std::shared_ptr<Event> event);
            void deleteEvent(std::shared_ptr<Event> event);

        private:
            std::vector<std::shared_ptr<Event>> activeEvents;
            std::shared_ptr<socketx::Poller> poller;//智能指针管理poller
            std::atomic<bool> stop;//在类里面加上一个这样的对象是为了循环
    };
}
#endif