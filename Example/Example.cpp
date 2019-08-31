ThreadPool threadPool;
threadPool.Init();
threadPool.Obtain([](void* param) {
	cout << "Thread action." << endl;
	}, NULL);