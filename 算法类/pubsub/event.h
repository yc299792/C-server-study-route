
#ifndef NS_EVENT_H_
#define NS_EVENT_H_

#include "glo_def.h"
#include <string>

class iEvent
{
    public:
	virtual u16 getEid() const { return eid_; };
	virtual void setEid(u16 eid){ eid_ = eid; };
	virtual u32 getSN() const { return sn_; };
	virtual void setSN(u32 sn){ sn_ = sn; };
	virtual std::ostream& dump(std::ostream& out) const { return out; };
    virtual bool decode(){ return true; };
    virtual char* encode(){ return NULL; };
    iEvent(char* content);
	virtual ~iEvent();
    
    private:
        u16           eid_;    /* event id */
	    u32           sn_;     /* event sn */
    
    protected:
        std::string   content_; /* event content */

};

#endif

