#ifndef __TIMER_HPP__
#define __TIMER_HPP__

#include <boost/chrono.hpp>

namespace KLib
{
    template<class T>
    class Timer
    {
    public:
        typedef boost::chrono::system_clock Clock;
        typedef Clock::time_point TimePoint;
        typedef boost::chrono::milliseconds Milliseconds;
        typedef boost::chrono::seconds Seconds;
        typedef boost::chrono::minutes Minutes;
        typedef boost::chrono::hours Hours;

        Timer(TimePoint timeout, Milliseconds period, T const& handler):
            _handler(handler),
            _timeout(timeout),
            _period(period)
        {

        }

        Timer(TimePoint timeout, Seconds period, T const& handler):
        _handler(handler),
        _timeout(timeout),
        _period(period)
        {

        }

        Timer(TimePoint timeout, Minutes period, T const& handler):
            _handler(handler),
            _timeout(timeout),
            _period(boost::duration_cast<Milliseconds>())
        {

        }

        Timer(TimePoint timeout, Hours period, T const& handler):
        _handler(handler),
        _timeout(timeout),
        _period(period)
        {

        }

        TimePoint getTimeoutPoint() const
        {
            return _timeout;
        }

        int next()
        {
            if (_isPeroid()) {
                _timeout = boost::chrono::system_clock::now() + _period;
                return 0;
            }else{
                return -1;
            }
        }

        virtual void handle()
        {
            _handler();
        }
        
    private:
        bool _isPeroid() const
        {
            return _period > Milliseconds(0);
        }

        T _handler;
        TimePoint _timeout;
        Milliseconds _period;    
    };
}
#endif