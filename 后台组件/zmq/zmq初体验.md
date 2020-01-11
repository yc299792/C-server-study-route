[TOC]

### 传统网络通信

![1578703754582](C:\Users\ADMINI~1\AppData\Local\Temp\1578703754582.png)

对着这个图就很简单了：

服务端：

肯定要有一个socket然后绑定fd和ip，port，当然这时候需要转换成网络字节序，然后一堆操作，绑定完成以后就用这个fd去监听这个端口，开启listen，然后等待连接就是调用accept阻塞在这里等待连接，客户端连接后循环阻塞读写。

客户端：

先要填充好socket，然后调用connect连接服务端，也要带上ip和端口号，建立连接以后，用write和read读写，最后关闭套接字。

可以看到这种网络模型有很多问题要解决：

阻塞I/O:也就是说服务端或者客户端的读写都会阻塞，这样毫无疑问程序太死板。

多路复用：服务端要解决I/O多路复用，也就是可以处理多个客户端，比如epoll，select，poll等多路复用技术。

信号事件驱动I/O：也就是说当有I/O时采取通知机制，而不是轮询。

异步I/O：异步进行I/O操作，因为读写都会耗时，这不利于高并发的服务器开发，可以采取多线程。



而且在传统这种网络编程中，socket细节还比较复杂，并且需要自行解决分包组包，粘包，异常重连，消息缓存，消息加密，且TCP是一对一连接，就是一个socket只能有有一个连接。

那么接下来就介绍一个zmq的基本使用，它帮我们解决了这些问题。

### zmq是什么

zmq：我的理解就是，它是一个库，一个封装了网络通信过程，程序员只用使用它的接口就能进行网络编程，并且不在像之前那么麻烦，虽然现在可能还不够优秀，但是它的很多思想都是很好的，而且也有很多应用，所以尝试用它是一个好的选择。

客户端：

1. 使用**void \*zmq_ctx_new ();**,这个函数返回一个新的contex对象
2. 使用**void \*zmq_socket (void *context, int type);**得到一个socket对象，就是类似句柄，这个type是socket类型，抽象类12种，每一种对应一个场景。
3. 使用**int zmq_connect (void \*socket, const char *endpoint);**进行连接，连接也是非常简单的，传入一个字符串和scoket对象就可以了。
4. 然后就可以使用接受发送函数进行传输了。



服务端：

1. 使用**void \*zmq_ctx_new ();**,这个函数返回一个新的contex对象
2. 使用**void \*zmq_socket (void *context, int type);**得到一个socket对象，就是类似句柄，这个type是socket类型，抽象类12种，每一种对应一个场景。
3. 前两步是一样的，第三步绑定：**int zmq_bind (void \*socket, const char *endpoint);**绑定ip和端口号。
4. 然后就可以了。

总结一下，zmq的socket是可以复用的，可以同时连接多个客户端，连接只关注ip和端口号，然后服务端也是只用绑定ip和端口号，总之：真香。然后具体流程看代码：

客户端：

```C++
/*
 * 
 * g++ -o request request.cpp -lzmq
 *
 */


#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include<assert.h>

#define MAX_BUFFER_LEN 1024

//同时定义了两个服务器，这里可以更多，格式就这么写即可，下面只用一个套接字
const char* hosts[] = {
    "tcp://192.168.116.130:9000",
    "tcp://192.168.116.130:8000"

};

int main (void){    

    printf ("Connecting to hello world server…\n");
    void *context = zmq_ctx_new ();
    assert(context);
    void *requester = zmq_socket (context, ZMQ_REQ);

    for (int i = 0; i < sizeof(hosts) / sizeof(hosts[0]); i ++){
        if (0 != zmq_connect (requester, hosts[i])) {
            printf("connect server %s failed.\n", hosts[i]);
        }
    }
	//会轮流发送给连接的服务器，负载均衡，而且可以先启动客户端，真香
    for (int i = 0; i < 50; i ++){
        char buffer [MAX_BUFFER_LEN] = {0};
        sprintf(buffer, "hello_%d", i);
        printf ("Send %s to server.\n", buffer);
        zmq_send (requester, buffer, strlen(buffer), 0);
        memset(buffer, 0, MAX_BUFFER_LEN);
        zmq_recv (requester, buffer, 10, 0);
        printf ("Received %s.\n", buffer);
    }

    zmq_close (requester);
    zmq_ctx_destroy (context);
   
    return 0;
}



```



服务端：

```C++
/*
 * 
 *  g++ -o responser responser.cpp -lzmq
 */

#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#define MAX_BUFFER_LEN 1024

int main (void)
{
    //  Socket to talk to clients
    void *context = zmq_ctx_new ();
    assert(context);
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://*:8000");
    if (rc != 0)
    {
        printf ("Error occurred during zmq_bind(): %s\n", zmq_strerror (errno));
        return -1;
    }

    int count = 0;

    while (1) 
    {
        char buffer [MAX_BUFFER_LEN] = {0};
        zmq_recv (responder, buffer, 10, 0);
        printf ("%s.\n", buffer);
        sleep (1);
        zmq_send (responder, "World", 5, 0);
        printf("send world to client with count=%d.\n", count);
        count ++;
    }

    return 0;
}

```





