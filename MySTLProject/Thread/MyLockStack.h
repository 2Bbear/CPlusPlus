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

/*
test 코드
#include <iostream>
#include <thread>
#include "../MySTLProject/Thread/MyLockQueue.h"
#include "../MySTLProject/Thread/MyLockStack.h"

using namespace std;


LockQueue<int32_t> q;
LockStack<int32_t> s;

void Push()
{
    while (true)
    {
        int32_t value = rand() % 100;
        q.Push(value);

        this_thread::sleep_for(10ms);
    }
}
void Pop()
{
    while (true)
    {
        int32_t data = 0;
        if (q.TryPop(data))
            cout << data << endl;
    }
}
int main()
{
    thread t1(Push);
    thread t2(Pop);
    thread t3(Pop);

    t1.join();
    t2.join();
    t3.join();
}
*/