/****************************************************************************************************************************
Name: ThreadPool.cpp.
Author: 叶伟(Lareyd).
Copyright: Copyright (c) 2018 - 2018, 叶伟(Lareyd). All rights reserved.
Version: 1.0.
Description: Implementation of ThreadPool settings.
****************************************************************************************************************************/


#include "Thread.h"
#include "ThreadPool.h"


namespace Threads
{
	ThreadPool::~ThreadPool()
	{
		Clear();
	}

	void ThreadPool::Init(uint32_t size, uint32_t extensionSize, long double cleanInterval, long double cleanDuration)
	{
		if (cleanInterval < 0)
		{
			cleanInterval = 60. * 10;
		}
		this->cleanInterval = cleanInterval;
		if (cleanDuration < 0)
		{
			cleanDuration = 60. * 10;
		}
		this->cleanDuration = cleanInterval;
		cleanThreadLock.lock();
		cleanThread = new thread(&ThreadPool::CleanAction, this);

		if (0 != extensionSize)
		{
			this->extensionSize = extensionSize;
		}
		else
		{
			this->extensionSize = 5;
		}

		if (0 == size)
		{
			size = 5;
		}
		CreateThreads(size);

		valid = true;
	}

	bool ThreadPool::IsValid()
	{
		return valid;
	}

	Thread* ThreadPool::Obtain(Action action, void* param)
	{
		if (!valid
			|| NULL == action)
		{
			return NULL;
		}

		commonDataLock.lock();

		if (data.empty())// If no thread is available, create number of extensionSize threads.
		{
			if (0 == extensionSize)
			{
				extensionSize = 5;
			}
			CreateThreads(extensionSize);
		}

		const pair<Thread*, PTHREAD_DATA>* element = NULL;
		for (const pair<Thread*, PTHREAD_DATA>& keyValue : data)
		{
			element = &keyValue;
			break;
		}

		element->second->action = action;
		element->second->actionParams = param;
		element->second->idle = false;
		element->second->condition->notify_one();

		data.erase(element->first);
		usedData[element->first] = element->second;

		this_thread::sleep_for(chrono::duration<int, milli>(10));// Ensure fast switch between lock and unlock won't cause trouble, i.e. dead lock.
		commonDataLock.unlock();

		return element->first;
	}

	void ThreadPool::Clear()
	{
		valid = false;

		cleanThreadLock.unlock();
		if (NULL != cleanThread)
		{
			delete cleanThread;
			cleanThread = NULL;
		}

		for (const pair<Thread*, PTHREAD_DATA>& keyValue : data)
		{
			keyValue.second->terminate = true;
			keyValue.second->condition->notify_one();

			delete keyValue.second->lock;
			delete keyValue.second->condition;
			delete keyValue.second->thread;
			this_thread::sleep_for(chrono::duration<int, milli>(50));// Wait for thread action to terminate.
			delete keyValue.second;
		}
		data.clear();

		for (const pair<Thread*, PTHREAD_DATA>& keyValue : usedData)
		{
			keyValue.second->terminate = true;
			keyValue.second->condition->notify_one();

			delete keyValue.second->lock;
			delete keyValue.second->condition;
			delete keyValue.second->thread;
			delete keyValue.second;
		}
		usedData.clear();
	}

	void ThreadPool::CreateThreads(uint32_t count)
	{
		for (uint32_t i = 0; i < count; i++)
		{
			PTHREAD_DATA threadParams = new THREAD_DATA{ NULL, false, true, chrono::high_resolution_clock::now(), NULL, NULL, new mutex(), new condition_variable() };

			thread* stlThread = new thread(&ThreadPool::TaskAction, this, threadParams);
			stlThread->detach();
			Thread* thread = new Thread(stlThread);

			threadParams->thread = thread;
			data.insert(pair<Thread*, PTHREAD_DATA>(thread, threadParams));
		}
	}

	void ThreadPool::TaskAction(void* param)
	{
		PTHREAD_DATA threadParams = (PTHREAD_DATA)param;
		for (; ; )
		{
			if (threadParams->terminate)
			{
				return;
			}

			{
				unique_lock<mutex> lock(*threadParams->lock);
				threadParams->condition->wait(lock);
			}

			if (NULL != threadParams->action)
			{
				threadParams->action(threadParams->actionParams);
				threadParams->action = NULL;

				if (!threadParams->terminate
					&& valid)
				{
					threadParams->idle = true;
					threadParams->startIdleTime = chrono::high_resolution_clock::now();

					commonDataLock.lock();

					PTHREAD_DATA params = usedData[threadParams->thread];
					usedData.erase(threadParams->thread);
					data[threadParams->thread] = params;

					this_thread::sleep_for(chrono::duration<int, milli>(10));// Ensure fast switch between lock and unlock won't cause trouble, i.e. dead lock.
					commonDataLock.unlock();
				}
			}
		}
	}

	void ThreadPool::CleanAction()
	{
		for (; ; )
		{
			chrono::duration<long double> interval(cleanInterval);
			cleanThreadLock.try_lock_for(interval);

			if (!valid)
			{
				return;
			}

			commonDataLock.lock();

			chrono::time_point<chrono::high_resolution_clock> now =  chrono::high_resolution_clock::now();
			unordered_map<Thread*, PTHREAD_DATA>::iterator end = data.end();
			for (unordered_map<Thread*, PTHREAD_DATA>::iterator iterator = data.begin(); iterator != end;)
			{
				if (!iterator->second->idle// not idle
					|| chrono::duration<long double>(now - iterator->second->startIdleTime).count() < cleanDuration)// Thread idle duration < cleanDuration
				{
					++iterator;
					continue;
				}

				iterator->second->terminate = true;
				iterator->second->condition->notify_one();
				this_thread::sleep_for(chrono::duration<int, milli>(50));// Wait for thread action to terminate.

				delete iterator->second->lock;
				delete iterator->second->condition;
				delete iterator->second->thread;
				delete iterator->second;
				data.erase(iterator->first);
				iterator = data.begin();
				end = data.end();
			}

			this_thread::sleep_for(chrono::duration<int, milli>(10));// Ensure fast switch between lock and unlock won't cause trouble, i.e. dead lock.
			commonDataLock.unlock();
		}
	}
}