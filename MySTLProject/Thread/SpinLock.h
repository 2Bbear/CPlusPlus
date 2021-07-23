#pragma once
#include <atomic>

/*
lock, unlock 함수만 소문자로 시작하는 이유는 lock_guard에서 호출하는 시그니처가 소문자로 시작하는 형태를 취하고 있기 때문이다.
*/
class MySpinLock
{
public:
    void lock();
    void unlock();
   
private:
    std::atomic<bool> _locked = false;
    
};

