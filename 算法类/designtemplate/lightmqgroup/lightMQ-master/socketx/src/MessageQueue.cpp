
#include "MessageQueue.hpp"
#include<ctime>

namespace socketx
{
    ssize_t MessageQueue::send()
    {
        //if(sendList.size()==0) return -1;
        if(sendGroup.size()==0) return -1;
        Message msg;
		int reciveid = 0;
        bool flag = queue_.try_pop(msg);
        if(flag)
        {
            /*for(auto it=sendList.begin(); it!=sendList.end(); ++it)
            {
                (*it)->sendmsg(msg);
            }*/
            srand((int)time(NULL));
            
			for(auto it=sendGroup.begin(); it!=sendGroup.end(); ++it)
            {	
            	reciveid = rand()%(it->size());//从组里随机取一个处理订阅者处理
                (*it)[reciveid]->sendmsg(msg);
            }
            return 1;
        }
        
        return -1;
    }

    void MessageQueue::recv(Message msg)
    {
        queue_.push(msg);
    }

    void MessageQueue::addConnection(std::shared_ptr<Connection> conn)
    {
    	int groupid = conn->getGroupId();
		if(groupid < sendGroup.size())
		{
			if(std::find(sendGroup[groupid].begin(),sendGroup[groupid].end(),conn)==sendGroup[groupid].end())
        	{
            	sendGroup[groupid].push_back(conn);
        	}
			else
        	{
           	    printf("This connection is existed...\n");
        	}
		}
		else
		{
			groupid = sendGroup.size(); 
			conn->setGroupId(groupid);//不管咋样id不能超过size
			sendGroup.push_back(std::vector<std::shared_ptr<Connection>> ());//添加一个组
			sendGroup[groupid].push_back(conn);//加入新组
		}
      /*  if(std::find(sendList.begin(),sendList.end(),conn)==sendList.end())
        {
            sendList.push_back(conn);
        }
        else
        {
            printf("This connection is existed...\n");
        }*/
    }
    
    void MessageQueue::removeConnection(std::shared_ptr<Connection> conn)
    {
    	int groupid = conn->getGroupId();

		if(groupid < sendGroup.size())
		{
			auto it = std::find(sendGroup[groupid].begin(),sendGroup[groupid].end(),conn);
			if(it == sendGroup[groupid].end())
			{
				printf("No such a connection in MessageQueue...\n");
			}
			else
			{
				sendGroup[groupid].erase(it);
			}
		}
		else
		{
			printf("error groupid,plese check...\n");
		}
       /* auto it = std::find(sendList.begin(),sendList.end(),conn);
        if(it==sendList.end())
        {
            printf("No such a connection in MessageQueue...\n");
        }
        else
        {
            sendList.erase(it);
        }*/
    }
}
