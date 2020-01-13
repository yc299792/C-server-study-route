#include<stdio.h>
#include<zmq.h>
#include<string.h>
#include<assert.h>

/*
*该模块有三个连接，
*1、接受任务的连接，负则pull任务
*2、发送结果的连接，把任务执行结果发布给任务结果收集者
*3、接受控制信号的连接、收到结束信号就结束程序
*/

int int main(int argc, char const *argv[])
{
	void* context = zmq_init(1);
	assert(context);

	//连接control
	void* pullsocket = zmq_socket(context,ZMQ_SUB);
	assert(pullsocket);
	zmq_connect(pullsocket,"tcp://localhost:8000");
	zmq_setsockopt(pullsocket,ZMQ_SUBSCRIBE,"",0);

	//连接任务发布者
	void* recvwork = zmq_socket(context,ZMQ_PULL);
	zmq_connect(recvwork,"tcp://localhost:9000");

	//发送任务执行结果消息给收集者
	void* sendpush = zmq_socket(context,ZMQ_PUSH);
	zmq_connect(sendpush,"tcp://localhost:9999");

	zmq_pollitems_t item[] = {
		{pullsocket,0,ZMQ_POLLIN,0},
		{recvwork,0,ZMQ_POLLIN,0},
	};

	while(1)
	{
		zmq_msg_t msg;
		zmq_poll(item,2,-1);

		if(item[1].revents & ZMQ_POLLIN)
		{
			zmq_msg_init(&msg);
			zmq_recv(recvwork,&msg,0);

			//进行worker


			//发送结果
			zmq_msg_init(&msg);
			zmq_send(sendpush,&msg,0);

			zmq_close(&msg);
		}

		//受到控制就结束
		if(item[0].revents & ZMQ_POLLIN)
		{
			break;
		}

	}

	zmq_close(pullsocket);
	zmq_close(recvwork);
	zmq_close(sendpush);
	zmq_term(context);




	return 0;
}
