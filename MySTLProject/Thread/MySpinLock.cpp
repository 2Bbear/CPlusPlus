#pragma warning(disable: 4819)

#include <thread>
#include "MySpinLock.h"

using namespace std;

void MySpinLock::lock()
{
    bool expected = false;//기대하는 값
    bool desired = true;//expected와 같다면 대입할 값.
    //CAS Compare And Swap
    while (_locked.compare_exchange_strong(expected, desired) == false)
    {
        expected = false;
    }
}

void MySpinLock::unlock()
{
    _locked.store(false);
}