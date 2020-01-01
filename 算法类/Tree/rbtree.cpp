#include<stdio.h>
#include<assert.h>

#define RED 0
#define BLACK 0

typedef int KEY_TYPE;

typedef struct rbTreeNode{
	KEY_TYPE key;
	void* value;

	//这里可以再做一层封装,把树的操作独立出来
	unsigned char color;
	struct rbTreeNode* parent;
	struct rbTreeNode* left;
	struct rbTreeNode* right;
	
}rbTreeNode;

typedef struct {
	rbTreeNode* root;
	rbTreeNode* nil;
}rbTree;

//左旋操作，对当前节点进行左旋操作就是把当前节点变成自己的左儿子，然后把右儿子变成父亲节点，本质就是把右边多的节点向左边转移
void rbTreeLeftRotate(rbTree* T,rbTreeNode* x)
{
	rbTreeNode* y = x->right;
	
	x->right = y->left; //先让x右指向y的左节点
	//让y的左节点找到父亲
	if (y->left != T->nil)
	{
		y->left->parent = x;
	}

	y->parent = x->parent;
	//如果x的父亲是空的那么,就代表y应该就是根节点
	if (x->parent == T->nil)
	{
		T->root = y;
	}
	else if (x == x->parent->left)
	{
		x->parent->left = y;
	}
	else if (x == x->parent->right)
	{
		x->parent->right = y;
	}

	y->left = x;
	x->parent = y;
}
//右旋操作就是把左旋中的x，y互换，left，right呼唤即可，就是在左树的子节点更多时，把节点向右边转移
void rbTreeRightRotate(rbTree *T, rbTreeNode *y) {

	rbTreeNode *x = y->left;

	y->left = x->right;
	if (x->right != T->nil) {
		x->right->parent = y;
	}

	x->parent = y->parent;
	if (y->parent == T->nil) {
		T->root = x;
	}
	else if (y == y->parent->right) {
		y->parent->right = x;
	}
	else {
		y->parent->left = x;
	}

	x->right = y;
	y->parent = x;
}

void insertNodeFixup(rbTree* T, rbTreeNode* z)
{
	//只有当z的父节点是红色才进行调整
	while (z->parent->color == RED)
	{
		//如果父节点是祖父的左孩子
		if (z->parent == z->parent->parent->left)
		{
			//接下来去判断叔叔节点
			rbTreeNode* y = z->parent->parent->right;
			if (y->color = RED)
			{
				//叔叔是红色爹是红色，那就变色,然后把祖父节点设置为当前要插入的节点
				z->parent->color = BLACK;
				y->color = BLACK;
				z->parent->parent->color = RED;
				z = z->parent->parent;
			}
			else {
				//叔叔节点是黑色的，那么就要往叔叔这边旋转，旋转之前再判断是否需要拉直爷爷的子链
				if (z == z->parent->right) {
					//如果当前节点是右节点直接左旋，这部操作是为了拉直分支
					z = z->parent;
					rbTreeLeftRotate(T, z);
				}

				//把父亲变黑，把祖父变红，然后右旋
				z->parent->color = BLACK;
				z->parent->parent->color = RED;
				rbTreeRightRotate(T, z->parent->parent);
			}
		}
		else {//按照相反的操作进行
			rbTreeNode* y = z->parent->parent->left;
			if (y->color == RED) {
				z->parent->color = BLACK;
				y->color = BLACK;
				z->parent->parent->color = RED;

				z = z->parent->parent; //z --> RED
			}
			else {
				//叔叔节点是红色的，先进行拉直操作
				if (z == z->parent->left) {
					z = z->parent;
					rbTreeRightRotate(T, z);
				}

				z->parent->color = BLACK;
				z->parent->parent->color = RED;
				rbTreeLeftRotate(T, z->parent->parent);
			}
		}
		
	
	}

	T->root->color = BLACK;
}


void insertrbTreeNode(rbTree* T, rbTreeNode* z)
{
	rbTreeNode* y = T->nil;
	rbTreeNode* x = T->root;

	//先去找到插入节点的位置,当x不等于叶子节点时nil时
	while (x != T->nil)
	{
		y = x;

		if (z->key < x->key)
		{
			x = x->left;
		}
		else if(z->key > x->key)
		{
			x = x->right;
		}
		else
		{
			//这就是红黑树不允许出现相同元素时的做法，集合就是这样实现的
			return;
		}
	}

	//循环退出y就是插入的父节点位置
	if (y == T->nil)
	{
		//证明没有循环，也就是说红黑树是空树
		T->root = z;
	}
	else if (z->key < y->key)
	{
		y->left = z;
	}
	else {
		y->right = z;
	}

	//以z为当前节点去自调整让红黑树平衡
	z->left = T->nil;
	z->right = T->nil;
	z->color = RED;
	insertNodeFixup(T, z);
}
