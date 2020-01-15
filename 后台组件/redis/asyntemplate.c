/**************************************************
*一个异步处理的框架简单实现
*封装完了可以适用于各种异步请求
*yc 2020-1-15
*用一个简单的异步http模拟一下
*
*
***************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>		/* close */
#include <netdb.h> 
#include<pthread.h>
#include <sys/epoll.h>


#define HTTP_VERSION    "HTTP/1.1"
#define USER_AGENT		"User-Agent: Mozilla/5.0 (Windows NT 5.1; rv:10.0.2) Gecko/20100101 Firefox/10.0.2\r\n"
#define ENCODE_TYPE		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
#define CONNECTION_TYPE "Connection: close\r\n"
#define BUFFER_SIZE 4096


#define ASYNC_CLIENT_NUM		1024
#define HOSTNAME_LENGTH			128
//设置epoll回调函数
typedef void (*async_result_cb)(const char *hostname, const char *result);

//epoll传递的参数
struct ep_arg {
	int sockfd;
	char hostname[HOSTNAME_LENGTH];
	async_result_cb cb;
};
//上下文
struct async_context{
	int epfd;
	pthread_t thread_id;

};

//域名解析函数
static char* host_to_ip(const char* hostname)
{
	//注意这个函数不可重入，也就是说只能在单线程中使用
	struct hostent* hostip = gethostbyname(hostname);
	if(hostip)
	{	//将int型ip转换成192.168.13.11得形式
		return inet_ntoa(*(struct in_addr*)*hostip->h_addr_list);
	}
	else{
		return NULL;
	}
}

//发起请求的函数,传入最后的回调函数
int async_client_request(struct async_context* ctx,char* hostname,char* resource,async_result_cb cb)
{
	char* ip = host_to_ip(hostname);
	if(NULL == ip) return -1;

	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in sin = {0};
	sin.sin_addr.s_addr = inet_addr(ip);
	sin.sin_port = htons(80);
	sin.sin_family = AF_INET;


	if(-1 == connect(sockfd,(struct sockaddr*)&sin,sizeof(struct sockaddr)))
	{
		return -1;
	}

	//设置sockfd为非阻塞的
	fcntl(sockfd,F_SETFL,O_NONBLOCK);

	char buffer[BUFFER_SIZE] = {0};

	int len = sprintf(buffer,
"GET %s %s\r\n\
Host: %s\r\n\
%s\r\n\
\r\n",
		 resource, HTTP_VERSION,
		 hostname,
		 CONNECTION_TYPE
		 );
	printf("%s\n",buffer);
	len = send(sockfd,buffer,strlen(buffer),0);

	//发送完之后不用阻塞，而是把监听交给epoll
	struct ep_arg *eparg = (struct ep_arg*)calloc(1, sizeof(struct ep_arg));
	if(eparg == NULL) return -1;
	eparg->sockfd = sockfd;
	eparg->cb = cb;

	struct epoll_event ev;
	ev.data.ptr = eparg;
	ev.events = EPOLLIN;

	int ret = epoll_ctl(ctx->epfd, EPOLL_CTL_ADD, sockfd, &ev); 
	printf("%d,%d,%d\n",ret,ctx->epfd,sockfd);
	return ret;
}
//线程处理响应，也就是异步处理响应
static void async_client_callback(void* arg)
{
	struct async_context *ctx = (struct async_context*)arg;
	int epfd = ctx->epfd;

	while(1)
	{
		//epoll每一个连接对应一个数元素
		struct epoll_event events[ASYNC_CLIENT_NUM] = {0};
		int nready = epoll_wait(epfd,events,ASYNC_CLIENT_NUM,-1);
		if(nready < 0)
		{
			if (errno == EINTR || errno == EAGAIN) {
				continue;
			} else {
				break;
			}
		}
		else if(nready == 0)
		{
			continue;
		}
		printf("nready:%d\n", nready);
		int i = 0;
		for(i = 0;i < nready;i++)
		{	//处理每一个返回的数据

			struct ep_arg* data = (struct ep_arg*)events[i].data.ptr;
			int sockfd = data->sockfd;

			char buffer[BUFFER_SIZE] = {0};

			recv(sockfd,buffer,BUFFER_SIZE,0);
			//回调连接的处理函数把数据传送过去
			data->cb(data->hostname,buffer);

			int ret = epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,NULL);

			close(sockfd);
			//记得释放掉内存
			free(data);

		}
	}

}

//初始化一个上下文对象，用于后续的异步返回处理和，请求处理
struct async_context* async_client_init(void)
{
	struct async_context* ctx = (struct async_context*)calloc(1,sizeof(struct async_context));
	if(NULL == ctx)
	{
		return NULL;
	}

	if((ctx->epfd = epoll_create(1)) < 0)
		return NULL;
	//创建线程处理发送请求后的，响应消息
	int ret = pthread_create(&ctx->thread_id,NULL,(void*)async_client_callback,(void*)ctx);

	if(ret)
	{
		perror("pthread_create!");
		close(ctx->epfd);
		free(ctx);
		return NULL;
	}

	return ctx;
}
//释放资源
static void async_client_uninit(struct async_context* ctx)
{
	close(ctx->epfd);
	pthread_cancel(ctx->thread_id);
	free(ctx);
}

struct http_request {
	char *hostname;
	char *resource;
};
struct http_request reqs[] = {
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=beijing&language=zh-Hans&unit=c" },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=changsha&language=zh-Hans&unit=c" },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=shenzhen&language=zh-Hans&unit=c" },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=shanghai&language=zh-Hans&unit=c" },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=tianjin&language=zh-Hans&unit=c" },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=wuhan&language=zh-Hans&unit=c" },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=hefei&language=zh-Hans&unit=c" },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=hangzhou&language=zh-Hans&unit=c" },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=nanjing&language=zh-Hans&unit=c" },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=jinan&language=zh-Hans&unit=c" },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=taiyuan&language=zh-Hans&unit=c" },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=wuxi&language=zh-Hans&unit=c" },
	{"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=suzhou&language=zh-Hans&unit=c" },
};

//每一个响应的回调函数
static void http_async_client_result_callback(const char *hostname, const char *result) {
	
	printf("hostname:%s, result:%s\n\n\n\n", hostname, result);
	
}

int main(int argc,char* argv[])
{
	struct async_context* ctx = async_client_init();
	if(ctx == NULL) return -1;

	int count = sizeof(reqs)/sizeof(reqs[0]);

	for(int i = 0;i < count;i++)
	{
		async_client_request(ctx,reqs[i].hostname,reqs[i].resource,http_async_client_result_callback);
	}

	getchar();
}
