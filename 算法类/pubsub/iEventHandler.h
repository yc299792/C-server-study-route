
#ifndef NS_IEVENT_HANDLER_H_
#define NS_IEVENT_HANDLER_H_

#include "event.h"

class iEventHandler
{
public:
    virtual bool handle(const iEvent* ev) = 0;
    virtual ~iEventHandler() {};
    iEventHandler(){};
};

#endif

