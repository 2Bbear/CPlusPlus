#include <iostream>
#include <Windows.h>
#include <process.h>
#include <list>

#include "../MySTLProject/CThreadProfiler.h"



CThreadProfiler profile(true);

void Test1()
{
    profile.ProfileBegin(L"Test1");

    profile.ProfileEnd(L"Test1");
}

void Test2()
{
    profile.ProfileBegin(L"Test2");
    for (int i = 0; i < 10; i++)
    {
        Test1();
    }
    profile.ProfileEnd(L"Test2");
}
void Test3()
{
    profile.ProfileBegin(L"Test3");
    for (int i = 0; i < 10; i++)
    {
        Test2();
    }
    profile.ProfileEnd(L"Test3");
}

unsigned __stdcall TestFunc(void* pArguments)
{
    profile.ProfileBegin(L"TestFunc");

    for (int i = 0; i < 10; i++)
    {
        Test3();
    }



    profile.ProfileEnd(L"TestFunc");
    return 0;
}
int main()
{
    //스레드 생성하기
    HANDLE hThread1 = (HANDLE)_beginthreadex(NULL, 0, &TestFunc, NULL, 0, 0);
    HANDLE hThread2 = (HANDLE)_beginthreadex(NULL, 0, &TestFunc, NULL, 0, 0);
    HANDLE hThread3 = (HANDLE)_beginthreadex(NULL, 0, &TestFunc, NULL, 0, 0);
    HANDLE hThread4 = (HANDLE)_beginthreadex(NULL, 0, &TestFunc, NULL, 0, 0);
    WaitForSingleObject(hThread1, INFINITE);
    WaitForSingleObject(hThread2, INFINITE);
    WaitForSingleObject(hThread3, INFINITE);
    WaitForSingleObject(hThread4, INFINITE);

    //모든 스레드 종료 됨을 확인하고서 저장
    profile.SaveLogIntoFile();

    CloseHandle(hThread1);
    CloseHandle(hThread2);
    CloseHandle(hThread3);
    CloseHandle(hThread4);
    return 0;
}