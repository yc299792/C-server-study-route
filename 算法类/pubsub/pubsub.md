### 发布订阅模式

发布订阅模式是一种借助中间人完成工作的思想，本来两个模块之间相互通信，然后这样的不好的地方就是耦合度比较高，如果通过一个中间人来处理他们的消息，那么他们之间的耦合度就降低了，也就是说两者现在都依赖中间人，这样的话就会方便扩展。

现在这里就有三个角色：

- 发布者，负责把消息发布给中间人
- 中间人，通常是一个消息队列，接受发布者的消息，然后把消息传递给订阅者
- 订阅者，订阅者订阅中间人的消息，收到消息后进行处理





单机版本的分析：

首先有一个单例：

```C
#define dms DispatchMsgService::getInstance()
```

这个就是上面的消息中间人，发布者和订阅者围绕它展开

这是订阅者操作需要提供事件的id和处理者，：

```C++
bool DispatchMsgService::subscribe(u16 eid, iEventHandler* handler)
{
    if (NULL == handler) return false;
    
    T_EventHandlersMap::iterator iter = subscribers_.find(eid);//查找eid消息类型
    if (iter != subscribers_.end())
    {//找到就把订阅者添加进来，存储的是map，eid和一个handle容器，在根据容器查找，没有订阅就加
        T_EventHandlers::iterator hdl_iter = std::find(iter->second.begin(), iter->second.end(), handler);
        if ( hdl_iter == iter->second.end())
        {
           iter->second.push_back(handler);
        }
    }
    else
    {
        subscribers_[eid].push_back(handler);//把新消息类型和这个一并加入map
    }
    
    return true;
}
```

发布事件的接口,只需要把事件加入到队列中即可完成发布：

```C++
bool DispatchMsgService::publish(iEvent* ev)
{
    if (NULL == ev)
    {
        return false;
    }

    return msg_queue_.enqueue(ev, 0);
}
```

程序开始我们只需要初始化一下中间人,其实就是创建一个线程去处理事件：

```C++
bool DispatchMsgService::init()
{
    /* create thread */
    int ret=pthread_create(&thread_hdl_, NULL, svc, this);//创建一个线程，并且把this传入就是传入自己
    if(ret !=0 )  
    {  
        printf("Create pthread error!\n");  
        return false;  
    }
    svr_exit_ = false;
    
    return true;
}
```

线程中的操作：

```C++
void* DispatchMsgService::svc(void* argv)//线程的处理函数是要把消息派发给订阅者的
{
    printf("dms is runing\n");

    DispatchMsgService* dmsvr = (DispatchMsgService*) argv;//argv就是派发者对象
    if (argv == NULL)
    {
        printf("parameter of thread is invalid.\n");
        return NULL;
    }
    
    while(!dmsvr->svr_exit_)//不用退出就循环
    {
        iEvent* ev = NULL;
        
        /* wait only 1 ms to dequeue */
        if (-1 == dmsvr->msg_queue_.dequeue(ev, 1))
        {
            continue;
        }
        
        std::stringstream ss;
        ev->dump(ss);//出队的消息转化为字符串
        printf("dequeue an event(%s).\n", ss.str().c_str());

        dmsvr->process(ev);//把这个事件交给订阅者，也就是回调订阅者的处理函数
        delete ev;//记得释放掉
    }

    dmsvr->subscribers_.clear();
    
    printf("dms quit.\n");

    return NULL;
}
```

主函数中是这样的：

```C++

int main(int argc, char** argv)
{
    dms->init();//会先拿到分发者对象，然后创建线程派发消息
    
    LoginEventHandler handler;//登陆消息的处理者
    handler.init();//订阅登录消息
    
    user panjinlian;
    panjinlian.login();//这个是用户登录了会提交一个登录事件
    
    for(;;)
    {
        sleep(1);
    }

    return 0;
}
```

整个流程就是：先创建中间人-》开线程发布消息给消息的订阅者，消息订阅者订阅，然后发布者给中间人发布了一个消息，那么这时中间人就会让订阅者处理。





