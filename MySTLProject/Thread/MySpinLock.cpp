#pragma warning(disable: 4819)

#include <thread>
#include "MySpinLock.h"

using namespace std;

void MySpinLock::lock()
{
    bool expected = false;
    bool desired = true;
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