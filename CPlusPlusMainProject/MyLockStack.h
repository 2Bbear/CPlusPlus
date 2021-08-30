#pragma once
#include <mutex>
#include <atomic>
#include <stack>

using namespace std;

template<typename T>
class LockStack
{
private:
    stack<T> _stack;
    mutex _mutex;
    condition_variable _condVar;
public:
    LockStack() {}
    LockStack(const LockStack&) = delete; //복사 시도 막기
    LockStack& operator=(const LockStack&) = delete;// 복사 연산자 막기
public:
    void Push(T _value)
    {
        lock_guard<mutex> lock(_mutex);
        _stack.push(std::move(_value));
        _condVar.notify_one();
    }

    void TryPop(T& _value)
    {
        lock_guard<mutex> lock(_mutex);
        if (_stack.empty())
            return false;

        _value = std::move(_stack.top());
        _stack.pop();
        return true;
    }

    void WaitPop(T& _value)
    {
        unique_lock<mutex> lock(_mutex);
        _condVar.wait(lock, [&_stack] {return _stack->empty() == false; });
        _value = std::move(_stack.top());
        _stack.pop();
    }
};

