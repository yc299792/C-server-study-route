#include "Event.hpp"
#include "EventLoop.hpp"

namespace socketx{

    Event::Event(EventLoop *loop, int fd):
        loop_(loop),fd_(fd),events_(0),revents_(0)
    {
        printf("New event established!\n");
    }

    Event::~Event()
    {

    }
    
    /*Handle the event according to revents*/
    void Event::handleEvent()
    {
        /*Reading*/
        if(revents_ & (POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI))
        {
            readFunc();//回调读写事件的函数
        }

        /*Writing*/
        if(revents_ & (POLLOUT | POLLWRNORM | POLLWRBAND))
        {
            writeFunc();
        }

        /*Error*/
        if(revents_ & (POLLERR | POLLNVAL))
        {
            errorFunc();
        }
    }

    /*Regist Read and Write events*/
    void Event::enableReading()
    {
        events_ |= POLLIN | POLLPRI;
        loop_->updateEvent(shared_from_this());//shared_from_this就是返回一个事件对象与this共享该对象
    }
    
    void Event::enableWriting()
    {
        events_ |= POLLOUT | POLLWRNORM | POLLWRBAND;
        loop_->updateEvent(shared_from_this());
    }
    
    void Event::disableReading()
    {
        events_ &= ~(POLLIN | POLLPRI);
        loop_->updateEvent(shared_from_this());
    }

    void Event::disableWriting()
    {
        events_ &= ~(POLLOUT | POLLWRNORM | POLLWRBAND);
        loop_->updateEvent(shared_from_this());
    }

    void Event::deleteEvent()
    {
        loop_->deleteEvent(shared_from_this());
    }
}