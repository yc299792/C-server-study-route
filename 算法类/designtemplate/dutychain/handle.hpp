#pragma once
#include<iostream>
#include"request.h"

class Bank{

public:

	//Bank(string name, string title, Bank* next) 
	//{
	//	this->person_name = name;
	//	this->title_name = title;
	//	this->nexthanler = next;
	//}
	virtual bool isPermision(Request& req)=0;//为抽象类供继承

	virtual void handleReuest(Request& req)
	{
		if (isPermision(req))
		{
			
			cout << person_name<<"    " << "you can recive money two hours later" << endl;
		}
		else {
			nexthanler->handleReuest(req);
		}
	}


protected:
	string person_name;//处理人的名字
	string title_name;//岗位名


	Bank*	nexthanler;//上级处理者
};
