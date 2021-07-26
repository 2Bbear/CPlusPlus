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

condition_variable cv; // ���Ǻ��� ǥ�� mutex�� ¦��� �۵��� , ���� ���� ������Ʈ

//�����͸� �о�ִ� �Լ�
void Producer()
{
    while(true)
    {
        
        {
            unique_lock<mutex> lock(m);
            q.push(100);
        }
        ::SetEvent(handle); // �̺�Ʈ�� ���¸� signal ���·� ������.
        this_thread ::sleep_for(1000ms); // 1�ʿ� �ѹ��� �۵��ϰ� ��
        
    }
}
void Consumer()
{
    while(true)
    {
        ::WaitForSingleObject(handle, INFINITE); // handle�� �ñ׳� ���°� �ɶ����� �����. ���� �� thread�� Ŀ�� ���� ��ȯ�� �ȴ�.
        
        unique_lock<mutex> lock(m);
        if(q.empty() == false)
        {
            int32_t data= q.front();
            q.pop();
            cout<<q.size()<<endl;
        }
        //::ResetEvent(handle); handle�� auto reset�� �ƴ� ��� �ٽ� no signal ���·� �ٲ� �־�� �ϱ� ������ �� �Լ��� ����Ѵ�.
    }
}

int main()
{
    //Ŀ�� ������Ʈ. �̺�Ʈ ����.
    handle = ::CreateEvent(NULL/*���ȼӼ�*/,FALSE/*false==autoreset*/,FALSE/*�ʱ� �ñ׳� ����*/,NULL/*�̸�*/); 

    thread t1(Producer);
    thread t2(Consumer);

    t1.join();
    t2.join();

    ::CloseHandle(handle);

    return 0;
}