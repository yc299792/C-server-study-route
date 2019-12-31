#include<iostream>
#include<assert.h>
#include"btree.h"

#define MAX_SIZE 512

/*
*创建一个二叉树，用字符串创建形如这样的字符串A(B(D,C)，G)
*用左括号表示接下来的是左节点
*右括号表示当前节点建立完毕
*,代表接下来是右节点
*/
int createTree(Node* &root, const char* str)
{
	Node* Stack[MAX_SIZE], *p = NULL;

	int j = 0, k, top = -1;
	char ch = str[j];
	assert(root == NULL);
	while (ch != '\0')
	{
		switch (ch)
		{
		//如果是左括号证明前面一个节点接下来需要接上左子树,先入栈
		case '(':Stack[++top] = p;k=1; break;
		//证明左右节点都完成了出栈
		case ')':top--; break;
		//证明接下来是右子树
		case ',':k = 2; break;
		default:
			p = (Node*)malloc(sizeof(Node));
			p->data = ch;
			p->left = p->right = NULL;
			//需要判断新节点是否为根节点,不是还得判断是栈中的左节点还是右节点
			if(root == NULL)
			{
				root = p;
			}
			else {
				switch (k)
				{
				case 1:Stack[top]->left = p; break;
				case 2:Stack[top]->right = p; break;

				}
			}

			break;
		}


		ch = str[++j];
	}
	return 0;
}


Node* findNode(Node* root, KEY_TYPE data)
{


	if (root == NULL) return NULL;
	if (root->data == data) return root;
	
	Node* p = NULL;
	//先找左边
	p = findNode(root->left, data);
	if (p != NULL)
	{
		//找到就返回
		return p;
	}
	else
	{
		//没找到就是右边了
		return findNode(root->right, data);
	}
	
}

int highTree(Node* root)
{
	if (root == NULL)
		return 0;
	//左树高就是根节点加上左子树，右树高就是右边加上右子树，返回较大的那一个
	return 1 + highTree(root->left) > 1 + highTree(root->right) ? 1 + highTree(root->left) : 1 + highTree(root->right);



}

void showTree(Node* root)
{
	if (root == NULL) return;

	std::cout << root->data;
	//有左或者右都需要先输出（，然后显示左，然后有右就显示，和右最后输出）
	if (root->left || root->right)
	{
		std::cout << '(';

		showTree(root->left);
		if (root->right)
		{
			std::cout << ',';
			showTree(root->right);
			
		}
		std::cout << ')';


	}

}


int main()
{
	Node* root = NULL;
	const char* str = "A(B(D(C,B),C),G)";

	createTree(root, str);

	//std::cout << findNode(root, 'C')->data << std::endl;
	//std::cout << highTree(root) << std::endl;
	showTree(root);
	system("pause");

}
