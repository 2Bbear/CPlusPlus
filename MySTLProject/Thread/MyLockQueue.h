#pragma once
#include <mutex>
#include <atomic>
#include<queue>
using namespace std;

template<typename T>
class LockQueue
{
private:
    queue<T> _queue;
    mutex _mutex;
    condition_variable _condVar;
public:
    LockQueue() {}
    LockQueue(const LockQueue&) = delete;
    LockQueue& operator=(const LockQueue&) = delete;

    void Push(T _value)
    {
        lock_guard<mutex> lock(_mutex);
        _queue.push(std::move(_value));
        _condVar.notify_one();
    }
    bool TryPop(T& _value)
    {
        lock_guard<mutex> lock(_mutex);
        if (_queue.empty())
            return false;

        _value = std::move(_queue.front());
        _queue.pop();
        return true;
    }

    void WaitPop(T& _value)
    {
        unique_lock<mutex> lock(_mutex);
        _condVar.wait(lock, [&_queue] {return _queue->empty() == false; });
        _value = std::move(_queue.top());
        _queue.pop();
    }

};
