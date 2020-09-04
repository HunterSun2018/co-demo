#include <iostream>
#include <coroutine>
#include <chrono>
#include <thread>
#include <functional>

using namespace std;

struct Add100Awaitable
{
    Add100Awaitable(int init) : _value(init) {}
    bool await_ready() const { return false; }
    int await_resume() { return _value; }
    void await_suspend(coroutine_handle<> handle)
    {
        std::thread([this, handle]() {
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

template <typename T>
struct Task
{
    struct promise_type;
    using co_handle = coroutine_handle<promise_type>;

    struct promise_type
    {
        //auto get_return_object() { return Task{}; }
        auto get_return_object() { return Task{co_handle::from_promise(*this)}; }
        auto initial_suspend() { return suspend_never{}; }
        auto final_suspend() { return suspend_never{}; }
        void unhandled_exception() { std::terminate(); }
        auto return_value(T v)
        {
            value = v;
            return suspend_never{};
        }

        T value;
    };

    T get()
    {
        return handle.promise().value;
    }

    ~Task()
    {
        //handle.destroy();
        cout << "~Task() ..." << endl;
    }

    co_handle handle;
};

Task<int> Add100ByCoroutine(int init)
{
    int ret = co_await Add100Awaitable(init);
    ret = co_await Add100Awaitable(ret);
    ret = co_await Add100Awaitable(ret);

    co_return ret;
}

int main()
{
    auto ret = Add100ByCoroutine(10);

    getchar();

    cout << "result : " << ret.get() << endl;

    return 0;
}
