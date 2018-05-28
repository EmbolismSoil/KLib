#include "utils/TimerTaskQueue.hpp"
#include <iostream>
#include <boost/chrono.hpp>

static void handler(void)
{
    KLib::TimerTaskQueue::timer_type::TimePoint now = boost::chrono::high_resolution_clock::now();    
    std::cout << "now : " << boost::chrono::duration_cast<KLib::TimerTaskQueue::timer_type::Seconds>(now).count() << std::endl;
}

int main(void)
{
    KLib::TimerTaskQueue::timer_type::TimePoint now = boost::chrono::high_resolution_clock::now();
    std::cout << "now : " << boost::chrono::duration_cast<KLib::TimerTaskQueue::timer_type::Seconds>(now).count() << std::endl;
    KLib::TimerTaskQueue timer_queue;
    timer_queue.runAfter(handler, KLib::TimerTaskQueue::timer_type::Seconds(2));

    timer_queue.runEvery(handler, KLib::TimerTaskQueue::timer_type::Seconds(4));

    timer_queue.exec();
    
    return 0;
}