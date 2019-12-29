#include<stdio.h>
#include<stdlib.h>
#include<assert.h>


#define SIZE 512

typedef int KEY_TYPE;


typedef struct
{
	KEY_TYPE* base;
	int top;
	int stack_size;
}Stack;


int initStack(Stack* s)
{

	s->base = (KEY_TYPE*)calloc(SIZE,sizeof(KEY_TYPE));
	s->top = 0;
	s->stack_size = SIZE;
	
	return 0;
}

void pushStack(Stack* s,KEY_TYPE value)
{
	assert(s != NULL);

	if (s->top >= s->stack_size)
	{
		s->base = (KEY_TYPE*)realloc(s->base,(SIZE+s->stack_size)*sizeof(KEY_TYPE));
		assert(s->base);
		s->stack_size += SIZE;

	}

	s->base[s->top++] = value;
	

}

bool isEmpty(Stack* s)
{
	assert(s);
	return s->top == 0 ? true : false;
}



void popStack(Stack* s,KEY_TYPE* data)
{
	if (isEmpty(s)) return;

	*data = s->base[--s->top];
}

int sizeStack(Stack* s)
{
	return s->top;
}


void destroyStack(Stack* s)
{
	if (s == NULL) return;
	assert(s != NULL);
	free(s->base);
	s->base = NULL;
}


int main()
{
	Stack s;
	initStack(&s);
	KEY_TYPE data;

	for (int i = 1; i <= 100; i++)
	{
		pushStack(&s, i);
	}

	printf("%4d\n",s.stack_size);

	for (int i = 1; i <= 500; i++)
	{
		pushStack(&s, i);
	}

	printf("%4d\n", s.stack_size);


	destroyStack(&s);

	system("pause");



}
