#include <iostream>
#include <coroutine>
#include <chrono>
#include <thread>
#include <functional>
//#include <ranges>

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

        suspend_always yield_value(T v)
        {
            value = std::move(v);
            return {};
        }

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
        //cout << "~Task() ..." << endl;
    }

    // range-based for support
    struct iter
    {
        explicit iter(co_handle h) : handle(h) {}

        void operator++()
        {
            handle.resume();
        }

        T operator*() const { return handle.promise().value; }

        bool operator==(std::default_sentinel_t) const { return handle.done(); }

    private:
        co_handle handle;
    };

    iter begin()
    {
        //handle.resume();
        return iter(handle);
    }

    std::default_sentinel_t end() { return {}; }

    co_handle handle;
};

Task<int> Add100ByCoroutine(int init)
{
    int ret = co_await Add100Awaitable(init);
    cout << "ret = " << ret << endl;

    ret = co_await Add100Awaitable(ret);
    cout << "ret = " << ret << endl;

    ret = co_await Add100Awaitable(ret);
    cout << "ret = " << ret << endl;

    for (int i = 0; i < 5; i++)
    {
        ret = co_await Add100Awaitable(ret);
        co_yield ret;
    }

    co_return ret;
}

int main()
{
    // for (auto i : ints(5))
    //     std::cout << i << '\n';

    auto ret = Add100ByCoroutine(10);

    cout << "Add100ByCoroutine is running in coroutine." << endl;

    getchar();

    for (auto i : ret)
        cout << i << endl;

    cout << "result : " << ret.get() << endl;

    return 0;
}
