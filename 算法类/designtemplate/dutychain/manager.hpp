#pragma once
#include"handle.h"

class Manager :public Bank
{
public:
	/*  1.常量成员，因为常量只能初始化不能赋值，所以必须放在初始化列表里面
		2.引用类型，引用必须在定义的时候初始化，并且不能重新赋值，所以也要写在初始化列表里面
		3. 没有默认构造函数的类类型，因为使用初始化列表可以不必调用默认构造函数来初始化，而是直接调用拷贝构造函数初始化
	*/
	//父类由于自己写了构造函数所以没有默认构造函数，子类构造函数之前会先调用父类构造函数，所以这里必须使用初始化列表
	Manager(string name, string title, Bank* next) //:Bank(name, title, next)
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
		else {
			return false;
		}
	}


protected:
	const int MAX_MONEY = 100000;
};
