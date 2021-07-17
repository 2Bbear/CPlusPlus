#pragma once

#pragma region �׽�Ʈ�ڵ�
/*
�׽�Ʈ �ڵ�
CThreadProfiler profile(true);

void Test1()
{
    profile.ProfileBegin(L"Test1");

    profile.ProfileEnd(L"Test1");
}

void Test2()
{
    profile.ProfileBegin(L"Test2");
    for (int i = 0;i<10; i++)
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
    //������ �����ϱ�
    HANDLE hThread1 = (HANDLE)_beginthreadex(NULL, 0, &TestFunc, NULL, 0, 0);
    HANDLE hThread2 = (HANDLE)_beginthreadex(NULL, 0, &TestFunc, NULL, 0, 0);
    HANDLE hThread3 = (HANDLE)_beginthreadex(NULL, 0, &TestFunc, NULL, 0, 0);
    HANDLE hThread4 = (HANDLE)_beginthreadex(NULL, 0, &TestFunc, NULL, 0, 0);
    WaitForSingleObject(hThread1, INFINITE);
    WaitForSingleObject(hThread2, INFINITE);
    WaitForSingleObject(hThread3, INFINITE);
    WaitForSingleObject(hThread4, INFINITE);

    //��� ������ ���� ���� Ȯ���ϰ� ����
    profile.SaveLogIntoFile();

    CloseHandle(hThread1);
    CloseHandle(hThread2);
    CloseHandle(hThread3);
    CloseHandle(hThread4);
    return 0;
}
*/
#pragma endregion

#include <Windows.h>
#include <list>

#define CProfiler_MaxNameSize 64


class CThreadProfiler
{
private:
    struct PROFILE_SAMPLE
    {
        long lFlag; // ���������� ��� ����. (�迭�ÿ���)
        WCHAR profileName[CProfiler_MaxNameSize] = {}; // �������� ���� �̸�.
        int threadID;
        LARGE_INTEGER lStartTime; // �������� ���� ���� �ð�.

        __int64 iTotalTime = 0; // ��ü ���ð� ī���� Time. (��½� ȣ��ȸ���� ������ ��� ����)
        /*
        2 �迭�� ������ �ּ� �ɸ��ð� 2��, �ִ� �ɸ��ð� 2���� ���� �����Ϸ��� ��, �� �κ��� ȯ������ ������ ���� �� �ֱ� �����̴�.
        */
        __int64 iMin[2] = { 999999999999999, }; // �ּ� ���ð� ī���� Time. (�ʴ����� ����Ͽ� ���� / [0] �����ּ� [1] ���� �ּ� [2])
        __int64 iMax[2] = { -1, }; // �ִ� ���ð� ī���� Time. (�ʴ����� ����Ͽ� ���� / [0] �����ִ� [1] ���� �ִ� [2])
        __int64 iCall = 0; // ���� ȣ�� Ƚ��.
    };
    struct ThreadSample
    {
        std::list<PROFILE_SAMPLE>* datalist;
        int threadID;
        ThreadSample();
        ~ThreadSample();
    };
    struct TOTALPROFILE_SAMPLE
    {
        PROFILE_SAMPLE sample;
        int count = 0;
    };
public:
    CThreadProfiler(bool _isCalTotalThread=false);
    ~CThreadProfiler();
public:
    ThreadSample g_threadSample[1000];//������ ������ �������. Ȥ�� ���� ��� ������ �� ���� ���ƾ� ��.
private:
    DWORD tlsIdx;
    long threadSampleIdx = 0;
private:
    ThreadSample* GetSample();
public:
    void ProfileBegin(const WCHAR* _str);
    void ProfileEnd(const WCHAR* _str);
    void TestPrintProfileDataList();

    bool INT64ToPointString(__int64 _arg, char _temp[50]);
    bool SaveLogIntoFile();

// ��ü ������ ����� ���� ���� �ڵ�
private:
    bool isCalTotalThread = false;
    std::list<TOTALPROFILE_SAMPLE>* totaldataList;
    //�������� ������ �����ϰ� �����ϴ� �Լ�
    void IntegrtingInformation();
    TOTALPROFILE_SAMPLE* FindFuncInTotalDataList(WCHAR * funcName);

};
