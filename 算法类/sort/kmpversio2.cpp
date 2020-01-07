#include<iostream>
#include<string.h>
#include<stdlib.h>
#include<assert.h>

void getNext(const char* p,int next[])
{
	int k = -1;
	int j = 0;
	next[0] = -1;

	int size = strlen(p);

	while(j < size - 1)
	{
		//p[k]前缀,p[j]后缀
		if(k == -1 || p[k] == p[j])
		{
			k++;
			j++;
			if(p[k] != p[j])//排除k等于-1的情况
				next[j] = k;
			else
				next[j] = next[k];//相等就递归一下求next[j]
		}
		else
		{
			k = next[k];//递归调用p[next[k]]的位置找p[j]
		}
	}
	


}

int kmpNextStrIndex(const char* src,const char* pattern)
{
	assert(src && pattern);


	int len1 = strlen(src);
	int len2 = strlen(pattern);


	int i = 0,j = 0;
	
	int* next = (int*)malloc(len2*sizeof(int));

	getNext(pattern,next);
	
	while(i < len1 && j < len2)
	{
		if(j == -1 || src[i] == pattern[j])
		{
			i++;
			j++;
		}
		else
		{
			j = next[j];
		}

		
	}

	if(j == len2)
	{
		return i-j;
	}
	else
	{
		return -1;
	}
	

}

int main()
{
	const char* src = "abaabbdsafasdasdfaaab";
	const char* pattern = "aaab";

	std::cout << kmpNextStrIndex(src,pattern)<<std::endl;
}



















