#include <zmq.h>
#include<stdio.h>
#include<string.h>


int int main(int argc, char const *argv[])
{
	//获取上下文并指定后台I/O线程数
	void* context = zmq_init(1);

	//创建pullsocket对象
	void* pullsocket = zmq_socket(context,ZMQ_PULL);
	zmq_connect(pullsocket,"tcp://localhost:8000");

	//创建subsocket对象
	void* subsocket = zmq_socket(context,ZMQ_SUB);
	zmq_connect(subsocket,"tcp://localhost:9000");
	zmq_setsockopt(subsocket,ZMQ_SUBSCRIBE,"10000 ",6);

	//初始化轮询poll对象
	zmq_pollitem_t items[] = {
		{pullsocket,0,ZMQ_POLLIN,0},
		{subsocket,0,ZMQ_POLLIN,0}
	};

	//循环接受消息
	while(1)
	{
		zmq_msg_t msg;
		//阻塞等待事件发生
		zmq_poll(items,2,-1);

		if(items[0].revents & ZMQ_POLLIN)
		{
			zmq_msg_init(&msg);
			zmq_recv(pullsocket,&msg,0);
			//del msg



			zmq_close_msg(&msg);
		}
		if(items[1].revents & ZMQ_POLLIN)
		{
			zmq_msg_init(&msg);
			zmq_recv(subsocket,&msg,0);
			//del msg



			zmq_close_msg(&msg);
		}


	}
	zmq_close(subsocket);
	zmq_close(pullsocket);
	zmq_term(context);


	return 0;
}
