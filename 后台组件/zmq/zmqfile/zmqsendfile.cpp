/***************************************************************
*文件传输的客户端
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

int read_file_frame(char* filename,int pos,char* buffer,int length)
{
	FILE* fp = fopen(filename,"rb");
	if (NULL == fp)
	{
		std::cout<<"fopen failed!"<<std::endl;
		return -1;
	}

	//调整文件指针
	fseek(fp,pos,SEEK_SET);
	int size = fread(buffer,1,length,fp);
	if(size != length)
	{
		std::cout << "read over!" << std::endl;
	}

	fclose(fp);
	return size;
}

int main(int argc, char const *argv[])
{
	//获取文件的路径
	if(argc != 2)
	{
		std::cout << "please input like this:" << argv[0] << " <filepath>"<<std::endl;
	}

	//判断文件是否存在
	if(0 != access(argv[1],R_OK))
	{
		std::cout<<"file is not exists,please check:"<<argv[1]<<std::endl;
		return -1;
	}
	//开始使用zmq
	zmq::context_t context(1);

	zmq::socket_t socket(context,ZMQ_PUSH);

	socket.setsockopt(ZMQ_SNDTIMEO,0);

	socket.connect("tcp://localhost:9000");


	//开始发送文件
	file_route_msg head;
	memset(head,0,sizeof(head));
	head.path_len = strlen(argv[1]);
	head.path_len = htonl(head.path_len);//转换成网络字节序
	memcpy(head.path_len,argv[1],head.path_len);


	
	//发送文件路径
	socket.send(&head.path_len,4,ZMQ_SNDMORE);
	socket.send(head.path,strlen(head.path),ZMQ_SNDMORE);

	int pos = 0;

	char buffer[FILE_FRAME_SIZE] = {0};
	while(1)
	{
		buffer[FILE_FRAME_SIZE] = {0};

		//从文件中读取一帧
		int size = read_file_frame(argv[1],pos,buffer,FILE_FRAME_SIZE);
		pos += size;

		if(size < 0)
		{
			std::cout << "read file failed!" << std::endl;
			socket.send(NULL,0,0);
			break;
		}
		else if(size == FILE_FRAME_SIZE)
		{//完整读取一帧
			zmq::message_t msg_frame(buffer,size);
			socket.send(msg_frame,ZMQ_SNDMORE);
		}
		else if(size < FILE_FRAME_SIZE)
		{
			std::cout << "reaf file content over" <<pos<<" bytes"<<std::endl;
			zmq::message_t msg_frame(buffer,size);
			socket.send(msg_frame,0);
			break;

		}

	}
	socket.close();
	context.close();


	return 0;
}
