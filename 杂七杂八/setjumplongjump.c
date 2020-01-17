#include<setjmp.h>
#include<stdio.h>

jmp_buf env; 

void set_index(int itx)
{
	longjmp(env,itx);
}


int main(int argc, char const *argv[])
{

	int count = setjmp(env);

	if(count == 0)
	{
		printf("count:%d\n",count);
		set_index(++count);
	}else if(count == 1)
	{
		printf("count:%d\n",count);
		set_index(++count);

	}else if(count == 2)
	{
		printf("count:%d\n",count);
		set_index(++count);
	}
	return 0;
}
