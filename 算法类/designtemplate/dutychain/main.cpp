#include<iostream>
#include"request.h"
#include"employee.h"
#include"Manager.h"


int main()
{
	Request req("zhangsan",20,8000);
	Request req1("zhanglong",20,88888);
	Manager manager("lisi", "manager", nullptr);

	Employee e("wangwu", "employee", &manager);

	e.handleReuest(req);

	e.handleReuest(req1);

	system("pause");


}
