#pragma once
#include <atomic>

/*
lock, unlock 함수만 소문자로 시작하는 이유는 lock_guard에서 호출하는 시그니처가 소문자로 시작하는 형태를 취하고 있기 때문이다.

목적
- 기존 Spinklock의 높은 cpu 점유율을 줄여보자!

방법
- cpu 점유율을 높이는 while 문을 돌지 못하게 하자
    - Sleep을 통해 스레드 자체를 반환 시키자!
    - Yield 문을 이용해 같은 core의 다른 스레드에게 기회를 주자

제공된 tryCount 횟수만큼 재시도 후 sleep을 통해 커널모드로 스레드를 넘긴다.
*/
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
    const int32_t reTryCount = 1; // 재시도 횟수
    
};

/* 
스레드 테스트 코드

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