#include "utils/TimerTaskQueue.hpp"
#include <iostream>
#include <boost/chrono.hpp>
#include <boost/chrono/chrono_io.hpp>
#include <boost/chrono.hpp>
#include <unistd.h>

static void handler(void)
{
    KLib::TimerTaskQueue::timer_type::TimePoint now = KLib::TimerTaskQueue::timer_type::Clock::now();
    std::time_t t = KLib::TimerTaskQueue::timer_type::Clock::to_time_t(now);    
    std::cout << std::ctime(&t) << std::endl;
}

int main(void)
{
    KLib::TimerTaskQueue timer_queue;
    timer_queue.runEvery(handler, KLib::TimerTaskQueue::timer_type::Seconds(4));
    boost::thread t([&timer_queue](){timer_queue.exec();});
    
    ::sleep(21);
    timer_queue.exit();
    return 0;
}