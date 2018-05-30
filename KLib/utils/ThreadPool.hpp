#ifndef __THREADPOOL_HPP__
#define __THREADPOOL_HPP__

#include <boost/thread.hpp>
#include <list>
#include <vector>

namespace KLib
{
class ThreadPool
{
public:
    ThreadPool(int threadNum):
        _threadNum(threadNum)
    {
        if (_threadNum <= 0)
        {
            _threadNum = 1;
        }
    }

    void init()
    {
        for (int cnt = 0; cnt < _threadNum; ++cnt)
        {
            boost::shared_ptr<boost::thread> t(new boost::thread(boost::bind(&ThreadPool::_run, this)));
            _threads.push_back(t);
        }
    }

    void post(boost::function<void(void)> const& task)
    {
        {
            boost::lock_guard<boost::mutex> guard(_mtx);
            _tasks.push_front(task);
        }
        
        //惊群
        _cond.notify_all();
    }

    virtual ~ThreadPool()
    {
        for (std::vector<boost::shared_ptr<boost::thread> >::const_iterator pos = _threads.begin(); pos != _threads.end(); ++pos)
        {
            (*pos)->join();
        }
    }

private:
    void _run()
    {
        boost::function<void(void)> task;
        boost::unique_lock<boost::mutex> lk(_mtx);
        _cond.wait(lk, boost::bind(&ThreadPool::_hasTask, this));
        task = _tasks.back();
        _tasks.pop_back();
        lk.unlock();

        task();
    }

    bool _hasTask()
    {
        return !_tasks.empty();
    }

    int  _threadNum;
    std::list<boost::function<void(void)> > _tasks;
    std::vector<boost::shared_ptr<boost::thread> > _threads;
    boost::mutex _mtx;
    boost::condition_variable _cond;    
};
}

#endif