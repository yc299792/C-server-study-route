
#include "LoginEventHandler.h"
#include "DispatchMsgService.h"
#include "events_def.h"

#include <stdio.h>

LoginEventHandler::LoginEventHandler()
{
}

LoginEventHandler::~LoginEventHandler()
{
}

void LoginEventHandler::init()
{
    dms->subscribe(EID_LOGIN_REQ, this);

}

void LoginEventHandler::uninit()
{
    dms->unsubscribe(EID_LOGIN_REQ, this);
}


bool LoginEventHandler::handle(const iEvent* ev)
{
    printf("enter handle method.\n");

    if (EID_LOGIN_REQ == ev->getEid())
    {
        LoginEvent* req = (LoginEvent*)ev;

        //do somethings
        // ..........
        
        printf("usr(%s)'s login success.\n", req->getName().c_str());
    }
    else
    {
        printf("unsubscribed events\n");
    }

    return true;
}


