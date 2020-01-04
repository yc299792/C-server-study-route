#pragma once
/*====================================================================
 *
 * @file		MsgQueue.h
 * @author		yangchao
 * @date		2020-1-4
 *
 * Copyright (c) 2020 yangchao
 *
 * @brief		A Generic Message Queue
 *
 *
 *===================================================================*/

#include<list>
#include<pthread.h>
#include<ctime>
#include<iostream>

using namespace std;

/*----------------------------------------------------------------------
  |   NPT_GenericQueue
  +---------------------------------------------------------------------*/

template<class T>
class GenericQueue{
public:
	static GenericQueue<T>* createInstance(int max_items = 0);

	//method
	virtual ~GenericQueue(){}
	//抽象类只是接口
	virtual int enqueue(T* item, int timeout) = 0;
	virtual int dequeue(T*& item, int timeout = -1) = 0;
	virtual int peek(T*& item, int timeout = -1) = 0;
	virtual bool isFull() = 0;
	virtual bool isEmpty() = 0;
	virtual int queueStatus() = 0;

protected:
	GenericQueue(){}
};

/*----------------------------------------------------------------------
  |   NPT_MsgQueue
  +---------------------------------------------------------------------*/
template<class T>
class MsgQueue {

public:
	MsgQueue(int max_items=0):m_delegate(GenericQueue<T>::createInstance(max_items)){}
	virtual ~MsgQueue<T>() { delete m_delegate; }//析构的时候释放掉

	virtual int enqueue(T* item, int timeout = 0)
	{
		return m_delegate->enqueue(reinterpret_cast<T*>(item), timeout);
	}

	virtual int dequeue(T*& item, int timeout = 0)
	{
		return m_delegate->dequeue(reinterpret_cast<T*&>(item), timeout);
	}

	virtual int peek(T*& item, int timeout = 0)
	{
		return m_delegate->peek(reinterpret_cast<T*&>(item), timeout);
	}

	virtual int queueStatus()
	{
		return m_delegate->queueStatus();
	}

	virtual bool isFull()
	{
		return m_delegate->isFull();
	}

	virtual bool isEmpty()
	{
		return m_delegate->isEmpty();
	}


private:
	GenericQueue<T>* m_delegate;
};


/*----------------------------------------------------------------------
  |       PosixQueue::Peek
  +---------------------------------------------------------------------*/
template<class T>
class PosixQueue :class GenericQueue<T>
{
public:
	// methods
	PosixQueue(unsigned int max_items);
	~PosixQueue();
	int enqueue(T* item, int timeout);
	int dequeue(T*& item, int timeout);
	int peek(T*& item, int timeout);
	bool isFull();
	bool isEmpty();
	int queueStatus();

private:
	void abort();
	int getTimeout(int timeout, struct timespec& timed);

private:
	// members
	unsigned int            m_maxItems;
	pthread_mutex_t         m_mutex;//互斥锁
	pthread_cond_t          m_canPushCondition;//入队条件
	pthread_cond_t          m_canPopCondition;//出队条件
	int                     m_pushersWaitingCount;//等着入对的
	int                     m_poppersWaitingCount;//等着出队的
	std::list<T*>           m_items;//用一个链表维护成员
	bool                    m_aborting;
};

/*----------------------------------------------------------------------
  |       PosixQueue::PosixQueue
  +---------------------------------------------------------------------*/
template <class T>
PosixQueue<T>::PosixQueue(unsigned int max_items) :
	m_maxItems(max_items),
	m_pushersWaitingCount(0),
	m_poppersWaitingCount(0),
	m_aborting(false)
{
	pthread_mutex_init(&m_mutex, NULL);
	pthread_cond_init(&m_canPushCondition, NULL);
	pthread_cond_init(&m_canPopCondition, NULL);
}
/*----------------------------------------------------------------------
  |       PosixQueue::~PosixQueue()
  +---------------------------------------------------------------------*/
template <class T>
PosixQueue<T>::~PosixQueue()
{
	abort();
	// destroy resources
	pthread_cond_destroy(&m_canPushCondition);
	pthread_cond_destroy(&m_canPopCondition);
	pthread_mutex_destroy(&m_mutex);
}

template <class T>
bool PosixQueue<T>::isFull()
{
	bool full;
	pthread_mutex_lock(&m_mutex);//加锁
	full = (m_items.size() == m_maxItems);
	pthread_mutex_unlock(&m_mutex);//解锁
	return full;
}

template<class T>
bool PosixQueue<T>::isEmpty()
{
	bool empty;
	pthread_mutex_lock(&m_mutex);
	empty = (m_items.size() == 0);
	pthread_mutex_unlock(&m_mutex);
	return empty;
}

template<class T>
int PosixQueue<T>::queueStatus()
{
	int ret = m_items.size();
	if (ret >= m_maxItems - 1)
	{
		return -1;
	}
	return ret;
}

/*----------------------------------------------------------------------
  |       PosixQueue::abort
  +---------------------------------------------------------------------*/
template <class T>
void PosixQueue<T>::abort()
{
	pthread_cond_t abort_condition;
	pthread_cond_init(&abort_condition, NULL);//用一个abort条件变量
	//QueueItem* item = NULL;
	T* item = NULL;

	struct timespec timed;
	getTimeout(20, timed);

	// acquire mutex
	if (pthread_mutex_lock(&m_mutex))
	{
		return;
	}

	// tell other threads that they should exit immediately
	m_aborting = true;

	// notify clients，唤醒线程的，通知所有的订阅者
	pthread_cond_broadcast(&m_canPopCondition);
	pthread_cond_broadcast(&m_canPushCondition);

	// 等到所有的线程结束，设置了一个等待时间
	while (m_poppersWaitingCount > 0 || m_pushersWaitingCount > 0)
	{
		pthread_cond_timedwait(&abort_condition,
			&m_mutex,
			&timed);
	}

	while (!m_items.empty())
	{
		item = m_items.front();
		m_items.pop_front();//从头出队
		delete item;
	}
	//清空
	m_items.clear();

	pthread_mutex_unlock(&m_mutex);
}
/*----------------------------------------------------------------------
  |       PosixQueue::getTimeout
  +---------------------------------------------------------------------*/
template <class T>
int PosixQueue<T>::getTimeout(int timeout, struct timespec& timed)
{
	if (timeout != -1) {
		// get current time from system
		struct timeval now;
		if (gettimeofday(&now, NULL)) {
			return -1;
		}

		now.tv_usec += timeout * 1000;
		if (now.tv_usec >= 1000000) {
			now.tv_sec += now.tv_usec / 1000000;
			now.tv_usec = now.tv_usec % 1000000;
		}

		// setup timeout
		timed.tv_sec = now.tv_sec;
		timed.tv_nsec = now.tv_usec * 1000;
	}
	return 0;
}
/*----------------------------------------------------------------------
  |       PosixQueue::enqueue
  +---------------------------------------------------------------------*/
template <class T>
int PosixQueue<T>::enqueue(T* item, int timeout)
{
	struct timespec timed;
	if (timeout != -1)
	{
		getTimeout(timeout, timed);
	}
	// lock the mutex that protects the list
	if (pthread_mutex_lock(&m_mutex))
	{
		return -1;
	}

	int result = 0;
	// check that we have not exceeded the max
	if (m_maxItems)
	{
		while (m_items.size() >= m_maxItems)
		{
			// wait until we can push
			++m_pushersWaitingCount;
			if (timeout == -1)
			{
				pthread_cond_wait(&m_canPushCondition, &m_mutex);
				--m_pushersWaitingCount;
			}
			else
			{
				int wait_res = pthread_cond_timedwait(&m_canPushCondition, &m_mutex, &timed);
				--m_pushersWaitingCount;
				if (wait_res == ETIMEDOUT)
				{
					result = -1;
					break;
				}
			}
			if (m_aborting)
			{
				result = -1;
				break;
			}
		}
	}
	// add the item to the list
	if (result == 0)
	{
		m_items.push_back(item);
		// wake up any thread that may be waiting to pop
		if (m_poppersWaitingCount)
		{
			pthread_cond_broadcast(&m_canPopCondition);
		}
	}
	// unlock the mutex
	pthread_mutex_unlock(&m_mutex);

	return result;
}

/*----------------------------------------------------------------------
  |       PosixQueue::dequeue
  +---------------------------------------------------------------------*/
template <class T>
int PosixQueue<T>::dequeue(T*& item, int timeout)
{
	struct timespec timed;
	if (timeout != -1)
	{
		getTimeout(timeout, timed);
	}
	// lock the mutex that protects the list
	if (pthread_mutex_lock(&m_mutex))
	{
		return -1;
	}

	int result = 0;

	if (timeout)
	{
		while (m_items.empty())
		{
			// no item in the list, wait for one
			++m_poppersWaitingCount;
			if (timeout == -1)
			{
				pthread_cond_wait(&m_canPopCondition, &m_mutex);
				--m_poppersWaitingCount;
			}
			else
			{
				int wait_res = pthread_cond_timedwait(&m_canPopCondition, &m_mutex, &timed);
				--m_poppersWaitingCount;
				if (wait_res == ETIMEDOUT)
				{
					result = -1;
					break;
				}
			}
			if (m_aborting)
			{
				result = -1;
				break;
			}
		}
	}

	if (m_items.empty() && (result == 0))
	{
		result = -1;
	}

	if (result == 0)
	{
		item = m_items.front();
		m_items.pop_front();
	}

	// wake up any thread that my be waiting to push
	if (m_maxItems && (result == 0) && m_pushersWaitingCount)
	{
		pthread_cond_broadcast(&m_canPushCondition);
	}

	// unlock the mutex
	pthread_mutex_unlock(&m_mutex);

	return result;
}
/*----------------------------------------------------------------------
  |       PosixQueue::Peek
  +---------------------------------------------------------------------*/
template <class T>
int PosixQueue<T>::peek(T*& item, int timeout)
{
	struct timespec timed;
	if (timeout != -1)
	{
		getTimeout(timeout, timed);
	}

	// lock the mutex that protects the list
	if (pthread_mutex_lock(&m_mutex))
	{
		return -1;
	}

	int result = 0;

	typename std::list<T*>::iterator head = m_items.begin();

	if (timeout)
	{
		while (head == m_items.end())
		{
			// no item in the list, wait for one
			++m_poppersWaitingCount;
			if (timeout == -1)
			{
				pthread_cond_wait(&m_canPopCondition, &m_mutex);
				--m_poppersWaitingCount;
			}
			else
			{
				int wait_res = pthread_cond_timedwait(&m_canPopCondition, &m_mutex, &timed);
				--m_poppersWaitingCount;
				if (wait_res == ETIMEDOUT)
				{
					result = -1;
					break;
				}
			}

			if (m_aborting)
			{
				result = -1;
				break;
			}

			head = m_items.begin();
		}
	}
	else
	{
		if (head == m_items.end()) result = -1;
	}

	item = (head != m_items.end()) ? *head : NULL;

	// unlock the mutex
	pthread_mutex_unlock(&m_mutex);

	return result;
}

/*----------------------------------------------------------------------
  |       GenericQueue::CreateInstance
  +---------------------------------------------------------------------*/
template<class T>
GenericQueue<T>* GenericQueue<T>::createInstance(int max_items)
{
	return new PosixQueue<T>(max_items);
}
