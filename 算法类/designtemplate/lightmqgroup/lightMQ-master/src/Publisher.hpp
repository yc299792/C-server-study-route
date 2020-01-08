
#ifndef _PUBLISHER_HPP__
#define _PUBLISHER_HPP__

#include "socketx.hpp"
#include "Protocol.hpp"
//消息的发布者
class Publisher
{
    public:
        Publisher(std::shared_ptr<socketx::Connection> conn)
        : conn_(conn), connected(true)//使用初始化列表初始化连接和连接状态
        {
            
        }
		//发布消息的接口，提供主题和消息内容
        void publish(std::string topic, std::string msg);
        void unpublish(std::string topic);
    
    private: 
        std::vector<std::string> topic_;//消息主题数组
        std::shared_ptr<socketx::Connection> conn_;//连接的对象
        bool connected;//是否是连接状态
};


#endif
