/****************************************************************************************************************************
Name: Thread.h.
Author: 叶伟(Lareyd).
Copyright: Copyright (c) 2018 - 2018, 叶伟(Lareyd). All rights reserved.
Version: 1.0.
Description: Declaration of Thread settings.
****************************************************************************************************************************/


#ifndef __THREAD_H__
#define __THREAD_H__


#include <thread>
using namespace std;


namespace Threads
{
	class Thread
	{
	private:
		std::thread* thread;
		std::thread noEffectThread;

	public:
		Thread(std::thread* thread);

	public:
		// 获取线程id。Get thrread id.
		// 若初始化传入NULL或不正确，返回无效线程的id。If NULL or invalid input at initialization, will return thread id of no effect.
		thread::id GetId();

		// 获取线程句柄。Get thread handle.
		// 若初始化传入NULL或不正确，返回NULL。If NULL or invalid input at initialization, will return NULL.
		thread::native_handle_type GetHandle();
	};
}


#endif