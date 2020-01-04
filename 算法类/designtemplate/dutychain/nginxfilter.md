### 过滤器模式-Nginx是怎么用的

#### 过滤器模式：

什么是过滤器大家应该都知道，常见的就是对自来水一层层过滤，那么抽象出来的话，就是有一个流，然后层层把关，每一次都筛选。在我们的软件设计中经常会有这样的应用，比如对一些信号的拦截，也是过滤器模式的思想。再或者一些网络框架中的中间件设计，利用中间件对数据进行处理也是过滤器思想。

那么Nginx中怎么使用这种思想的，Nginx中利用过滤器来处理http的response响应：

首先它是这么做的，每一个过滤器都独立成为一个模块：

ngx_http_not_modified_filter_module
默认打开，如果请求的if-modified-since等于回复的last-modified间值，说明回复没有变化，清空所有回复的内容，
返回304。
ngx_http_range_body_filter_module 默认打开，只是响应体过滤函数，支持range功能，如果请求包含range请求，那就只发送range请求的一段内容。
ngx_http_copy_filter_module 始终打开，只是响应体过滤函数，主要工作是把文件中内容读到内存中，以便进行处理。
ngx_http_headers_filter_module 始终打开，可以设置expire和Cache-control头，可以添加任意名称的头
ngx_http_userid_filter_module 默认关闭，可以添加统计用的识别用户的cookie。
ngx_http_charset_filter_module 默认关闭，可以添加charset，也可以将内容从一种字符集转换到另外一种字符集，不支持多字节字符集。
ngx_http_ssi_filter_module 默认关闭，过滤SSI请求，可以发起子请求，去获取include进来的文件
ngx_http_postpone_filter_module 始终打开，用来将子请求和主请求的输出链合并
ngx_http_gzip_filter_module 默认关闭，支持流式的压缩内容
ngx_http_range_header_filter_module 默认打开，只是响应头过滤函数，用来解析range头，并产生range响应的头。
ngx_http_chunked_filter_module 默认打开，对于HTTP/1.1和缺少content-length的回复自动打开。
ngx_http_header_filter_module 始终打开，用来将所有header组成一个完整的HTTP头。
ngx_http_write_filter_module 始终打开，将输出链拷贝到r->out中，然后输出内容。

**有很多个这样的模块，每一个模块就是一个过滤器**

```C
ngx_module_t  ngx_http_write_filter_module = {
    NGX_MODULE_V1,
    &ngx_http_write_filter_module_ctx,     /* module context */ //每一个过滤器模块都要有一个这样结构体，结构体提供了函数指针为模块初始化使用
    NULL,                                  /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};
```

ngx_module_t，每一个模块就是一个这样的结构体类型，用于存储每一个模块的共有信息，这也是一种抽象就是把大家共有的抽象出来，然后ngx_http_write_filter_module_ctx是具体模块的信息，也是一个结构体，它是这样的：

```C
static ngx_http_module_t  ngx_http_write_filter_module_ctx = {//这个就是下面的类型的一个实例
    NULL,                                  /* preconfiguration */
    ngx_http_write_filter_init,            /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    NULL,                                  /* create location configuration */
    NULL,                                  /* merge location configuration */
};

typedef struct {
    ngx_int_t   (*preconfiguration)(ngx_conf_t *cf);
    ngx_int_t   (*postconfiguration)(ngx_conf_t *cf);//该函数就是一个后面初始化的函数指针，回调

    void       *(*create_main_conf)(ngx_conf_t *cf);
    char       *(*init_main_conf)(ngx_conf_t *cf, void *conf);

    void       *(*create_srv_conf)(ngx_conf_t *cf);
    char       *(*merge_srv_conf)(ngx_conf_t *cf, void *prev, void *conf);

    void       *(*create_loc_conf)(ngx_conf_t *cf);
    char       *(*merge_loc_conf)(ngx_conf_t *cf, void *prev, void *conf);
} ngx_http_module_t;
```

这里只放了一个ngx_http_write_filter_init，那么它是什么呢，是一个函数指针，这个函数一会会被调用对每一个模块初始化。



下面来看看是怎么初始化每一个模块的,在ngx_http.c中有一个函数：static char *
ngx_http_block，这是里面的一段代码：

```C
	//这里开始构建过滤器链
    for (m = 0; cf->cycle->modules[m]; m++) {
        if (cf->cycle->modules[m]->type != NGX_HTTP_MODULE) {//判断这个模块是不是http的模块，是就是过滤器
            continue;
        }
		//这给取出的module就是一个ngx_http_module_t类型的指针
        module = cf->cycle->modules[m]->ctx;
//通过函数指针回调
        if (module->preconfiguration) {//这个函数是让一个全局的ngx_http_top_body_filter指针指向本模块
            if (module->preconfiguration(cf) != NGX_OK) {
                return NGX_CONF_ERROR;
            }
        }
    }
```

比如：

```C
static ngx_int_t
ngx_http_write_filter_init(ngx_conf_t *cf)
{
    ngx_http_top_body_filter = ngx_http_write_filter;//这个模块是第一个模块

    return NGX_OK;
}
```

这里会有点头大，其实这里就是Nginx的设计这里的精华，特别的美妙：

 ngx_http_top_body_filter这个是一个指针，对就是指向消息体的头指针还有一个头指针 ngx_http_top_header_filter，那么这里是第一个模块，所以直接把头指针指向这里就行了。

那么是怎么变成一个链状结构的，就是下面这样的：

```C
static ngx_int_t
ngx_http_chunked_filter_init(ngx_conf_t *cf)
{
    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter = ngx_http_chunked_header_filter;

    ngx_http_next_body_filter = ngx_http_top_body_filter;
    ngx_http_top_body_filter = ngx_http_chunked_body_filter;

    return NGX_OK;
}
```

**每一模块初始化的这个回调函数在被调用的时候，先让next指向头，再让头指针指向自己，这样循环下来就成了一个链状结构。这里不惊感叹一下，一个链表的头插法居然还能这么用，太强了！**



最后是这样遍历链表的：

一个消息头：

```C
ngx_int_t
ngx_http_send_header(ngx_http_request_t *r)
{
    if (r->post_action) {
        return NGX_OK;
    }

    if (r->header_sent) {
        ngx_log_error(NGX_LOG_ALERT, r->connection->log, 0,
                      "header already sent");
        return NGX_ERROR;
    }

    if (r->err_status) {
        r->headers_out.status = r->err_status;
        r->headers_out.status_line.len = 0;
    }

    return ngx_http_top_header_filter(r);//遍历消息头的过滤器
}

```

一个消息体：

```C
ngx_int_t
ngx_http_output_filter(ngx_http_request_t *r, ngx_chain_t *in)
{
    ngx_int_t          rc;
    ngx_connection_t  *c;

    c = r->connection;

    ngx_log_debug2(NGX_LOG_DEBUG_HTTP, c->log, 0,
                   "http output filter \"%V?%V\"", &r->uri, &r->args);

    rc = ngx_http_top_body_filter(r, in);//遍历消息体的过滤器链表

    if (rc == NGX_ERROR) {
        /* NGX_ERROR may be returned by any filter */
        c->error = 1;
    }

    return rc;
}


```

#### 后话：

nginx这里是先把一个个过滤器独立成一个个模块，然后把这些模块抽象成一个共有的结构体，这个结构体是用来记录每一个模块的共有信息，然后每一个具体的模块又是一个结构体， ngx_http_module_t就是这样的类型，这个结构体提供给每一个模块一些函数指针以供回调，具体模块只要实现这些函数就行了，nginx会通过函数指针调用，一般这里实现的是初始化函数比如上面的写过滤器的初始化函数，这个函数就是用来将一个个过滤器串联起来的，形成一种链状结构。

好了说到这里我自己都很清楚了，相信多看几遍肯定能懂。



### 写在最后

花了一下时间去思考这段流程，以前总觉得C语言没有太好的封装性，或者不能实现一些很巧妙的设计，现在想想只是自己太菜而已哈哈哈，Nginx还有很多优秀的开源项目貌似都是用C语言搞定的，再也不会觉得C语言不好了，这些优秀的设计思想，封装也好，解耦也罢，通过对这些设计思想的不断总结学习，一定可以让自己在设计时越来越好。
