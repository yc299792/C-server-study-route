#include "PubSub.hpp"
#include <glog/logging.h>

void PubSub::filter(std::shared_ptr<Connection> conn, const Message &msg)
{
    Protocol pro;
    pro.deserialization(msg);
    std::string cmd = pro.getCmd();
    LOG(INFO) << "Received a message of cmd: " << cmd << " topic: " << pro.getTopic() << "msg:" << pro.getMsg();
    
    if(cmd=="sub" || cmd=="unsub" || cmd=="regist")
        subscriberStub(conn, msg);
    else
        publisherStub(conn, msg);
}

void PubSub::subscriberStub(std::shared_ptr<Connection> conn, const Message &msg){
    Protocol pro;
    pro.deserialization(msg);
    if(pro.getCmd()=="sub")
        subscribe(conn,pro.getTopic());
    else if(pro.getCmd()=="unsub"){
        unsubscribe(conn,pro.getTopic());
    }else if(pro.getCmd()=="regist")
        regist(conn);
    else
    {
        LOG(ERROR) << "subscriberStub: no such command " << pro.getCmd() <<" ...";
    }
}

/*
*1、先创建一个协议对象，解析msg
*2、然后再去判断是哪种命令进行相应的操作
*
*/
void PubSub::publisherStub(std::shared_ptr<Connection> conn, const Message &msg)
{
    Protocol pro;
    pro.deserialization(msg);
    if(pro.getCmd()=="pub")
        publish(pro.getTopic(), msg);
    else if(pro.getCmd()=="unpub")
        unpublish(conn,pro.getTopic());
    else
    {
        LOG(ERROR) << "publisherStub: no such command...";
    }
}

//订阅如果没有连接会被拒绝
void PubSub::regist(std::shared_ptr<Connection> conn)
{
    if(!subscriberMap_.count(conn))
    {
        subscriberMap_[conn] = std::set<std::string>();
        LOG(INFO) << "A subscriber registed " << conn->getFD() << "...";
    }
}

void PubSub::subscribe(std::shared_ptr<Connection> conn, const std::string &topic)
{
    if(subscriberMap_.count(conn))
    {
        subscriberMap_[conn].insert(topic);//key是conn，value是一个集合
    }
    else
    {
        subscriberMap_[conn] = std::set<std::string>();//第一次订阅需要先加入conn到map中
        subscriberMap_[conn].insert(topic);
    }
    //队列中存储这种主题的消息，{key，value}的方式插入第一次需要为主题创建一个消息队列
    if(!queueMap_.count(topic))
    {
        //queueMap_[topic]->addConnection(conn);
        queueMap_.insert({topic,std::make_shared<socketx::MessageQueue>()});//创建一个智能指针维护MessageQueue对象
    }

    queueMap_[topic]->addConnection(conn);//就是topic里去添加一个conn
    
    LOG(INFO) << "Subscriber " << conn->getFD() << "subscribes topic " << topic.c_str();

}

void PubSub::unsubscribe(std::shared_ptr<Connection> conn, const std::string &topic)
{
    if(subscriberMap_.count(conn))
    {
        subscriberMap_[conn].erase(topic);
    }
    else
    {
        return;
    }
    
    if(queueMap_.count(topic))
    {
        queueMap_[topic]->removeConnection(conn);
    }
    else
    {
        LOG(INFO) << "Unsubscribe error: no such a topic....";
    }
}

void PubSub::publish(const std::string &topic, const Message &msg)
{
   LOG(INFO) << "called publish with content " << msg.getData();

   /*
    *  如果没有任何人订阅，那就广播给所有已经注册的客户端
    */
#if 1
   if(!queueMap_.count(topic))
   {
        broadcast(topic);
        queueMap_.insert({topic,std::make_shared<socketx::MessageQueue>()});
    }
#endif
    queueMap_[topic]->recv(msg);
    if(pool_.getIdleThreadNum()==0) pool_.addThread();
    pool_.submit(std::bind(&PubSub::publishMsg, this, topic));
}

void PubSub::unpublish(std::shared_ptr<Connection> conn, const std::string &topic)
{
    if(channelMap_.count(topic))
    {
        auto it = std::find(channelMap_[topic].begin(),channelMap_[topic].end(),conn);
        if(it == channelMap_[topic].end())
        {
            LOG(INFO) << "unpublish: no such a connection in this topic...";
        }
        else
        {
            channelMap_[topic].erase(it);
        }
    }
    else
    {
        LOG(INFO) << "Unpublish error: no such a topic....\n";
    }
}

void PubSub::broadcast(const string &topic)
{
    Protocol pro("broadcast", topic, "");
    Message msg = pro.serialization();
    for(auto it=subscriberMap_.begin();it!=subscriberMap_.end();++it)
    {
        if((it->second).count(topic)) continue;
        else
        {
            it->first->sendmsg(msg);
        }
    }
}

void PubSub::checker()
{

}

void PubSub::publishMsg(const std::string &topic)
{
    LOG(INFO) << "enter publishMsg";
    while(!queueMap_[topic]->empty())
    {
        if (1 != queueMap_[topic]->send())
        {
            //LOG(ERROR) << "send msg failed.";
        }
    }
}
