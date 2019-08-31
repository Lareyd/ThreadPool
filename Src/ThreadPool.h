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
		uint32_t extensionSize;// Number of threads to increase when thread pool is extended. Default is 5. 

	public:
		long double cleanInterval;// Interval to clean excess threads. Default is 10 minutes, in seconds.
		long double cleanDuration;// Thread has been idle more this would be cleaned from the thread pool. Default is 10 minutes, in seconds.

	private:
		bool valid;// Whether valid.

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
		// Initialization.
		// size：Number of threads to create when initialize.
		// extensionSize：Number of threads to increase when thread pool is extended.
		// cleanInterval：Interval to clean excess threads. Default is 10 minutes, in seconds.
		// cleanDuration：Thread has been idle more this would be cleaned from the thread pool. Default is 10 minutes, in seconds.
		void Init(uint32_t size = 5, uint32_t extensionSize = 5, long double cleanInterval = 600., long double cleanDuration = 600.);

		// Whether valid.
		bool IsValid();

		// Obtain a thread to execute the action.
		// action：The action that thread executes.
		// param：Action parameter.
		// On success, returns thread pointer. If not initialized, or invalid input, or on failure, returns NULL.
		Thread* Obtain(Action action, void* param);

	private:
		void Clear();

	private:
		void CreateThreads(uint32_t count);

	private:
		void TaskAction(void* param);

		void CleanAction();
	};
}


#endif