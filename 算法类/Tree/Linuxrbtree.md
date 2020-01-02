### 红黑树特性Linux内存管理

有这么一句话：**如果对查找效率要求过高，而且又需要平凡的去增删改操作时就应该想到使用红黑树**！因为红黑树的特性就是：查找效率较高，增删效率也较高，自身是平衡的，没有AVL树强平衡，所以它在删除操作时要比AVL树优秀很多，红黑树来源于2-3树，它的五条性质都来源于2-3树，就是高度和宽度之间的博弈，而且每条性质都顺其自然。有兴趣可以研究一下，真的很有意思哦。



Linux内存管理模块使用红黑树来提升虚拟内存的查找速度，我们来看看Linux内核目录下的rbtree.c文件：

插入，基本就是红黑树的插入节点调整操作

```C
void rb_insert_color(rb_node_t * node, rb_root_t * root)
{
	rb_node_t * parent, * gparent;//一个父节点一个祖父节点
	//只有当父节点为红色的时候才需要调整
	while ((parent = node->rb_parent) && parent->rb_color == RB_RED)
	{
		gparent = parent->rb_parent;
 		//如果父节点是爷爷的左子节点，就去判断叔叔节点，是红色就变色操作，否则就进行旋转操作
		if (parent == gparent->rb_left)
		{
			{
				register rb_node_t * uncle = gparent->rb_right;
				if (uncle && uncle->rb_color == RB_RED)
				{
					uncle->rb_color = RB_BLACK;
					parent->rb_color = RB_BLACK;
					gparent->rb_color = RB_RED;
					node = gparent;
					continue;
				}
			}
 
			if (parent->rb_right == node)
			{
				register rb_node_t * tmp;
				__rb_rotate_left(parent, root);
				tmp = parent;
				parent = node;
				node = tmp;
			}
 
			parent->rb_color = RB_BLACK;
			gparent->rb_color = RB_RED;
			__rb_rotate_right(gparent, root);
		} else {//当父节点是右子节点的时候就反过来即可
			{
				register rb_node_t * uncle = gparent->rb_left;
				if (uncle && uncle->rb_color == RB_RED)
				{
					uncle->rb_color = RB_BLACK;
					parent->rb_color = RB_BLACK;
					gparent->rb_color = RB_RED;
					node = gparent;
					continue;
				}
			}
 
			if (parent->rb_left == node)
			{
				register rb_node_t * tmp;
				__rb_rotate_right(parent, root);
				tmp = parent;
				parent = node;
				node = tmp;
			}
 
			parent->rb_color = RB_BLACK;
			gparent->rb_color = RB_RED;
			__rb_rotate_left(gparent, root);
		}
	}
 	//最后把根节点颜色改变一下，保证性质一
	root->rb_node->rb_color = RB_BLACK;
}
```



删除操作有兴趣的可以去我的github上看一下<https://github.com/yc299792/C-/blob/master/%E7%AE%97%E6%B3%95%E7%B1%BB%2FTree%2Frbtreeerase.md>:

```C
void rb_erase(rb_node_t * node, rb_root_t * root)
{
	rb_node_t * child, * parent;
	int color;
 
	if (!node->rb_left)
		child = node->rb_right;
	else if (!node->rb_right)
		child = node->rb_left;
	else
	{
		rb_node_t * old = node, * left;
 
		node = node->rb_right;
		while ((left = node->rb_left))
			node = left;
		child = node->rb_right;
		parent = node->rb_parent;
		color = node->rb_color;
 
		if (child)
			child->rb_parent = parent;
		if (parent)
		{
			if (parent->rb_left == node)
				parent->rb_left = child;
			else
				parent->rb_right = child;
		}
		else
			root->rb_node = child;
 
		if (node->rb_parent == old)
			parent = node;
		node->rb_parent = old->rb_parent;
		node->rb_color = old->rb_color;
		node->rb_right = old->rb_right;
		node->rb_left = old->rb_left;
 
		if (old->rb_parent)
		{
			if (old->rb_parent->rb_left == old)
				old->rb_parent->rb_left = node;
			else
				old->rb_parent->rb_right = node;
		} else
			root->rb_node = node;
 
		old->rb_left->rb_parent = node;
		if (old->rb_right)
			old->rb_right->rb_parent = node;
		goto color;
	}
 
	parent = node->rb_parent;
	color = node->rb_color;
 
	if (child)
		child->rb_parent = parent;
	if (parent)
	{
		if (parent->rb_left == node)
			parent->rb_left = child;
		else
			parent->rb_right = child;
	}
	else
		root->rb_node = child;
 
 color:
	if (color == RB_BLACK)
		__rb_erase_color(child, parent, root);
}
```



调整操作：

```C
static void __rb_erase_color(rb_node_t * node, rb_node_t * parent,
			     rb_root_t * root)
{
	rb_node_t * other;
 
	while ((!node || node->rb_color == RB_BLACK) && node != root->rb_node)
	{
		if (parent->rb_left == node)
		{
			other = parent->rb_right;
			if (other->rb_color == RB_RED)
			{
				other->rb_color = RB_BLACK;
				parent->rb_color = RB_RED;
				__rb_rotate_left(parent, root);
				other = parent->rb_right;
			}
			if ((!other->rb_left ||
			     other->rb_left->rb_color == RB_BLACK)
			    && (!other->rb_right ||
				other->rb_right->rb_color == RB_BLACK))
			{
				other->rb_color = RB_RED;
				node = parent;
				parent = node->rb_parent;
			}
			else
			{
				if (!other->rb_right ||
				    other->rb_right->rb_color == RB_BLACK)
				{
					register rb_node_t * o_left;
					if ((o_left = other->rb_left))
						o_left->rb_color = RB_BLACK;
					other->rb_color = RB_RED;
					__rb_rotate_right(other, root);
					other = parent->rb_right;
				}
				other->rb_color = parent->rb_color;
				parent->rb_color = RB_BLACK;
				if (other->rb_right)
					other->rb_right->rb_color = RB_BLACK;
				__rb_rotate_left(parent, root);
				node = root->rb_node;
				break;
			}
		}
		else
		{
			other = parent->rb_left;
			if (other->rb_color == RB_RED)
			{
				other->rb_color = RB_BLACK;
				parent->rb_color = RB_RED;
				__rb_rotate_right(parent, root);
				other = parent->rb_left;
			}
			if ((!other->rb_left ||
			     other->rb_left->rb_color == RB_BLACK)
			    && (!other->rb_right ||
				other->rb_right->rb_color == RB_BLACK))
			{
				other->rb_color = RB_RED;
				node = parent;
				parent = node->rb_parent;
			}
			else
			{
				if (!other->rb_left ||
				    other->rb_left->rb_color == RB_BLACK)
				{
					register rb_node_t * o_right;
					if ((o_right = other->rb_right))
						o_right->rb_color = RB_BLACK;
					other->rb_color = RB_RED;
					__rb_rotate_left(other, root);
					other = parent->rb_left;
				}
				other->rb_color = parent->rb_color;
				parent->rb_color = RB_BLACK;
				if (other->rb_left)
					other->rb_left->rb_color = RB_BLACK;
				__rb_rotate_right(parent, root);
				node = root->rb_node;
				break;
			}
		}
	}
	if (node)
		node->rb_color = RB_BLACK;
}
```

