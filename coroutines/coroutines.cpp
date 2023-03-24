#include <cassert>
#include <catch2/catch_test_macros.hpp>
#include <coroutine>
#include <iostream>
#include <string>
#include <syncstream>
#include <thread>
#include <vector>
#include <utility>
#include <ranges>

using namespace std::literals;

namespace Step_0
{
    void foo(int max)
    {
        std::cout << "..coro(START, " << max << ")\n";

        for (int value = 1; value <= max; ++value)
        {
            ///////////////////////////////////////////////////////
            std::cout << "..coro(" << value << ", " << max << ")\n";
        }

        std::cout << "..coro(END, " << max << ")\n";
    }
} // namespace Step_0

namespace Step_1
{
    struct TaskResumer
    {
        struct promise_type;

        using CoroutineHandle = std::coroutine_handle<promise_type>;

        struct promise_type
        {
            TaskResumer get_return_object()
            {
                return TaskResumer{CoroutineHandle::from_promise(*this)};
            }

            std::suspend_always initial_suspend() const noexcept { return {}; }

            std::suspend_always final_suspend() noexcept { return {}; }

            void unhandled_exception()
            {
                std::terminate();
            }

            void return_void()
            { }
        };

        TaskResumer(CoroutineHandle coroutine)
            : coroutine_{coroutine}
        { }

        TaskResumer(const TaskResumer&) = delete;
        TaskResumer& operator=(const TaskResumer&) = delete;
        TaskResumer(TaskResumer&&) = delete;
        TaskResumer& operator=(TaskResumer&&) = delete;

        ~TaskResumer() noexcept
        {
            if (coroutine_)
                coroutine_.destroy();
        }

        bool resume() const
        {
            if (!coroutine_ || coroutine_.done())
                return false;

            coroutine_.resume(); // resuming suspended coro

            return !coroutine_.done();
        }

    private:
        CoroutineHandle coroutine_;
    };

    TaskResumer foo(int max)
    {
        std::string str = "HELLO";
        std::cout << "..coro(START, " << max << ")\n";

        for (int value = 1; value <= max; ++value)
        {
            std::cout << "..coro(" << value << ", " << max << ")\n";
            ///////////////////////////////////////////////////////
            co_await std::suspend_always{}; // SUSPENSION POINT
        }

        std::cout << "..coro(END, " << max << ")\n";
    }
} // namespace Step_1

TEST_CASE("first coroutine")
{
    using namespace Step_1;

    TaskResumer task = foo(5);

    while (task.resume())
    {
        std::cout << "coro() suspended...\n";
    }
    std::cout << "coro() done!\n";
}

///////////////////////////////////////////////////////////////////////////////////////////////
// custom Awaiter

inline auto sync_out(std::ostream& out = std::cout)
{
    return std::osyncstream{out};
}

struct FireAndForget
{
    struct promise_type
    {
        FireAndForget get_return_object()
        {
            return {};
        }

        std::suspend_never initial_suspend() const noexcept
        {
            sync_out() << "...Initial suspension point...\n";
            return {};
        }

        std::suspend_never final_suspend() const noexcept
        {
            sync_out() << "...Final suspension point...\n";
            return {};
        }

        void unhandled_exception() { std::terminate(); }

        void return_void() const noexcept
        {
            sync_out() << "...Exiting the coroutine...\n";
        }
    };
};

auto resume_on_new_thread()
{
    // Awaiter
    struct ResumeOnNewThreadAwaiter : std::suspend_always
    {
        void await_suspend(std::coroutine_handle<> coroutine_hndl)
        {
            std::thread([coroutine_hndl] { coroutine_hndl.resume(); }).detach();
        }

        std::thread::id await_resume() const noexcept
        {
            return std::this_thread::get_id();
        }
    };

    return ResumeOnNewThreadAwaiter{};
}

FireAndForget coro_on_many_threads(int id)
{
    const int max_step = 3;
    int step = 1;
    sync_out() << "Coro#" << id << " - Part#" << step << "/" << max_step << " - started on THD#" << std::this_thread::get_id() << "\n";

    std::thread::id thd_id = co_await resume_on_new_thread();

    ++step;
    sync_out() << "Coro#" << id << " - Part#" << step << "/" << max_step << " - continues on THD#" << std::this_thread::get_id() << "\n";
    assert(thd_id == std::this_thread::get_id());

    thd_id = co_await resume_on_new_thread(); /////////////////////////////////// context switch

    ++step;
    sync_out() << "Coro#" << id << " - Part#" << step << "/" << max_step << " - ends on THD#" << std::this_thread::get_id() << "\n";
    assert(thd_id == std::this_thread::get_id());
}

// TEST_CASE("resume part of the function on the new thread")
// {
//     using namespace std::literals;

//     FireAndForget tsk1 = coro_on_many_threads(1);
//     FireAndForget tsk2 = coro_on_many_threads(2);
//     FireAndForget tsk3 = coro_on_many_threads(3);

//     std::this_thread::sleep_for(5s);
// }

////////////////////////////////////////////////////////////////////
//

namespace FutureStd
{

    template <typename T>
    class [[nodiscard]] Generator
    {
    public:
        struct promise_type;

        using CoroutineHandle = std::coroutine_handle<promise_type>;

        struct promise_type
        {
            Generator get_return_object()
            {
                return Generator{CoroutineHandle::from_promise(*this)};
            }

            std::suspend_always initial_suspend() const { return {}; }

            std::suspend_always final_suspend() const noexcept { return {}; }

            void unhandled_exception() { std::terminate(); }

            std::suspend_always yield_value(auto&& yielded_value)
            {
                value = std::forward<decltype(yielded_value)>(yielded_value);
                return {};
            }

            void return_void() { }

            T value;
        };

        struct iterator
        {
            using value_type = T;
            using reference = T;
            using iterator_category = std::input_iterator_tag;

            CoroutineHandle coroutine_handle_ = nullptr;

            iterator() = default;

            iterator(auto coroutine_handle)
                : coroutine_handle_{coroutine_handle}
            { }

            T operator*() const
            {
                assert(coroutine_handle_ != nullptr);
                return coroutine_handle_.promise().value;
            }

            T* operator->() const
            {
                assert(coroutine_handle_ != nullptr);
                return &coroutine_handle_.promise().value;
            }

            iterator& operator++()
            {
                move_to_next();
                return *this;
            }

            iterator operator++(int)
            {
                iterator prev_pos = *this;
                move_to_next();
                return prev_pos;
            }

            bool operator==(const iterator& other) const = default;

        private:
            friend class Generator;

            void move_to_next()
            {
                if (coroutine_handle_ && !coroutine_handle_.done())
                {
                    coroutine_handle_.resume();

                    if (coroutine_handle_.done())
                    {
                        coroutine_handle_ = nullptr;
                    }
                }
            }
        };

        Generator(CoroutineHandle coroutine_hndl)
            : coroutine_hndl_{coroutine_hndl}
        { }

        Generator(const Generator&) = delete;
        Generator& operator=(const Generator&) = delete;

        ~Generator()
        {
            if (coroutine_hndl_)
                coroutine_hndl_.destroy();
        }

        std::optional<T> next_value()
        {
            assert(coroutine_hndl_);
            // if (!coroutine_hndl_ || coroutine_hndl_.done())
            //     return std::nullopt;

            coroutine_hndl_.resume();

            if (coroutine_hndl_.done())
                return std::nullopt;

            return coroutine_hndl_.promise().value;
        }

        iterator begin() const
        {
            if (!coroutine_hndl_ || coroutine_hndl_.done())
                return {};

            iterator it{coroutine_hndl_};
            it.next();
            return it;
        }

        iterator end() const
        {
            return {};
        }

        iterator begin()
        {
            if (!coroutine_hndl_ || coroutine_hndl_.done())
                return {};

            iterator it{coroutine_hndl_};
            it.move_to_next();
            return it;
        }

        iterator end()
        {
            return {};
        }

    private:
        CoroutineHandle coroutine_hndl_;
    };
} // namespace FutureStd

using FutureStd::Generator;

Generator<int> fibonacci(int n)
{
    auto a = 0, b = 1;

    while (a < n)
        co_yield std::exchange(a, std::exchange(b, a + b));
}

namespace views = std::ranges::views;

TEST_CASE("fibonacci with generator")
{
    for (const auto& item : fibonacci(100))
        std::cout << item << " ";
    std::cout << "\n";
}