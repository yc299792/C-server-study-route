/*
*这是二叉搜索树的创建方式和基本操作
*/
#include<stdio.h>
#include<stdlib.h>
#include<assert.h>


typedef int KEY_TYPE;

//把树的操作分离出来
#define BTREE_ENTRY(name,type)\
struct name{				\
	struct type* left;\
	struct type* right;}


struct btreeNode {
	KEY_TYPE data;
	//这里是单独的数的操作
	BTREE_ENTRY(, btreeNode)bst;
};

struct btreeRoot {
	struct btreeNode* root;
};


struct btreeNode* createNode(KEY_TYPE data)
{
	struct btreeNode* node = (struct btreeNode*) malloc(sizeof(struct btreeNode));

	if (node == NULL)
	{
		return NULL;
	}
	node->data = data;
	node->bst.left = node->bst.right = NULL;
	return node;
}

int insertNode(struct btreeRoot* tree, KEY_TYPE data)
{
	assert(tree);

	if (tree->root == NULL)
	{
		tree->root = createNode(data);
		return 0;
	}

	struct btreeNode* node = tree->root;
	struct btreeNode* tmp = NULL;

	while (node)
	{
		tmp = node;
		//一直找比data更小的节点
		if (node->data < data)
		{
			node = node->bst.right;
		}
		else {
			node = node->bst.left;
		}
	}

	if (tmp->data > data)
	{
		tmp->bst.left = createNode(data);
	}
	else {
		tmp->bst.right = createNode(data);
	}
	return 0;


}

int btreeTravalsal(struct btreeNode* root)
{
	
	if (root == NULL) return 0;


	btreeTravalsal(root->bst.left);
	printf("%4d", root->data);

	btreeTravalsal(root->bst.right);

}


#define ARRSIZE 20

int main()
{
	int arr[ARRSIZE] = {24,25,13,35,23, 26,67,47,38,98, 20,13,17,49,12, 21,9,18,14,15 };
	struct btreeRoot tree = { 0 };
	for (int i = 0; i < ARRSIZE; i++)
	{
		insertNode(&tree, arr[i]);
	}
	btreeTravalsal(tree.root);
	system("pause");


}
