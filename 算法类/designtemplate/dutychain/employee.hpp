#pragma once
#include"handle.h"

class Employee :public Bank
{
public:
	//这里必须使用初始化列表去调用父类的构造函数，因为父类没有其他构造函数
	Employee(string name,string title,Bank* next)//:Bank(name,title,next)
	{
		person_name = name;
		title_name = title;
		nexthanler = next;
	}

	bool isPermision(Request& req)
	{
		if (req.getRequestMoney() <= MAX_MONEY)
		{
			return true;
		}
		else
		{
			return false;
		}
	}




protected:
	const int MAX_MONEY = 10000;
};
