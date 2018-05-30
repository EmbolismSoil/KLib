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
#include <boost/bind.hpp>    
#include <sys/eventfd.h>
#include <stdint.h>
#include "ThreadPool.hpp"

namespace KLib
{
    class TimerTaskQueue
    {
    public:
        typedef Timer<boost::function<void(void)> > timer_type;
        TimerTaskQueue(int timeout_ms = -1, int threads = 1):
            _timeout(timeout_ms),
            _workers(threads)
        {
            _workers.init();
            _timerfd = ::timerfd_create(CLOCK_REALTIME, 0);
            if (_timerfd < 0)
            {
                std::stringstream ss;
                char buf[1024];
                strerror_r(errno, buf, sizeof(buf));
                ss << "failed in timerfd_create, ret = " << _timerfd << ", " << buf;
                throw std::runtime_error(ss.str());
            }

            _eventfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
            if (_eventfd < 0)
            {
                std::stringstream ss;
                char buf[1024];
                strerror_r(errno, buf, sizeof(buf));
                ss << "failed in timerfd_create, ret = " << _eventfd << ", " << buf;
                throw std::runtime_error(ss.str());
            }

            _run.store(true);

            ::pollfd pfd;
            pfd.fd = _timerfd;
            pfd.events = (POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND);

            _fds.push_back(pfd);

            pfd.fd = _eventfd;
            _fds.push_back(pfd);
        }

        virtual ~TimerTaskQueue()
        {

        }

        void runAt(boost::function<void(void)> const& handler, timer_type::TimePoint timePoint)
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
            timer_type::TimePoint timeout = boost::chrono::system_clock::now() + duration;
            runAt(handler, timeout);
        }

        template<class DT>
        void runEvery(boost::function<void(void)> const& handler, DT const& period)
        {
            timer_type::TimePoint timeout = boost::chrono::system_clock::now() + period;
            TimerPtr timer(new timer_type(timeout, period, handler));
            {
                boost::lock_guard<boost::mutex> guard(_timersMtx);
                _timers[timer] = false;                
            }

            _updateTimeoutPoint();
        }

        template<class DT>
        void runEvery(boost::function<void(void)> const& handler,timer_type::TimePoint const& timePoint, DT const& period)
        {
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
                    for (std::vector<pollfd>::const_iterator pos = _fds.begin(); pos != _fds.end() && ret; ++pos )
                    {
                        ::pollfd const& pfd = *pos;                        

                        if (pfd.revents & (POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND))
                        {
                            if (pfd.fd == _timerfd){
                                ::itimerspec time;
                                timerfd_gettime(_timerfd, &time);
                                handleTimeout(time);
                            }else{
                                //eventfd
                                uint64_t tmp;
                                ::read(_eventfd, &tmp, sizeof(tmp));
                                (void)tmp;
                            }
                            --ret;
                        }
                    }
                }

            }
        }

        void exit()
        {
            _run.store(false, boost::memory_order_release);
            _wakeUp();
        }
        
    private:
        typedef boost::shared_ptr<timer_type> TimerPtr;

        void _wakeUp()
        {
            uint64_t counter = 0;
            ::write(_eventfd, &counter, sizeof(counter));
        }

        static void _handle(TimerPtr ptr)
        {
            ptr->handle();
        }        

        struct _TimerCmp{
        public:
            bool operator()(TimerPtr const& x, TimerPtr const& y)
            {
                return x->getTimeoutPoint() < y->getTimeoutPoint();
            }
        };

        void handleTimeout(::itimerspec const& time)
        {
            if (_timers.empty())
            {
                uint64_t exp;
                ssize_t s = read(_timerfd, &exp, sizeof(uint64_t));
                (void)s;

                return;
            }

            timer_type::TimePoint now = boost::chrono::system_clock::now();
            std::vector<TimerPtr> timeoutTimers;
            std::map<TimerPtr, bool, _TimerCmp>::iterator pos = _timers.begin();

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
                boost::lock_guard<boost::mutex> guard(_timersMtx);
                _timers.erase(_timers.begin(), pos);
            }

            for (std::vector<TimerPtr>::iterator pos = timeoutTimers.begin(); pos != timeoutTimers.end(); ++pos)
            {
                _workers.post(boost::bind(&TimerTaskQueue::_handle, *pos));
            }
            
            {
                boost::lock_guard<boost::mutex> guard(_timersMtx);
                for (std::vector<TimerPtr>::iterator pos = timeoutTimers.begin(); pos != timeoutTimers.end(); ++pos)
                {
                    if ((*pos)->next() == 0)
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

                ::time_t ms = boost::chrono::duration_cast<timer_type::Milliseconds>(timePoint.time_since_epoch()).count();
                ::itimerspec time = 
                {
                    {0, 0},
                    {ms / 1000, (ms % 1000) * 1000000}
                };
    
                ::timerfd_settime(_timerfd, TFD_TIMER_ABSTIME, &time, NULL);
            }

        }        

        std::map<TimerPtr, bool, _TimerCmp> _timers;
        int _timerfd;
        int _eventfd;

        boost::atomic<bool> _run;
        std::vector<pollfd> _fds;
        int const _timeout;
        boost::mutex _timersMtx;

        ThreadPool _workers;
    };
}

#endif
