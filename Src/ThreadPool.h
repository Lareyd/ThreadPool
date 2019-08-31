/****************************************************************************************************************************
Name: ThreadPool.h.
Author: 叶伟(Lareyd).
Copyright: Copyright (c) 2018 - 2018, 叶伟(Lareyd). All rights reserved.
Version: 1.0.
Description: Declaration of ThreadPool settings.
****************************************************************************************************************************/


#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__


#include <stdint.h>

#include <unordered_map>
#include <thread>
#include <mutex>
using namespace std;

class Thread;


namespace Threads
{
	typedef void (*Action)(void* param);


	class ThreadPool
	{
	private:
		typedef struct ThreadData
		{
			Thread* thread;// Thread that executes the action.
			bool terminate;// Whether to terminate the thread.
			bool idle;
			chrono::time_point<chrono::high_resolution_clock> startIdleTime;// Time when turn to be idle.
			Action action;// Thread procedure. Used with Obtain(Action action, void* params = NULL).
			void* actionParams;// Thread procedure parameters.
			mutex* lock;
			condition_variable* condition;
		} THREAD_DATA, *PTHREAD_DATA;


	public:
		uint32_t extensionSize;// 每次扩展线程池时增加的线程数。默认为5。Number of threads to increase when thread pool is extended. Default is 5. 

	public:
		long double cleanInterval;// 每清除一次多余线程的时间间隔。默认10分钟，以秒为单位。Interval to clean excess threads. Default is 10 minutes, in seconds.
		long double cleanDuration;// 线程静默时长超过这个时长将被清除。默认10分钟，以秒为单位。Thread has been idle more this would be cleaned from the thread pool. Default is 10 minutes, in seconds.

	private:
		bool valid;// 线程池是否可用。Whether valid.

	private:
		unordered_map<Thread*, PTHREAD_DATA> data;
		unordered_map<Thread*, PTHREAD_DATA> usedData;
		mutex commonDataLock;

	private:
		thread* cleanThread;// Execute cleaning excess threads.
		timed_mutex cleanThreadLock;

	public:
		~ThreadPool();

	public:
		// 初始化。Initialization.
		// size：初始化线程池含有的线程数。Number of threads to create when initialize.
		// extensionSize：每次扩展线程池时增加的线程数。Number of threads to increase when thread pool is extended.
		// cleanInterval：每清除一次多余线程的时间间隔。默认10分钟，以秒为单位。Interval to clean excess threads. Default is 10 minutes, in seconds.
		// cleanDuration：线程静默时长超过这个时长将被清除。默认10分钟，以秒为单位。Thread has been idle more this would be cleaned from the thread pool. Default is 10 minutes, in seconds.
		void Init(uint32_t size = 5, uint32_t extensionSize = 5, long double cleanInterval = 600., long double cleanDuration = 600.);

		// 是否可用。Whether valid.
		bool IsValid();

		// 获取一个线程以执行任务。Obtain a thread to execute the action.
		// action：线程执行的任务。The action that thread executes.
		// param：任务的参数。// Action parameter.
		// 若成功，返回执行任务的线程指针。若未初始化，或输入参数不正确，或失败，返回NULL。On success, returns thread pointer. If not initialized, or invalid input, or on failure, returns NULL.
		Thread* Obtain(Action action, void* param);

	private:
		// 调用之后线程池将不再可用。Thread pool would not be valid after calling this method.
		void Clear();

	private:
		void CreateThreads(uint32_t count);

	private:
		void TaskAction(void* param);

		void CleanAction();
	};
}


#endif