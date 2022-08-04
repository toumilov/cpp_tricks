# Thread pool

Typically, in client-server applications daemons are processing client requests in a spawned thread. This is supposed to release request listener loop for accepting incoming requests. Often a new thread is created for each request processing. However, thread number must be somehow limited. Otherwise, system resources could be improperly managed. To limimt the number of threads, thread pools are used.
Thread pool is initialized to handle maximum of N threads. To put a work item into a thread pool, user needs to `get` a worker object.
Once all jobs need to be stopped, user must call `join` to finish all pending work items.

# Example
1) Create a thread pool specifying it's size.
```
ThreadPool pool( 3 );
```
2) Post work items (functions and their parameters) into a thread pool.
```
void test( int i )
{
	printf("<%d>\n", i);
	sleep(1);
}
...
for( int i = 0; i < 10; i++ )
{
    pool.get().run( &test, int( i ) );
}
```
3) Cleanup before exit by calling `join`. It will not return back until pool is empty. New items should not be added once `join` is called.
```
pool.join();
```
