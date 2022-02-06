# System wide locks

If multiple applications are using single resource, or application should be launched only once, it is a matter of using system wide locks.
Linux provides two main ways how these locks may be implemented:
- File locks (lock associated with a temporary file)
- Semaphores (counter API provided by OS)

## File lock
File locks provide only basic functions:
- try lock
- release

Full source and usage examples: **file_lock.cpp**.

Interface:
```
class GlobalLock
{
public:
	GlobalLock( const char *path );
	~GlobalLock();

	bool acquire();
	void release();
	bool locked() const;
};
```
Constructor expects a file name, which will be used as a lock name (example: "/tmp/test.lock").

Destructor is releasing lock it it was acquired.

Lock *acquire()* function may return true, if lock has been taken successfully, otherwise, it returns false.

Lock *release()* function unlocks the file, if it was previously acquired by the process.

To check if lock was taken, *locked()* function may be used.

## Semaphores
Semaphores are much more flaxible. They provide waiting locks.

Full source and usage examples: **file_lock.cpp**.

Interface:
```
class GlobalLock
{
public:
	GlobalLock( const char *name );
	~GlobalLock();

	operator bool() const;
	bool locked() const;
	bool lock();
	bool try_lock();
	bool try_lock( unsigned millisec );
	bool unlock();
};
```
Constructor expects an unique name (example: "/test").

Destructor is releasing lock it it was acquired.

To check if lock has been constructed successfully, class provides a bool operator.

To check if lock is acquired by any other concurrent process, *locked()* function may be used.

*lock()* is synchronous operation. It will wait for pending lock to be released. So, it should be used carefully to avoid deadlocks.

*try_lock()* is taking lock immediately, if it is available. Another version provides a time period to wait for semaphore availability.

*unlock()* is releasing lock if it was previously taken by a calling process.

### Scoped lock
A helper (wrapper) utility, which helps to acquire resource in a certain C++ scope.
