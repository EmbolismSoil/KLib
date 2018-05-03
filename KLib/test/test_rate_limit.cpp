//
// Created by lee on 18-5-3.
//

#include "algorithm/RateLimiter.hpp"
#include "utils/containerutils.hpp"
#include <boost/atomic.hpp>
#include <sys/time.h>
#include <boost/thread.hpp>
#include <iostream>
#include <stdlib.h>

 boost::atomic<uint64_t > req_ok_cnt;
 std::vector<uint64_t > points_req_ok;
 std::vector<uint64_t > points_req;
 boost::atomic<uint64_t > req_cnt;
 boost::atomic<int> us;

void t1_func()
{
    while(1)
    {
        ::usleep(1000000);//1s
        points_req_ok.push_back(req_ok_cnt.load());
        points_req.push_back(req_cnt.load());
        std::cout << "sample, req_cnt = " << req_cnt << ", req_ok_cnt = " << req_ok_cnt << std::endl;

        if (points_req.size() == 50)
        {
            us = 250;
        }

        if (points_req.size() == 100)
        {
            us = 100;
        }

        if (points_req.size() == 150)
        {
            std::cout << KLib::tostr(points_req) << std::endl;
            std::cout << KLib::tostr(points_req_ok) << std::endl;
            ::exit(0);
        }
    }
}

int main(void)
{
    us = 500;
    KLib::RateLimiter rateLimiter(1000, 10000);
    boost::thread t1(t1_func);
    for (int cnt = 0; cnt < 1000000; ++cnt) {
        if (rateLimiter.request(1) == 0) {
            ++req_ok_cnt;
        } else {
        }
        ++req_cnt;
        ::usleep(us);
    }

    return 0;
}