//
// Created by lee on 18-5-2.
//

#ifndef KLIB_RATELIMITER_HPP
#define KLIB_RATELIMITER_HPP

#include <stdint.h>
#include <sys/time.h>
#include <stdio.h>

namespace KLib
{
    class RateLimiter {
    public:
        RateLimiter(::uint64_t qps, ::uint64_t peak):
                _qpms(qps/1000),
                _peak(peak),
                _tokens(0)
        {
            _last_req_time.tv_usec = 0;
            _last_req_time.tv_sec = 0;
        }
        /*
         * @token_num : 请求令牌的个数
         * 返回值： 需要等待的微秒数
         * */
        uint64_t request(uint64_t token_num)
        {
            _produce();
            if (_tokens >= token_num)
            {
                _tokens -= token_num;
                return 0;
            }else{
                uint64_t wait_for_num = token_num - _tokens;
                uint64_t wait_us = 1000000*(double(wait_for_num) / _qpms);
                return wait_us;
            }
        }

    private:
        void _produce(void)
        {
            if (_tokens >= _peak){
                return;
            }

            //第一次调用
            if (_last_req_time.tv_sec == 0 && _last_req_time.tv_usec == 0){
                ::gettimeofday(&_last_req_time, NULL);
                _tokens = _qpms;

                return ;
            }

            ::timeval now;
            ::gettimeofday(&now, NULL);

            uint64_t delta = 1000*(now.tv_sec - _last_req_time.tv_sec) + (now.tv_usec - _last_req_time.tv_usec) / 1000;
            if (delta <= 0)
            {
                return;
            }

            uint64_t delta_q = delta * _qpms;
            uint64_t tmp = delta_q + _tokens;
            if (tmp >= _peak)
            {
                tmp = _peak;
            }

            _tokens = tmp;
            _last_req_time = now;
        }

        double _qpms;
        ::uint64_t _peak;
        ::uint64_t  _tokens;
        ::timeval _last_req_time;
    };
}

#endif //KLIB_RATELIMITER_HPP
