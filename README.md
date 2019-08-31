# ThreadPool
It's designed for easy to use, just call Obtain method to get a thread to run. Below is example.

ThreadPool threadPool;
threadPool.Init();
    threadPool.Obtain([](void* param) {
    cout << "Thread action." << endl;
}, NULL);