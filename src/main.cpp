#include <iostream>
#include <coroutine>
#include <chrono>
#include <thread>
#include <functional>

using namespace std;

// struct Task
// {
//     struct promise_type;
//     using co_handle = coroutine_handle<promise_type>;

//     struct promise_type
//     {
//         auto get_return_object() { return Task{co_handle::from_promise(*this)}; }
//         auto initial_suspend() { return std::suspend_never{}; }
//         auto final_suspend() { return std::suspend_never{}; }
//         void unhandled_exception() { std::terminate(); }
//         void return_void() {}
//     };

//     ~Task() { handle.destroy(); }

//     co_handle handle;
// };

// using call_back = std::function<void(int)>;
// void Add100ByCallback(int init, call_back f)
// {
//     std::thread t([init, f]() {
//         std::this_thread::sleep_for(std::chrono::seconds(1));
//         f(init + 100);
//     });

//     t.detach();
// }

// struct Add100AWaitable
// {
//     Add100AWaitable(int init) : init_(init) {}
//     bool await_ready() const { return false; }
//     int await_resume() { return result_; }
//     void await_suspend(coroutine_handle<> handle)
//     {
//         // 定义一个回调函数，在此函数中恢复协程
//         auto f = [handle, this](int value) mutable {
//             result_ = value;
//             handle.resume(); // 这句是关键
//         };
//         Add100ByCallback(init_, f);
//     }
//     int init_;   // 将参数存在这里
//     int result_; // 将返回值存在这里
// };

// Task Add100ByCoroutine(int init, call_back f)
// {
//     int ret = co_await Add100AWaitable(init);
//     ret = co_await Add100AWaitable(ret);
//     ret = co_await Add100AWaitable(ret);
//     f(ret);
// }

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

struct Task
{
    struct promise_type;
    using co_handle = coroutine_handle<promise_type>;

    struct promise_type
    {
        auto get_return_object() { return Task{}; }
        //auto get_return_object() { return Task{co_handle::from_promise(*this)}; }
        auto initial_suspend() { return suspend_never{}; }
        auto final_suspend() { return suspend_never{}; }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
    };

    // ~Task()
    // {
    //     //handle.destroy();
    //     cout << "~Task() ..." << endl;
    // }

    co_handle handle;
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