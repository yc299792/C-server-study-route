#ifndef _PUBSUB_HPP__
#define _PUBSUB_HPP__

#include "socketx.hpp"
#include "Protocol.hpp"

//这个就是中间队列
class PubSub{

    public:
        PubSub()=default;

        void subscriberStub(std::shared_ptr<Connection> conn, const Message &msg);//供订阅者订阅消息的接口
        void publisherStub(std::shared_ptr<Connection> conn, const Message &msg);//供发布者发布消息的接口
        void filter(std::shared_ptr<Connection> conn, const Message &msg);//消息的过滤器
        
    private:
        void regist(std::shared_ptr<Connection> conn);//连接
        void subscribe(std::shared_ptr<Connection> conn, const std::string &topic);//订阅
        void unsubscribe(std::shared_ptr<Connection>, const std::string &topic);//取消订阅

        void publish(const std::string &topic, const Message &msg);
        void unpublish(std::shared_ptr<Connection> conn, const std::string &topic);

        void broadcast(const string &topic);

        void checker();
        void publishMsg(const std::string &topic);

        /*
         * key : topic值
         * value : 消息队列，一个topic对应一个单独的msg queue，
         *         msg queue确保把所有的消息发送所有的订阅者，而且仅发送一次
         */
        std::unordered_map<std::string, std::shared_ptr<socketx::MessageQueue>> queueMap_;

        //用来存储连接的map
        std::unordered_map<std::string, std::vector<std::shared_ptr<Connection>>> channelMap_;


        /*
         * key : 连接的客户端
         * value : 一个集合，所有订阅的主题
         */
        std::unordered_map<std::shared_ptr<Connection>, std::set<std::string>> subscriberMap_;
        socketx::ThreadPool pool_;
};
#endif
