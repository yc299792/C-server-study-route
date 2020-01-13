/***************************************************************
*文件传输的服务端
****************************************************************/
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

#define FILE_FRAME_SIZE 1024


struct file_route_msg{
	unsigned int path_len;
	char path[255];
};



int main(int argc, char const *argv[])
{

	//使用zmq接受文件信息
	zmq::context_t context(1);

	zmq::socket_t socket(context,ZMQ_PULL);

	socket.setsockopt(ZMQ_RCVTIMEO,0);

	socket.bind("tcp://*:9000");

	//定义poll数组
	zmq::polliten_t items[]={
		{socket,0,ZMQ_POLLIN,0}
	};

	//接收文件
	while(true)
	{
		int rc = zmq::poll(items[0],1,-1);

		if(items[0].revents & ZMQ_POLLIN)
		{
			zmq::message_t msg_frame；
			socket.recv(&msg_frame);

			file_route_msg fs;
			//路径长度
			memcpy(&fs.path_len,msg_frame.data(),sizeof(fs.path_len));
			fs.path_len = ntohl(fs.path_len);

			//路径名
			socket.recv(&msg_frame);
			memset(fs.path,0,255);
			memcpy(fs.path,msg_frame.data(),fs.path_len);

			FILE* fp = fopen(fs.path,"wb+");

			if(NULL == fp)
			{
				std::cout << "fopen failed!" << std::endl;
				return -1;
			}

			while(true)
			{
				//接受文件内容
				bool more = msg_frame.more();
				//如果还有帧就一直接收
				if(more)
				{
					socket.recv(&msg_frame);
					int size = fwrite((char*)msg_frame.data(),1,msg_frame.size(),fp);
					if(size != msg_frame.size())
					{
						std::cout << "fwrite file failed!" << std::endl;
					}

				}
				else
				{
					break;
				}
			}

			fclose(fp);
			std::cout << "recieved file content success!" << std::endl;


		}
	}


	socket.close();
	contenx.close();


	return 0;
}
