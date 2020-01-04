
#include <stdio.h>
#include <unistd.h>

#include "events_def.h"
#include "DispatchMsgService.h"
#include "LoginEventHandler.h"

class user
{
public:
    bool login()
    {
        printf("try to login.\n");
        
        char msg[] = "{\"cmd\":\"login\",\"sn\":1,\"name\":\"pan jinlian\",\"password\":\"xiao neiku\"}";
        LoginEvent* ev = new LoginEvent(msg);
        ev->decode();

        dms->publish(ev);//发布一个登录事件
        
        return true;
    }
    
    user()
    {
    
    }
    
    virtual ~user(){};
};

int main(int argc, char** argv)
{
    dms->init();//会先拿到分发者对象，然后创建线程派发消息
    
    LoginEventHandler handler;
    handler.init();//订阅登录消息
    
    user panjinlian;
    panjinlian.login();//这个是用户登录了会提交一个登录事件
    
    for(;;)
    {
        sleep(1);
    }

    return 0;
}

