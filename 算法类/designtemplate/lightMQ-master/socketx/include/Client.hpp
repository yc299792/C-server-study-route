#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "utilx.hpp"
#include "Socket.hpp"
#include "Connection.hpp"

namespace socketx{

    class Client{//这个是对Client的一个封装，主要就是客户端的封装
        public:
            Client(EventLoop *loop, std::string hostname, std::string port);
            
            ~Client();

            /*Start to connect to a host. 
            * This function should retry the connection
            * until the connection is established or terminated by users.
            */
            void start();//用户使用这个方法连接服务器

            /* Remove the connection, called by an Connection*/
            void removeConnection(std::shared_ptr<Connection> conn);


            /* Provide an API for users 
            *  to handle new connections
            */
            void setHandleConnectionFunc(const std::function<void(std::shared_ptr<Connection>)> &func){
                handleConnectionFunc = func;
            }

            /* Provide an API for users
            *  to handle events of a connection.
            *  Users need to set these functions in their codes
            *  to regist corresponding events.
            */
            void setHandleReadEvents(const std::function<void(std::shared_ptr<Connection>)> &func){
                handleReadEvents = func;
                currentConn->setHandleReadEvents(handleReadEvents);//设置新连接读事件的回调函数
                currentConn->registReadEvents();
            }
            void setHandleWriteEvents(const std::function<void(std::shared_ptr<Connection>)> &func){
                handleWriteEvents = func;
                currentConn->setHandleWriteEvents(handleWriteEvents);
                currentConn->registWriteEvents();
            }
            void setHandleCloseEvents(const std::function<void(std::shared_ptr<Connection>)> &func){
                handleCloseEvents = func;
            }

            /* This function need reture the current connection*/
            std::shared_ptr<Connection> getCurrentConnection(){
                return currentConn;
            }

        private:

            /* Handle new connections
            * This function is used to create a new object
            * Connection as an event listened by EventLoop
            */
            void newConnection(int fd);//增加一个被监听的文件描述符

            EventLoop *loop_;//用来循环监听服务器端的写入事件
            std::string hostname_;//主机ip
            std::string port_;//端口号
            std::shared_ptr<ClientSocket> socket_;//这个是对socket的封装类对象
            std::shared_ptr<Connection> currentConn;//该对象用于从缓冲区读写数据
            std::map<int, std::shared_ptr<Connection>> connectionsMap;//连接的map
            //处理各个事件的回调函数
            std::function<void(std::shared_ptr<Connection>)> handleConnectionFunc;//就是回调函数的实现就是C++中对函数指针的封装
            std::function<void(std::shared_ptr<Connection>)> handleReadEvents;
            std::function<void(std::shared_ptr<Connection>)> handleWriteEvents;
            std::function<void(std::shared_ptr<Connection>)> handleCloseEvents;
    };

}


#endif