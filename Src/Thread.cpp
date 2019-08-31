/****************************************************************************************************************************
Name: Thread.cpp.
Author: 叶伟(Lareyd).
Copyright: Copyright (c) 2018 - 2018, 叶伟(Lareyd). All rights reserved.
Version: 1.0.
Description: Implementation of Thread settings.
****************************************************************************************************************************/


#include "Thread.h"


namespace Threads
{
	Thread::Thread(std::thread* thread)
	{
		this->thread = thread;
	}

	thread::id Thread::GetId()
	{
		if (NULL == thread)
		{
			return noEffectThread.get_id();
		}

		return thread->get_id();
	}

	thread::native_handle_type Thread::GetHandle()
	{
		if (NULL == thread)
		{
			return NULL;
		}

		return thread->native_handle();
	}
}