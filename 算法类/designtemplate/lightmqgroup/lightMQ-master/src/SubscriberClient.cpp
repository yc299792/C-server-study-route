
#include "Subscriber.hpp"
//订阅者的客户类
class SubscriberClient
{
    public:
        SubscriberClient(socketx::EventLoop *loop, std::string hostname, std::string port)
        : loop_(loop), 
          hostname_(hostname),
          port_(port),
          client_(std::make_shared<socketx::Client>(loop, hostname, port))
        {
			//bind函数的返回值是一个函数指针也就是一个可调用的对象，参数是一个函数指针，加上它的参数列表，第一个参数为this指针，第二个参数调用者指定.
            client_->setHandleConnectionFunc(std::bind(&SubscriberClient::handleConnection, this, std::placeholders::_1));//设置处理的回调函数
            client_->setHandleCloseEvents(std::bind(&SubscriberClient::handleCloseEvents, this, std::placeholders::_1));

            /*Get file descriptor of stdin and regist it into EventLoop*/
            cout<<"Input <Cmd> <Channel> : " << endl;
            int fd = fileno(stdin);
            stdinConn = std::make_shared<socketx::Connection>(loop_, fd);
            stdinConn->setHandleReadEvents(std::bind(&SubscriberClient::stdinReadEvents, this, std::placeholders::_1));//读取事件的处理函数
            stdinConn->registReadEvents();
        }

        void start()
        {
            client_->start();//客户端启动
        }

        void handleConnection(std::shared_ptr<socketx::Connection> conn)
        {
            printf("New connection comes, we are going to set read events!!!\n");
            client_->setHandleReadEvents(std::bind(&SubscriberClient::handleReadEvents, this, std::placeholders::_1));
            subscriber_ = std::make_shared<Subscriber>(conn);
            subscriber_->regist();
        }

        void handleReadEvents(std::shared_ptr<socketx::Connection> conn)
        {
            socketx::Message msg = conn->recvmsg();
            printf("recved msg : %s", msg.getData());
            if(msg.getSize()==0)
            {
                conn->handleClose();
                return;
            }

            subscriber_->filter(msg);
        }
        
        void handleCloseEvents(std::shared_ptr<socketx::Connection> conn)
        {
            printf("Close connection...\n");
            loop_->quit();
        }

        void stdinReadEvents(std::shared_ptr<socketx::Connection> conn)
        {
            std::string cmd, topic;
            if( cin >> cmd >> topic)
            {
                if(cmd=="sub") subscriber_->subscribe(topic);
                else if(cmd=="unsub") subscriber_->unsubscribe(topic);
            }
            else
                printf("Read error from stdin....\n");
        }

    private:
        std::shared_ptr<Subscriber> subscriber_;
        std::shared_ptr<socketx::Connection> stdinConn;//连接的对象
        socketx::EventLoop *loop_;//用来监听事件的对象
        std::shared_ptr<socketx::Client> client_;//客户端对象，所有网路操作都由这个对象完成
        std::string hostname_;//主机ip端口号
        std::string port_;
};

//订阅者的客户端
int main(int argc, char **argv)
{
    if(argc!=3)
    {
        fprintf(stderr,"usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    std::string hostname(argv[1]);//主机
    std::string port(argv[2]);//端口号
    socketx::EventLoop loop;//创建一个Eventloop对象
    SubscriberClient client(&loop, hostname, port);
    client.start();//会去连接服务端
    loop.loop();//循环处理连接的各种事件读写连接等事件

    return 0;
}