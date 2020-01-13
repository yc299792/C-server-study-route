//
//  Shows how to handle Ctrl-C
//
#include <zmq.h>
#include <stdio.h>
#include <signal.h>
//  ---------------------------------------------------------------------
//  消息处理
//
//  程序开始运行时调用s_catch_signals()函数；
//  在循环中判断s_interrupted是否为1，是则跳出循环；
//  很适用于zmq_poll()。
static int s_interrupted = 0;
static void s_signal_handler (int signal_value)
{
    s_interrupted = 1;
}
//设置信号的基本信息
static void s_catch_signals (void)
{
    struct sigaction action;
    action.sa_handler = s_signal_handler;
    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
}

int main (void)
{
    void *context = zmq_init (1);
    void *socket = zmq_socket (context, ZMQ_REP);
    zmq_bind (socket, "tcp://*:5555");
    s_catch_signals ();
    while (1) {
        //  阻塞式的读取会在收到信号时停止
        zmq_msg_t message;
        zmq_msg_init (&message);
        zmq_recv (socket, &message, 0);
        if (s_interrupted) {//发生中断时，s_interrupted 会被设置为1
            printf ("W: 收到中断消息，程序中止...\n");
            break;
        }
    }
    zmq_close (socket);
    zmq_term (context);
    return 0;
}
