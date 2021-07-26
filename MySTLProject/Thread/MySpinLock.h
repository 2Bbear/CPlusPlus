#pragma once
#include <atomic>

class MySpinLock
{
public:
    void lock();
    void unlock();
   
private:
    std::atomic<bool> _locked = false;
    
};

