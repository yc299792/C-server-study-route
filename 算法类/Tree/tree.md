### 树介绍

树是一种很常用的数据结构，在很多地方都能看到树的应用，因为树往往可以用来优化数据查找的效率。

在一些优秀的系统中就会经常用到各种树结构，比如红黑树、AVL树、B树等。



### 红黑树的应用：

##### 原理：

- 根节点是黑色的
- 只有红黑节点
- 叶节点都是黑色的nil
- 红色节点子节点必须是黑色的
- 对每个节点，从该节点到叶子节点路径上的黑节点数目一致，黑平衡性质

**以上的五个性质保证了红黑树是平衡的，所以红黑树的查找效率能够稳定在lgn**

##### STL模板库中map和set的底层实现：

### 红黑树中节点和迭代器的实现

所有这些优秀的底层设计都会把结构和数据分离

```C++
struct __rb_tree_node_base

{

typedef __rb_tree_color_type color_type;

typedef __rb_tree_node_base* base_ptr;

color_type color; // 红黑树的颜色

base_ptr parent; // 父节点

base_ptr left; // 左子节点

base_ptr right; // 右子节点

}

template <class Value>

struct __rb_tree_node : public __rb_tree_node_base

{

typedef __rb_tree_node<Value>* link_type;

Value value_field; // 存储数据

};
```

map中迭代器为**struct** __rb_tree_base_iterator,主要成员就是一个__rb_tree_node_base节点,指向树中某个节点,作为迭代器与树的连接关系,还有两个方法,用于将当前迭代器指向前一个节点decrement()和下一个节点increment().源码:

```C++
template <class Value, class Ref, class Ptr>

struct __rb_tree_iterator : public __rb_tree_base_iterator

{

typedef Value value_type;

typedef Ref reference;

typedef Ptr pointer;

typedef __rb_tree_iterator<Value, Value&, Value*> iterator;

typedef __rb_tree_iterator<Value, const Value&, const Value*> const_iterator;

typedef __rb_tree_iterator<Value, Ref, Ptr> self;

typedef __rb_tree_node<Value>* link_type;

__rb_tree_iterator() {}//迭代器默认构造函数

__rb_tree_iterator(link_type x) { node = x; }//由一个节点来初始化迭代器

__rb_tree_iterator(const iterator& it) { node = it.node; }//迭代器复制构造函数

//迭代器解引用,即返回这个节点存储的数值,通过重载运算符的方式实现

reference operator*() const { return link_type(node)->value_field; }

#ifndef __SGI_STL_NO_ARROW_OPERATOR

//返回这个节点数值值域的指针，就是重载了->运算符

pointer operator->() const { return &(operator*()); }

#endif /* __SGI_STL_NO_ARROW_OPERATOR */

//迭代器++运算

self& operator++() { increment(); return *this; }

self operator++(int) {

self tmp = *this;

increment();

return tmp;

}

//迭代器--运算

self& operator--() { decrement(); return *this; }

self operator--(int) {

self tmp = *this;

decrement();

return tmp;

}

};

inline bool operator==(const __rb_tree_base_iterator& x,

const __rb_tree_base_iterator& y) {

return x.node == y.node;

// 两个迭代器相等,指这两个迭代器指向的节点相等

}

inline bool operator!=(const __rb_tree_base_iterator& x,

const __rb_tree_base_iterator& y) {

return x.node != y.node;

// 两个节点不相等,指这两个迭代器指向的节点不等

}
```

迭代器都会有解引用操作，本质就是重载了*运算符，这里对于set返回的就是set存储的value值，对于map而言则是pair<key,value>键值对，当然无论是set还是value底层都是用红黑树实现只是存储的值不一样而已。

STL底层对红黑树基本操作的封装：

```C++
inline void __rb_tree_rotate_left(__rb_tree_node_base* x, __rb_tree_node_base*& root)//左旋

inline void __rb_tree_rotate_right(__rb_tree_node_base* x, __rb_tree_node_base*& root)//右旋

inline void __rb_tree_rebalance(__rb_tree_node_base* x, __rb_tree_node_base*& root)//插入之后调节

inline __rb_tree_node_base* __rb_tree_rebalance_for_erase(__rb_tree_node_base* z,
__rb_tree_node_base*& root,

__rb_tree_node_base*& leftmost,

__rb_tree_node_base*& rightmost)//删除之后调节
```

这些操作就是红黑树保持平衡所必备的操作。



**红黑树的真正定义：**

```C++
//封装了节点的分配地址过程

link_type get_node() { return rb_tree_node_allocator::allocate(); }

//释放一个节点

void put_node(link_type p) { rb_tree_node_allocator::deallocate(p); }

//创建一个节点

link_type create_node(const value_type& x) {

link_type tmp = get_node(); // 先分配空间

__STL_TRY {

construct(&tmp->value_field, x); // 然后对节点的值进行赋值

}

__STL_UNWIND(put_node(tmp));

return tmp;

}

link_type clone_node(link_type x) { // 主要复制一个节点，主要复制了颜色

link_type tmp = create_node(x->value_field);

tmp->color = x->color;

tmp->left = 0;

tmp->right = 0;

return tmp;

}

void destroy_node(link_type p) {

destroy(&p->value_field); // 这里是回收节点中存储内容的内存

put_node(p); // 回收节点的内存

}
```

树的主要成员有三个：

```C++
size_type node_count; // 记录树的大小(节点的个数)

link_type header;//红黑树的根节点

Compare key_compare; // 节点间的比较器,这个是用户使用的时候必须提供的，预留接口
```

当然还提供了很多比较小的函数，为一些操作提供方便。



**map的实现**

map底层就是红黑树，就是对红黑树再加上了一层封装：

```C++
typedef Key key_type; //键值类型

typedef T data_type; // 值类型

typedef T mapped_type; //值类型

typedef pair<const Key, T> value_type; // 这个就是值类型,map存储pair键值对，set存储的是单独的值

typedef Compare key_compare; // 键值函数对象

class value_compare//值域比较器,其实也是比较键值

: public binary_function<value_type, value_type, bool> {

friend class map<Key, T, Compare, Alloc>;

protected :

Compare comp;

value_compare(Compare c) : comp(c) {}

public:

bool operator()(const value_type& x, const value_type& y) const {

return comp(x.first, y.first);

}

};

//map中红黑树的定义

typedef rb_tree<key_type, value_type,

select1st<value_type>, key_compare, Alloc> rep_type;

rep_type t;
```

注意一下map中用于比较有序的是pair的第一个值key，用key来进行排序。剩下的就是一下增删改查操作基本就是一些封装什么的，C++中泛型编程的思想就是使用类模板，但是在C语言中值的不同类型就需要通过void* 来实现了。

**SET的接口几乎和map是一模一样的！**

**总结：**

红黑树优秀的查找效率让map、set这样的容器选择使用它来实现，在进行自平衡时红黑树的效率也是优于平衡二叉树的，所以红黑树在很多时候都被这些所依赖。

