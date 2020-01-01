### NginxTimer的实现

在说主题之前先来说一下Linux中Timer的实现：在Linux中，设置定时器，是通过每次系统定时器时钟的中断处理程序来设置相应的软中断位，然后通过这个中断处理程序扫描系统中所有挂起的定时器，如果发现哪个定时器超时了就调用相应的处理函数，也就说Linux定时器是通过系统中断实现的。

在Nginx中，Timer是自己实现的，而且实现的方法完全不同，它是通过一个红黑树去维护所有的time节点。然后在工作进程中每一循环都会调用**ngx_process_events_and_timers**函数，这个函数中又会调用处理定时器的函数**ngx_event_expire_timers**，这个函数每一次都会从红黑树中取最小的时间值，判断是否超时，超时就执行他们的函数，直到取出的不超时为止。类似的做法还有libevent中的小根堆维护time的思想，这两种思想是类似的，而且也能把定时器的超时时间和一些I/O事件统一到了工作进程之中。

来看看定时器的初始化Ngx_event.c中：

```C
 /*初始化计时器，此处将会创建起一颗红黑色，来维护计时器。*/    
    if (ngx_event_timer_init(cycle->log) == NGX_ERROR) {
        return NGX_ERROR;
    }
//timer的初始化
ngx_int_t
ngx_event_timer_init(ngx_log_t *log)
{
//初始化红黑树
    ngx_rbtree_init(&ngx_event_timer_rbtree, &ngx_event_timer_sentinel,
                    ngx_rbtree_insert_timer_value);

#if (NGX_THREADS)

    if (ngx_event_timer_mutex) {
        ngx_event_timer_mutex->log = log;
        return NGX_OK;
    }

    ngx_event_timer_mutex = ngx_mutex_init(log, 0);
    if (ngx_event_timer_mutex == NULL) {
        return NGX_ERROR;
    }

#endif
    return NGX_OK;
}
```

这个函数就是调用 ngx_rbtree_init来初始化一颗nginx自己实现的红黑树。

接下来就是如何去定义一个timer事件了：

就是利用下面这个函数，将一个定时事件的超时时间值加入到红黑树中即可，与之对应的还有一个从红黑树中删除的操作函数：**ngx_event_del_timer**

```C
//将一个定时时间加入到红黑树中，交给红黑树维护
static ngx_inline void
ngx_event_add_timer(ngx_event_t *ev, ngx_msec_t timer)   //timer说白了就是一个int类型的值，表示超时的时间值，红黑树节点的key就是这个timer
{
    ngx_msec_t      key;
    ngx_msec_int_t  diff;

    key = ngx_current_msec + timer;  //表示该event的超时时间，为当前时间的值加上超时变量，这个时候可以就代表超时的时间点了

    if (ev->timer_set) {

        /*
         * Use a previous timer value if difference between it and a new
         * value is less than NGX_TIMER_LAZY_DELAY milliseconds: this allows
         * to minimize the rbtree operations for fast connections.
         */

        diff = (ngx_msec_int_t) (key - ev->timer.key);

        if (ngx_abs(diff) < NGX_TIMER_LAZY_DELAY) {
            ngx_log_debug3(NGX_LOG_DEBUG_EVENT, ev->log, 0,
                           "event timer: %d, old: %M, new: %M",
                            ngx_event_ident(ev->data), ev->timer.key, key);
            return;
        }

        ngx_del_timer(ev);
    }

    ev->timer.key = key;

    ngx_log_debug3(NGX_LOG_DEBUG_EVENT, ev->log, 0,
                   "event timer add: %d: %M:%M",
                    ngx_event_ident(ev->data), timer, ev->timer.key);
//需要进行加锁操作保证线程安全
    ngx_mutex_lock(ngx_event_timer_mutex);

    ngx_rbtree_insert(&ngx_event_timer_rbtree, &ev->timer);    //将定时事件的timer域插入到红黑树当中

    ngx_mutex_unlock(ngx_event_timer_mutex);

    ev->timer_set = 1;
}
```

**那么Nginx是如何处理定时事件的**：

在ngx_process_events_and_timers中对定时事件处理,先去把当前红黑树中最小的事件找到，然后传递给epoll的wait，这一点保证epoll可以处理超时事件，因为如果epoll收到超时事件却没有收到这个那么超时事件就不会得到处理了：

```C
if (ngx_timer_resolution) {
        timer = NGX_TIMER_INFINITE;
        flags = 0;

    } else {
        timer = ngx_event_find_timer();  //找到当前红黑树当中的最小的事件，传递给epoll的wait，保证epoll可以该时间内可以超时，可以使得超时的事件得到处理
        flags = NGX_UPDATE_TIME;

#if (NGX_THREADS)

        if (timer == NGX_TIMER_INFINITE || timer > 500) {
            timer = 500;
        }

#endif
    }
```

寻找最小的节点ngx_event_find_timer：

```C
//用于返回当前红黑树当中的超时时间，说白了就是返回红黑树中最左边的元素的超时时间
ngx_msec_t
ngx_event_find_timer(void)
{
    ngx_msec_int_t      timer;
    ngx_rbtree_node_t  *node, *root, *sentinel;

    if (ngx_event_timer_rbtree.root == &ngx_event_timer_sentinel) {
        return NGX_TIMER_INFINITE;
    }

    ngx_mutex_lock(ngx_event_timer_mutex);

    root = ngx_event_timer_rbtree.root;
    sentinel = ngx_event_timer_rbtree.sentinel;

    node = ngx_rbtree_min(root, sentinel);  //找到红黑树中key最小的节点

    ngx_mutex_unlock(ngx_event_timer_mutex);

    timer = (ngx_msec_int_t) (node->key - ngx_current_msec);//返回的是还剩下的超时时间

    return (ngx_msec_t) (timer > 0 ? timer : 0);
}
```

在ngx_process_events_and_timers函数中获取这个值，因为这个值是要用来设置epoll的wait时间的，保证epoll在处理的时候，最小的超时事件既然能及时得到处理，其他就都能都得到处理了，否则，已经超时了，epoll还未处理，那也就不能达到定时的目的了.

接下来：

就是说epoll wait事件是耗时的，在耗时的时间中，已经超时的就应该被删除，同时调用相应的处理函数处理.

```C
/*delta是上文对epoll wait事件的耗时统计，存在毫秒级的耗时 
        就对所有事件的timer进行检查，如果time out就从timer rbtree中， 
        删除到期的timer，同时调用相应事件的handler函数完成处理。 
      */  
    if (delta) {
        ngx_event_expire_timers();
    }
```

在ngx_process_events_and_timers中调用gx_event_expire_timers处理所有的定时事件，那么为什么要去判断这个delta呢？因为这个值统计的是处理其余事件的用时，如果用时超过了毫秒，那么就应调用ngx_event_expire_timers这个函数处理所有的定时，否则就不会调用，因为刚处理完，完全没必要再去处理一次，性能就是在这些细节中提升上来的。

ngx_event_expire_timers函数：

```C
void ngx_event_expire_timers(void)
{
    ngx_event_t        *ev;
    ngx_rbtree_node_t  *node, *root, *sentinel;

    sentinel = ngx_event_timer_rbtree.sentinel;

//循环处理所有的超时事件
    for ( ;; ) {

        ngx_mutex_lock(ngx_event_timer_mutex);

        root = ngx_event_timer_rbtree.root;

        if (root == sentinel) {
            return;
        }

        node = ngx_rbtree_min(root, sentinel);   //获取key最小的节点

        /* node->key <= ngx_current_time */

        if ((ngx_msec_int_t) (node->key - ngx_current_msec) <= 0) {   //判断该节点是否超时，以为是最小的如果没超时就跳出
            //通过偏移来获取当前timer所在的event
            ev = (ngx_event_t *) ((char *) node - offsetof(ngx_event_t, timer));

            ngx_log_debug2(NGX_LOG_DEBUG_EVENT, ev->log, 0,
                           "event timer del: %d: %M",
                           ngx_event_ident(ev->data), ev->timer.key);
//将当前timer移除
            ngx_rbtree_delete(&ngx_event_timer_rbtree, &ev->timer);

            ngx_mutex_unlock(ngx_event_timer_mutex);

            ev->timer_set = 0;

            ev->timedout = 1;

            ev->handler(ev);   //调用event的handler来处理这个事件

            continue;
        }

        break;
    }

    ngx_mutex_unlock(ngx_event_timer_mutex);
}
```

**总结：**

在Nginx中Timer的实现是通过这种从红黑树中选取最小定时事件的方式，利用了红黑树查找的高效，再结合自己对性能的优化，将定时器融入到Worker进程之中，在Nginx的运行环境之中，这种方式可能比Linux中断那一套更为高效，真的是太有智慧了！

