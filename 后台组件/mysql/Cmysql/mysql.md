### mysql

关于Mysql数据库应用非常广泛，是一种关系型数据库，提供持久化存储。

C++和C主要使用的是libmysql开发环境，安装也非常简单：

sudo apt-get install libmysqlclient-dev,安装完成后就可以使用这个库来编程了

![1578403310984](C:\Users\ADMINI~1\AppData\Local\Temp\1578403310984.png)



里面有很多的API：

```C
MYSQL ：mysql数据库连接句柄。在执行任何数据库操作之前首先就需要创建一个MYSQL结构。
MYSQL_RES ：执行查询语句（SELECT, SHOW, DESCRIBE, EXPLAIN）返回的结果。
MYSQL_ROW ：用来表示返回结果中的一行数据。由于每行数据格式不一致，因此使用此结构来统一表示。调用mysql_fetch_row()可从MYSQL_RES中返回一个MYSQL_ROW结构。
MYSQL_FIELD：用来表示一个field信息的元数据（元数据，即描述数据的数据），包括field name，field type以及field size等。
MYSQL_FIELD不包含field的值（MYSQL_ROW真正保存各field的值）。
MYSQL_FIELD_OFFSET： field在row中的索引值，从0开始
```

主要的操作：

```C
/* 创建一个mysql对象，后面所有的接口都依赖mysql */
MYSQL *mysql_init(MYSQL *mysql)
/* 连接mysql数据库 */
MYSQL *mysql_real_connect(MYSQL *mysql, const char *host, const char *user, const char *passwd, 
const char *db, unsigned int port, const char *unix_socket, unsigned long client_flag);
/* 执行mysql语句 */
int mysql_real_query(MYSQL *mysql, const char *stmt_str, unsigned long length);
/* 执行完查询语句后，获取结果集 */
MYSQL_RES *mysql_store_result(MYSQL *mysql);
/* 对结果集的操作*/
MYSQL_ROW mysql_fetch_row(MYSQL_RES *result);
my_ulonglong mysql_num_rows(MYSQL_RES *res);
unsigned int mysql_num_fields(MYSQL_RES *res);
/* 一行数据的结果，通过row[i]去获取值，如果是整形，还得去转换 */
typedef char **MYSQL_ROW;
/* 释放结果集的空间 */
void mysql_free_result(MYSQL_RES *result);
/* 关闭mysql连接，释放mysql句柄 */
void STDCALL mysql_close(MYSQL *sock);
```

这就是主要的一些函数操作，虽然不像python那么好用，python的数据库操作采用ORM（对象关系映射），，是非常简单的，但是这里也还挺好用的，这里提供一个C++的封装，主要就是一个处理连接的主类，和一个处理返回集合的类，总体也是比较简单的操作。

#### 事务的四个特性：ACID

原子性：一个事务必须当成一个整体去执行，要不都成功，要不就都不成功。

一致性：数据必须从一种一致性状态转移到另一种一致性状态，比如转账，一个少了，那么另一个地方就增加了。

隔离性：事务隔离有四种级别，隔离性是指是否允许其他的session见到这个事务的状态。

持久性：事务执行成功就会持久保存下数据。



四种隔离级别：

1. READ UNCOMMITTED可能会发生脏读，因为这种级别，未提交就对其他事务可见，也就是说，可能别人读去的数据不一定正确
2. READ COMMITTED可能会发生不可重复读，第一次读的时候，还没有执行完，所以是之前的状态，第二次读的结果却发生了变化，所以是不可重复读。
3. REPEATABLE READ可能会发生幻读，这种情况是，你在执行sql时让你在事务前后都一样，但实际上却改变了数据，所以你第二次读的数据不正确，就是幻读了。
4. SERIALIZABLE，这种操作解决了所有的问题，但是需要对数据库进行锁操作，虽然不会产生问题，但是会造成数据库性能大大折扣

总结：数据库的事务操作就是平衡好业务和事务隔离级别的关系。



**Mysql的存储过程：**

就是通过

create procedure 存储过程名字 ( [in|out|inout] 参数 datatype )

begin

MySQL 语句;

end;

设置一个类似函数的东西可以通过call procedure_name(参数列表)调用，这样就是把一些关联的sql语句一次执行，因为数据库传输sql语句是增加带宽的，因为要网络传输，所以这样做有一定的好处。



Redis的lua操作，redis可以通过lua脚本一次执行多条命令，这就和上面的存储过程有一致的思想。