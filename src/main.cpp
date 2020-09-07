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

        std::suspend_always yield_value(T v)
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
        void operator++() { /*handle.resume();*/ }
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
    ret = co_await Add100Awaitable(ret);
    ret = co_await Add100Awaitable(ret);

    for (int i = 0; i < 5; i++)
        co_yield co_await Add100Awaitable(ret);

    co_return ret;
}

template <class T>
struct generator
{
    struct promise_type
    {
        auto get_return_object()
        {
            return generator(std::coroutine_handle<promise_type>::from_promise(*this));
        }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() { throw; }
        std::suspend_always yield_value(T value)
        {
            current_value = std::move(value);
            return {};
        }
        void await_transform() = delete; // disallow co_await in generator coroutines
        T current_value;
    };

    generator(std::coroutine_handle<promise_type> h) : coro(h) {}
    generator(generator &&other) : coro(other.coro) { other.coro = {}; }
    ~generator()
    {
        if (coro)
            coro.destroy();
    }

    // range-based for support
    struct iter
    {
        explicit iter(std::coroutine_handle<promise_type> h) : coro(h) {}
        void operator++() { /*coro.resume();*/ }
        T operator*() const { return coro.promise().current_value; }
        bool operator==(std::default_sentinel_t) const { return coro.done(); }

    private:
        std::coroutine_handle<promise_type> coro;
    };
    iter begin()
    {
        coro.resume();
        return iter(coro);
    }
    std::default_sentinel_t end() { return {}; }

private:
    std::coroutine_handle<promise_type> coro;
};

generator<int> ints(int x)
{
    for (int i = 0; i < x; ++i)
        co_yield i;
}

int main()
{
    // for (auto i : ints(5))
    //     std::cout << i << '\n';

    auto ret = Add100ByCoroutine(10);

    for(auto i : ret)
        cout << i << endl;

    getchar();

    cout << "result : " << ret.get() << endl;

    return 0;
}
