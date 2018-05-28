#ifndef __TIMERTASKQUEUE_HPP__
#define __TIMERTASKQUEUE_HPP__

#include "Timer.hpp"
#include <map>
#include <boost/shared_ptr.hpp>
#include <sys/timerfd.h>
#include <sstream>
#include <boost/atomic.hpp>
#include <poll.h>
#include <vector>
#include <iostream>
#include <boost/thread.hpp>
#include <boost/function.hpp>

namespace KLib
{
    class TimerTaskQueue
    {
    public:
        typedef Timer<boost::function<void(void)> > timer_type;
        TimerTaskQueue(int timeout_ms = -1):
            _timeout(timeout_ms)
        {
            _timerfd = ::timerfd_create(CLOCK_REALTIME, 0);
            if (_timerfd < 0)
            {
                std::stringstream ss;
                char buf[1024];
                strerror_r(errno, buf, sizeof(buf));
                ss << "failed in timerfd_create, ret = " << _timerfd << ", " << buf;
                throw std::runtime_error(ss.str());
            }
            _run.store(true);

            ::pollfd pfd;
            pfd.fd = _timerfd;
            pfd.events = (POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND);

            _fds.push_back(pfd);
        }

        void runAt(boost::function<void(void)> const& handler, Timer::TimePoint timePoint)
        {
            TimerPtr timer(new timer_type(timePoint, timer_type::Milliseconds(0), handler));
            {
                boost::lock_guard<boost::mutex> guard(_timersMtx);
                _timers[timer] = false;                
            }
            _updateTimeoutPoint();
        }

        template<class DT>
        void runAfter(boost::function<void(void)> const& handler, DT const& duration)
        {
            timer_type::TimePoint timeout = boost::chrono::high_resolution_clock::now() + duration;
            runAt(handler, timeout);
        }

        template<class DT>
        void runEvery(boost::function<void(void)> const& handler, DT const& period)
        {
            timer_type::TimePoint timeout = boost::chrono::high_resolution_clock::now() + period;
            TimerPtr timer(new timer_type(timePoint, period, handler));
            {
                boost::lock_guard<boost::mutex> guard(_timersMtx);
                _timers[timer] = false;                
            }

            _updateTimeoutPoint();
        }

        void exec()
        {
            while(_run.load(boost::memory_order_acquire))
            {
                int ret = ::poll(&*_fds.begin(), _fds.size(), _timeout);
                if (ret < 0)
                {
                    char buf[1024];
                    strerror_r(errno, buf, sizeof(buf));
                    std::cerr << "failed in poll, ret = " << ret << ", " << buf << std::endl;
                    continue;
                }else if (ret == 0){                    
                    std::cerr << "poll timeout" << std::endl;
                    continue;
                }else{
                    for (std::vector<::pollfd>::const_iterator pos = _fds.begin(); pos != _fds.end() && ret; ++pos )
                    {
                        ::pollfd const& pfd = *pos;
                        if (pfd.revents & (POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND))
                        {
                            ::itimerspec time;
                            timerfd_gettime(_timerfd, &time);
                            handleTimeout(time);
                            --ret;
                        }
                    }
                }

            }
        }

        void exit()
        {
            _run.store(false, boost::memory_order_release);
        }
        
    private:
        typedef boost::shared_ptr<timer_type> TimerPtr;

        static bool _timer_cmp(TimerPtr x, TimerPtr y)
        {
            return x->getTimeoutPoint() < y->getTimeoutPoint();
        }

        void handleTimeout(::itimerspec const& time)
        {
            if (_timers.empty())
            {
                uint64_t exp;
                ssize_t s = read(_timerfd, &exp, sizeof(uint64_t));
                (void)s;

                return;
            }

            timer_type::TimePoint timePoint = boost::chrono::high_resolution_clock::now();
            std::vector<TimerPtr> timeoutTimers;
            std::map<TimerPtr, bool, _timer_cmp>::const_iterator pos = _timers.begin();

            for (; pos != _timers.end(); ++pos)
            {
                TimerPtr timer = pos->first;
                if (timer->getTimeoutPoint() > now){
                    break;
                }

                timeoutTimers.push_back(timer);
            }

            if (timeoutTimers.empty())
            {
                return;
            }

            {
                boost::lock_guard<boost::mutex> guard(_timerMtx);
                _timers.erase(_timers.begin(), pos);
            }

            for (std::vector<TimerPtr>::iterator pos = timeoutTimers.begin(); pos != timeoutTimers.end(); ++pos)
            {
                pos->handle();
            }
            
            {
                boost::lock_guard<boost::mutex> guard(_timerMtx);
                for (std::vector<TimePtr>::iterator pos = timeoutTimers.begin(); pos != timeoutTimers.end(); ++pos)
                {
                    if (pos->next() == 0)
                    {
                        _timers[*pos] = false;
                    }
                }
            }

            uint64_t exp;
            ssize_t s = read(_timerfd, &exp, sizeof(uint64_t));
            (void)s;

            _updateTimeoutPoint();
        }

        void _updateTimeoutPoint()
        {
            if (_timers.empty()){
                return;
            }

            {
                boost::lock_guard<boost::mutex> guard(_timersMtx);
                TimerPtr timer = _timers.begin()->first;
                timer_type::TimePoint timePoint = timer->getTimeoutPoint();
                ::time_t epoch_time = timePoint.time_since_epoch();
                
                ::time_t ms = boost::chrono::duration_cast<timer_type::Milliseconds>(time_point).count();
                ::itimerspec time = 
                {
                    .it_interval = {0, 0},
                    .it_value = {ms / 1000, (ms % 1000) * 1000000}
                };
    
                ::timerfd_settime(_timerfd, TFD_TIMER_ABSTIME, &time, NULL);
            }

        }

        std::map<TimerPtr, bool, _timer_cmp> _timers;
        int _timerfd;

        boost::atomics::atomic<bool> _run;
        std::vector<::pollfd> _fds;
        int const _timeout;

        boost::mutex _timersMtx;
    };
}

#endif