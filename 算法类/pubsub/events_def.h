
#ifndef NS_EVENTS_DEF_H_
#define NS_EVENTS_DEF_H_

#include "event.h"
#include "glo_def.h"


#include <string>
#include <map>
#include <vector>


class LoginEvent : public iEvent
{
    public : 
	LoginEvent(char* buffer);

        virtual ~LoginEvent(){};

        virtual std::ostream& dump(std::ostream& out) const;

        virtual bool decode();
        virtual char* encode();
        
        const std::string& getName(){ return name_; };
        const std::string& getPwd(){ return pwd_; };
       
    private :
        std::string name_;
        std::string pwd_;
};




#endif

