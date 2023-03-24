#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <numbers>
#include <string>
#include <thread>
#include <vector>
#include <latch>

using namespace std::literals;

void background_work(std::stop_token st, const int id, const std::string text,
    const std::chrono::milliseconds delay, std::latch& all_ready)
{
    all_ready.arrive_and_wait();

    std::cout << "Thread#" << id << " started..." << std::endl;

    for (const auto& letter : text)
    {        
        if (st.stop_requested())
        {
            std::cout << "Task#" << id << " is stopped..." << std::endl;
            return;
        }

        std::cout << "Thread#" << id << " - " << letter << std::endl;
        std::this_thread::sleep_for(delay);
    }
}

TEST_CASE("jthread")
{
    std::stop_source stop_src;
    auto stop_tkn = stop_src.get_token();
    std::latch all_ready{2};

    std::jthread thd1(background_work, stop_tkn,  1, "THD#111111111111111", 500ms, std::ref(all_ready));
    std::jthread thd2(background_work, stop_src.get_token(), 2, "THD#222222222222222222", 100ms, std::ref(all_ready));

    std::this_thread::sleep_for(2s);

    stop_src.request_stop();

    //may_throw();
}

