/*
 * @Author: Hunter
 * @Date: 2020-09-11 11:36:35
 * @LastEditTime: 2020-09-11 15:05:08
 * @LastEditors: Please set LastEditors
 * @Description: structs for coroutine helper 
 * @FilePath: /co_demo/src/co_helper.hpp
 */
#include <coroutine>

namespace co_helper
{
    ///
    /// struct Task for coroutine operator co_await and co_return
    ///
    template <typename T>
    struct Task
    {
        struct promise_type;
        using co_handle = std::coroutine_handle<promise_type>;
        co_handle _handle;

        struct promise_type
        {
            //auto get_return_object() { return Task{}; }
            auto get_return_object() { return Task{co_handle::from_promise(*this)}; }
            auto initial_suspend() { return std::suspend_never{}; }
            auto final_suspend() { return std::suspend_never{}; }

            void unhandled_exception() { std::terminate(); }

            auto return_value(T v)
            {
                value = v;
                return std::suspend_never{};
            }

            T value;
        };

        T get()
        {
            return _handle.promise().value;
        }

        ~Task()
        {
            if (_handle)
                _handle.destroy();            
        }
    };

    ///
    /// struct Generator for coroutine operator co_yield
    ///
    template <typename T>
    struct Generator
    {
        struct promise_type;
        using co_handle = std::coroutine_handle<promise_type>;
        co_handle _handle;

        ~Generator()
        {
            if (_handle)
                _handle.destroy();            
        }

        struct promise_type
        {
            //auto get_return_object() { return Task{}; }
            auto get_return_object() { return Generator{co_handle::from_promise(*this)}; }
            auto initial_suspend() { return std::suspend_never{}; }
            auto final_suspend() { return std::suspend_never{}; }

            void unhandled_exception() { std::terminate(); }

            auto yield_value(T v)
            {
                value = std::move(v);
                return std::suspend_always{};
            }

            T value;
        };

        // range-based for support
        struct iter
        {
            explicit iter(co_handle h) : _handle(h) {}

            void operator++() { _handle.resume(); }

            T operator*() const { return _handle.promise().value; }

            bool operator==(std::default_sentinel_t) const { return _handle.done(); }

        private:
            co_handle _handle;
        };

        iter begin()
        {
            return iter(_handle);
        }

        std::default_sentinel_t end() { return {}; }
    };

} // namespace co_helper
