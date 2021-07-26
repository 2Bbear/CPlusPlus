#include<thread>
#include "CountCheckSpinLock.h"

using namespace std;


void CountCheckSpinLock::lock()
{
    bool expected = false;
    bool desired = true;

    int32_t curCount = 0;

    //CAS Compare And Swap
    while (_locked.compare_exchange_strong(expected, desired) == false)
    {
        expected = false;

        if (curCount> reTryCount)
        {
            curCount = 0;
            this_thread::sleep_for(0ms); 
        } 
        ++curCount;
    }
}

void CountCheckSpinLock::unlock()
{
    _locked.store(false);
}