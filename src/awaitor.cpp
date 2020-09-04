// https://github.com/franktea/temp/blob/master/uncategorized/co_vs_callback.cpp
// https://github.com/franktea/temp/blob/master/uncategorized/coroutine.md
#include <iostream>
#include <thread>
#include <experimental/coroutine>
#include <chrono>
#include <functional>

using namespace std;
using namespace std::experimental;

// clang++ -std=c++2a -fcoroutines-ts -lstdc++ awaitor.cpp -o demo

struct Add100Awaitable
{
	Add100Awaitable(int init) : _value(init) {}

	bool await_ready() const { return false; }

	int await_resume() { return _value; }

	void await_suspend(coroutine_handle<> handle)
	{
		std::thread([this, handle]() mutable {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			std::cout << "Add100ByCallback: " << this->_value << std::endl;

			this->_value += 100;

			//resume coroutine
			handle.resume();
		}).detach();
	}

private:
	int _value;
};

struct Task
{
	// struct promise_type;
	// using co_handle = coroutine_handle<promise_type>;

	struct promise_type
	{
		auto get_return_object() { return Task{}; }
		//auto get_return_object() { return Task{co_handle::from_promise(*this)}; }
		auto initial_suspend() { return suspend_never{}; }
		auto final_suspend() { return suspend_never{}; }
		void unhandled_exception() { std::terminate(); }
		void return_void() {}
	};

	//~Task() { handle.destroy(); }

	//co_handle handle;
};

Task Add100ByCoroutine(int init)
{
	int ret = co_await Add100Awaitable(init);
	ret = co_await Add100Awaitable(ret);
	ret = co_await Add100Awaitable(ret);

	cout << "result : " << ret << endl;
}

int main()
{
	Add100ByCoroutine(10);

	getchar();

	return 0;
}