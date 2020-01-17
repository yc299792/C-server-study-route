#include<setjmp.h>
#include<stdio.h>

jmp_buf env;

#define Try   int exception = setjmp(env); if(exception == 0)

#define Catch(e) else if(exception == (e))

#define Finally

#define Throw(e) longjmp(env,(e))

int main()
{
	
	Try{
		Throw(2); 
	}Catch(1){
		printf("catch:1\n");
	}Catch(2){
		printf("catch:2\n");
	}Finally{
		printf("catch:\n");

	}
}
