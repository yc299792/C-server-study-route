#ifndef EVENT_HPP
#define EVENT_HPP

#include "utilx.hpp"


namespace socketx
{

    /*Forward declaration*/
    class EventLoop;
	//继承以后会有shared_from_this成员函数，这样在我们使用智能指针管理该类的对象的时候，可以安全的共享同一个对象，不用担心被释放两次
	//异步函数调用的时候我们是不知道的什么时候调用的，为了避免别人调用的时候这个对象不被释放，所以我们传递一个指向自身的shared——ptr给异步函数
	//就不用担心变量会失效了，引用计数谁最后用谁进行释放，保活事件对象
    class Event:public std::enable_shared_from_this<Event>
    {
        public:
            Event(EventLoop *loop, int fd);
            ~Event();

            /*Handle the event according to revents*/
            void handleEvent();
            
            /*Regist Read and Write events*/
            void enableReading();
            void enableWriting();
            void disableReading();
            void disableWriting();

            /*Unregiest events*/
            void deleteEvent();
            
            /*Set callback function*/
            void setReadFunc(const std::function<void()> &func)
            {
                readFunc = func;
            }
            
            void setWriteFunc(const std::function<void()> &func)
            {
                writeFunc = func;
            }
            
            void setErrorFunc(const std::function<void()> &func)
            {
                errorFunc = func;
            }

            /*Get or set some data member */
            int getFD()
            {
                return fd_;
            }
            
            int getEvents()
            {
                return events_;
            }
            
            int getRevents()
            {
                return revents_;
            }
            
            void setFD(int fd)
            {
                fd_ = fd;
            }
            
            void setEvents(int events)
            {
                events_ = events;
            }
            
            void setRevents(int revents)
            {
                revents_ = revents;
            }
        
        private:
            std::function<void()> readFunc;
            std::function<void()> writeFunc;
            std::function<void()> errorFunc;

            EventLoop *loop_;
            int fd_;
            int events_;
            int revents_;
    };

}
#endif
