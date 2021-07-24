#include<thread>
#include "CountCheckSpinLock.h"

using namespace std;


void CountCheckSpinLock::lock()
{
    bool expected = false;//기대하는 값
    bool desired = true;//expected와 같다면 대입할 값.

    int32_t curCount = 0;

    //CAS Compare And Swap
    while (_locked.compare_exchange_strong(expected, desired) == false)
    {
        expected = false;

        if (curCount> reTryCount)
        {
            curCount = 0;
            this_thread::sleep_for(0ms); //숫자를 넣은 경우 그 현재의 타임 슬라이스를 포기하고 넣은 숫자만큼 재 스케줄링 루프에 진입하지 않음.
                                   // 0을 넣은 경우 현재의 타임 슬라이스를 포기하고 바로 재 스케줄링 루프에 진입함.
            //this_thread::yield(); // 현재 타임 슬라이스를 포기하고 재 스레드 스케줄링 루프에 진입함
        } 
        ++curCount;
    }
}

void CountCheckSpinLock::unlock()
{
    _locked.store(false);
}