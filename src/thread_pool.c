#include "../inc/thread_pool.h"
// 开启线程的个数
const int NUMBER = 2;
// 任务结构体
typedef struct Task
{
	void (*function)(void *arg);
	void *arg;
} Task;

// 线程池结构体
struct ThreadPool
{
	/* 任务队列 */
	Task *taskQueue;
	int queueCapacity; // 容量
	int queueSize;	   // 当前任务的个数
	int queueFront;	   // 对头->取数据
	int queueRear;	   // 队尾->放数据

	pthread_t managerID;  // 管理员线程ID
	pthread_t *threadIDs; // 工作线程ID

	int minNum;	 // 最小线程数量
	int maxNum;	 // 最大线程数量
	int busyNum; // 忙的线程的个数
	int liveNum; // 存活线程的个数
	int exitNum; // 要销毁的线程的个数

	pthread_mutex_t mutexPool; // 锁整个的线程池
	pthread_mutex_t mutexBusy; // 锁busyNum变量

	pthread_cond_t notFull;	 // 任务队列是不是满
	pthread_cond_t notEmpty; // 任务队列是不是空

	int shutdown; // 销毁线程池，销毁是1，不销毁时0
};

ThreadPool *threadPoolCreate(int threadMinNum, int threadmaxNum, int queueSize)
{
	ThreadPool *pool = (ThreadPool *)malloc(sizeof(ThreadPool));
	do
	{
		if (pool == NULL)
		{
			printf("内存创建线程池失败\n");
			break;
		}

		pool->threadIDs = (pthread_t *)malloc(sizeof(pthread_t) * threadmaxNum); // 类型乘最大个数
		if (pool->threadIDs == NULL)
		{
			printf("内存创建线程失败\n");
			break;
		}

		memset(pool->threadIDs, 0, sizeof(pthread_t) * threadmaxNum);
		pool->minNum = threadMinNum;
		pool->maxNum = threadmaxNum;
		pool->busyNum = 0;
		pool->liveNum = threadMinNum;
		pool->exitNum = 0;

		if (pthread_mutex_init(&pool->mutexPool, NULL) != 0 ||
			pthread_mutex_init(&pool->mutexBusy, NULL) != 0 ||
			pthread_cond_init(&pool->notEmpty, NULL) != 0 ||
			pthread_cond_init(&pool->notFull, NULL) != 0)
		{
			printf("mutex or condition init error\n");
			break;
		}
		// 任务队列
		pool->taskQueue = (Task *)malloc(sizeof(Task) * queueSize);
		pool->queueCapacity = queueSize;
		pool->queueSize = 0;
		pool->queueFront = 0;
		pool->queueRear = 0;

		pool->shutdown = 0;
		// 创建线程
		pthread_create(&pool->managerID, NULL, manager, pool);
		for (int i = 0; i < threadMinNum; ++i)
		{
			pthread_create(&pool->threadIDs[i], NULL, worker, pool);
		}
		return pool;
	} while (0);

	// 释放资源
	if (pool && pool->threadIDs)
		free(pool->threadIDs);
	if (pool->taskQueue)
		free(pool->taskQueue);
	if (pool)
		free(pool);
	return NULL;
}

void threadPoolAdd(ThreadPool *pool, void (*func)(void *), void *arg)
{
	pthread_mutex_lock(&pool->mutexPool);
	while (pool->queueSize == pool->queueCapacity && !pool->shutdown)
	{
		// 阻塞生产者线程
		pthread_cond_wait(&pool->notFull, &pool->mutexPool);
	}
	if (pool->shutdown)
	{
		pthread_mutex_unlock(&pool->mutexPool);
		return;
	}
	// 添加任务
	pool->taskQueue[pool->queueRear].function = func;
	pool->taskQueue[pool->queueRear].arg = arg;
	pool->queueRear = (pool->queueRear + 1) % pool->queueCapacity;
	pool->queueSize++;

	pthread_cond_signal(&pool->notEmpty);
	pthread_mutex_unlock(&pool->mutexPool);
}

int threadPoolBusyNum(ThreadPool *pool)
{
	pthread_mutex_lock(&pool->mutexBusy);
	int busyNum = pool->busyNum;
	pthread_mutex_unlock(&pool->mutexBusy);
	return busyNum;
}

int threadPoolAliveNum(ThreadPool *pool)
{
	pthread_mutex_lock(&pool->mutexPool);
	int aliveNum = pool->liveNum;
	pthread_mutex_unlock(&pool->mutexPool);
	return aliveNum;
}

void *worker(void *arg)
{
	ThreadPool *pool = (ThreadPool *)arg;
	while (1)
	{
		// 加锁
		pthread_mutex_lock(&pool->mutexPool);
		// 当前任务队列是否为空
		while (pool->queueSize == 0 && !pool->shutdown)
		{
			// 阻塞工作线程
			pthread_cond_wait(&pool->notEmpty, &pool->mutexPool);
			// 判断是不是要销毁线程
			if (pool->exitNum > 0)
			{
				pool->exitNum--;
				if (pool->liveNum > pool->minNum)
				{
					pool->liveNum--;
					pthread_mutex_unlock(&pool->mutexPool);
					threadExit(pool);
				}
			}
		}
		// 判断线程池是否被关闭了
		if (pool->shutdown)
		{
			pthread_mutex_unlock(&pool->mutexPool);
			threadExit(pool);
		}
		// 从任务队列中取出一个任务
		Task task;
		task.function = pool->taskQueue[pool->queueFront].function;
		task.arg = pool->taskQueue[pool->queueFront].arg;
		// 移动头节点
		pool->queueFront = (pool->queueFront + 1) % pool->queueCapacity;
		pool->queueSize--;
		// 唤醒
		pthread_cond_signal(&pool->notFull);
		// 解锁
		pthread_mutex_unlock(&pool->mutexPool);

		printf("%lu线程开始工作\n", pthread_self());
		// 加锁
		pthread_mutex_lock(&pool->mutexBusy);
		pool->busyNum++;
		pthread_mutex_unlock(&pool->mutexBusy);
		// 工作
		task.function(task.arg);
		// free(task.arg);
		task.arg = NULL;
		printf("%lu线程结束工作\n", pthread_self());
		// 加锁
		pthread_mutex_lock(&pool->mutexBusy);
		pool->busyNum--;
		pthread_mutex_unlock(&pool->mutexBusy);
	}

	return NULL;
}

void *manager(void *arg)
{
	ThreadPool *pool = (ThreadPool *)arg;
	while (!pool->shutdown)
	{
		// 每隔三秒检测一次
		sleep(1);

		// 取出线程池中的任务的数量和当前线程的数量
		pthread_mutex_lock(&pool->mutexPool);
		int queueSize = pool->queueSize;
		int liveNum = pool->liveNum;
		pthread_mutex_unlock(&pool->mutexPool);

		// 取出忙的线程的数量
		pthread_mutex_lock(&pool->mutexBusy);
		int busyNum = pool->busyNum;
		pthread_mutex_unlock(&pool->mutexBusy);

		// 添加线程
		// 任务个数>存活的线程个数&&存活的线程数<最大线程数
		if (queueSize > liveNum && liveNum < pool->maxNum)
		{
			// 加锁
			pthread_mutex_lock(&pool->mutexPool);
			int counter = 0;
			for (int i = 0; i < pool->maxNum && counter < NUMBER && pool->liveNum < pool->maxNum; ++i)
			{
				if (pool->threadIDs[i] == 0)
				{
					pthread_create(&pool->threadIDs[i], NULL, worker, pool);
					counter++;
					pool->liveNum++;
				}
			}
			// printf("counter=%d\n", counter);
			pthread_mutex_unlock(&pool->mutexPool);
		}
		// 销毁线程
		// 忙的线程*2<存活的线程数&&存活的线程数>最小线程数
		if (busyNum * 2 < liveNum && liveNum > pool->minNum)
		{
			pthread_mutex_unlock(&pool->mutexPool);
			pool->exitNum = NUMBER;
			pthread_mutex_unlock(&pool->mutexPool);
			// 让工作的线程自杀
			for (int i = 0; i < NUMBER; ++i)
			{
				pthread_cond_signal(&pool->notEmpty);
			}
		}
	}

	return NULL;
}

void threadExit(ThreadPool *pool)
{
	pthread_t tid = pthread_self();
	for (int i = 0; i < pool->maxNum; ++i)
	{
		if (pool->threadIDs[i] == tid)
		{
			pool->threadIDs[i] = 0;
			// printf("threadExit() called,%lu exiting\n", tid);
			break;
		}
	}
	pthread_exit(NULL);
}

int threadPoolDestroy(ThreadPool *pool)
{

	if (pool == NULL)
	{
		return -1;
	}
	// 关闭线程池
	pool->shutdown = 1;
	// 阻塞回收管理者线程
	pthread_join(pool->managerID, NULL);

	// 唤醒阻塞的消费者线程
	for (int i = 0; i < pool->liveNum; ++i)
	{
		pthread_cond_signal(&pool->notEmpty);
	}
	// 释放堆内存
	if (pool->taskQueue)
	{
		free(pool->taskQueue);
	}
	if (pool->threadIDs)
	{
		free(pool->threadIDs);
	}

	pthread_mutex_destroy(&pool->mutexPool);
	pthread_mutex_destroy(&pool->mutexBusy);
	pthread_cond_destroy(&pool->notEmpty);
	pthread_cond_destroy(&pool->notFull);

	// free(pool);
	pool = NULL;
	return 0;
}
