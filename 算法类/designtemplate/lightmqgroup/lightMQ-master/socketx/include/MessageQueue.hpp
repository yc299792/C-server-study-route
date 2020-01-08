#ifndef MESSAGEQUEUE_HPP
#define MESSAGEQUEUE_HPP

#include "socketx.hpp"

namespace socketx
{
    class MessageQueue
    {
        public:
            MessageQueue()=default;

            ssize_t send();
            void recv(Message msg);

            void addConnection(std::shared_ptr<Connection> conn);
            void removeConnection(std::shared_ptr<Connection> conn);
            bool empty() const{ return queue_.empty();}
			int getGroupSize(){return sendGroup.size();}

        private:
            squeue<Message> queue_;//消息队列
            //std::vector<std::shared_ptr<Connection>> sendList;//维护着一个消息发送的接受者列表,容器类型
            std::vector<std::vector<std::shared_ptr<Connection>>> sendGroup;//维护着订阅组
    };
}

#endif