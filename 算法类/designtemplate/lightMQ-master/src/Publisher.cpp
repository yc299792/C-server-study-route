
#include "Publisher.hpp"
#include <glog/logging.h>

void Publisher::publish(std::string topic, std::string msg)
{
    Protocol pro("pub", topic, msg);//创建一个对象，按照协议
    Message pub_msg = pro.serialization();//底层会将消息都打包好
    conn_->sendmsg(pub_msg);//通过网络连接去发布这条消息，会到缓冲区，然后发布给MQ
    topic_.push_back(topic);//往topic里面加一个主题

    LOG(INFO) << "A message of topic" << topic <<" is publishing!";
}

void Publisher::unpublish(std::string topic)
{
    Protocol pro("unpub", topic, "");
    Message pub_msg = pro.serialization();
    conn_->sendmsg(pub_msg);
    //todo delete topic
    //topic_.move(topic);

    LOG(INFO) << "A topic " << topic << "will be unpublished!";
}


