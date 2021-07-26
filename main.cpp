#include <iostream>
#include <mutex>
#include <Windows.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>
#include <queue>
using namespace std;

mutex m;
queue<int32_t> q;
 HANDLE handle ;

condition_variable cv; // 조건변수 표준 mutex와 짝지어서 작동함 , 유저 레벨 오브젝트

//데이터를 밀어넣는 함수
void Producer()
{
    while(true)
    {
        
        {
            unique_lock<mutex> lock(m);
            q.push(100);
        }
        ::SetEvent(handle); // 이벤트의 상태를 signal 상태로 변경함.
        this_thread ::sleep_for(1000ms); // 1초에 한번씩 작동하게 끔
        
    }
}
void Consumer()
{
    while(true)
    {
        ::WaitForSingleObject(handle, INFINITE); // handle이 시그널 상태가 될때까지 대기함. 이제 이 thread는 커널 모드로 전환이 된다.
        
        unique_lock<mutex> lock(m);
        if(q.empty() == false)
        {
            int32_t data= q.front();
            q.pop();
            cout<<q.size()<<endl;
        }
        //::ResetEvent(handle); handle이 auto reset이 아닐 경우 다시 no signal 상태로 바꿔 주어야 하기 때문에 이 함수를 사용한다.
    }
}

int main()
{
    //커널 오브젝트. 이벤트 생성.
    handle = ::CreateEvent(NULL/*보안속성*/,FALSE/*false==autoreset*/,FALSE/*초기 시그널 상태*/,NULL/*이름*/); 

    thread t1(Producer);
    thread t2(Consumer);

    t1.join();
    t2.join();

    ::CloseHandle(handle);

    return 0;
}