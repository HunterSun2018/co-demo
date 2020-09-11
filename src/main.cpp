#include <iostream>
#include <coroutine>
#include <chrono>
#include <thread>
#include <functional>
//#include <ranges>
#include "co_helper.hpp"

using namespace std;
using namespace co_helper;

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

Task<int> co_add100(int init)
{
    int ret = co_await Add100Awaitable(init);
    cout << "ret = " << ret << endl;

    ret = co_await Add100Awaitable(ret);
    cout << "ret = " << ret << endl;

    ret = co_await Add100Awaitable(ret);
    cout << "ret = " << ret << endl;

    co_return ret;
}
///
/// calculate fibonacci numbers
///
Generator<uint64_t> fibonacci()
{
    uint64_t a = 0, b = 1;
    while (true)
    {
        co_yield b;
        auto tmp = a;
        a = b;
        b += tmp;
    }
}

int main()
{
    auto ret = co_add100(10);

    cout << "co_add100 is running in coroutine." << endl;

    getchar();

    cout << "result : " << ret.get() << endl;

    auto fibs = fibonacci();

    for (auto i : fibs)
    {
        if (i > 1'000'000)
            break;
        cout << i << endl;
    }

    return 0;
}
