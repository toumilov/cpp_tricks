#include <time.h>
#include <memory>
#include <chrono>
#include <functional>
#include <initializer_list>


struct Epoll
{
	enum class Event : uint32_t
	{
		In = 1,
		Out = 1<<1,
		Hangup = 1<<2,
		OneShot = 1<<3,
		EdgeTrigger = 1<<4,
	};
	struct Events
	{
		Events(std::initializer_list<Event> l);
		uint32_t e;
	};
	typedef std::function<void(int, Event)> Callback;

	Epoll();
	Epoll(Callback cb);
	Epoll(const Epoll&) = delete;
	Epoll(Epoll&&);
	Epoll& operator=(const Epoll&) = delete;
	Epoll& operator=(Epoll&&);
	~Epoll();
	operator bool() const;

	/**
	 * @brief Add file descriptor to the watch list
	 * @param[in] fd  - file descriptor
	 * @param[in] evt - event type
	 * @param[in] cb  - callback function (if nullptr, default callback is called)
	 * @return true if successfully added
	 */
	bool Add(int fd, const Events& evt, Callback cb = nullptr);

	/**
	 * @brief Remove descriptor from the watch list
	 * @param[in] fd - file descriptor
	 * @return true if successfully removed
	 */
	bool Remove(int fd);

	/**
	 * @brief Modify of re-arm one-shot event
	 * @param[in] fd  - file descriptor
	 * @param[in] evt - event type
	 * @return true if successful
	 */
	bool Modify(int fd, const Events& evt);

	/**
	 * @brief Wait for pending events
	 * @param[in] duration - maximum wait time
	 * @return true if events were dispatched or timeout occured
	 * @return false on error
	 */
	template<class Rep, class Period>
	bool Wait(const std::chrono::duration<Rep, Period>& duration) const
	{
		auto timeout = ToTimespec(duration);
		return Wait(&timeout);
	}
	bool Wait() const;

	/**
	 * @brief Leave waiting state
	 */
	void StopWait() const;

private:
	struct Impl;
	std::unique_ptr<Impl> m_impl;

	template<class Rep, class Period>
	static timespec ToTimespec(std::chrono::duration<Rep, Period> duration)
	{
		auto sec = std::chrono::duration_cast<std::chrono::seconds>(duration);
		duration -= sec;
		return timespec{sec.count(), std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count()};
	}

	bool Wait(const timespec* duration) const;
};