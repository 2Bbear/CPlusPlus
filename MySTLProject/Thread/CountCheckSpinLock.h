#pragma once
#include <atomic>

class CountCheckSpinLock
{
public:
    CountCheckSpinLock(int32_t _reTryCount = 1) :reTryCount(_reTryCount)
    {
    };
public:
    void lock();
    void unlock();
   
private:
    std::atomic<bool> _locked = false;
    const int32_t reTryCount = 1; 
    
};

/* 


#include <iostream>
#include <mutex>
#include <Windows.h>

#include "MySpinLock.h"
using namespace std;

int32_t sum = 0;
mutex m;
MySpinLock spinLock;

void Add()
{
    for (int32_t i=0;i<10'0000; i++)
    {
        lock_guard<MySpinLock> guard(spinLock);
        sum++;
    }
}

void Sub()
{
    for (int32_t i = 0; i < 10'0000; i++)
    {
        lock_guard<MySpinLock> guard(spinLock);
        sum--;
    }
}


int main()
{
    thread t1(Add);
    thread t2(Sub);

    t1.join();
    t2.join();

    cout << sum << endl;

    return 0;
}

*/