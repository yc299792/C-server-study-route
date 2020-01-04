
#ifndef NS_LOGIN_EVENT_HANDLER_H_
#define NS_LOGIN_EVENT_HANDLER_H_

#include "iEventHandler.h"
#include "event.h"
#include "eventid.h"


class LoginEventHandler : public iEventHandler
{
    public:
	
	LoginEventHandler();
        virtual ~LoginEventHandler();
        void init();
        void uninit();
        virtual bool handle(const iEvent* ev);

    protected:

    private:

};


#endif

