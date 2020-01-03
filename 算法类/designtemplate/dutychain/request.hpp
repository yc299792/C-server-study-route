#pragma once

#include<string>
#include<iostream>
using namespace std;
class Request {

public:
	Request(string name,int age,int request_money):name(name),age(age),request_money(request_money)
	{

	}
	
	int getRequestMoney()
	{
		return request_money;
	}

private:
	string name;
	int age;
	int request_money;

};
