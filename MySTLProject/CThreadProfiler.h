#pragma once

#pragma region 테스트코드
/*
테스트 코드
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
*/
#pragma endregion

#define CProfiler_MaxNameSize 64


class CThreadProfiler
{
public:
    struct PROFILE_SAMPLE
    {
        long lFlag; // 프로파일의 사용 여부. (배열시에만)
        WCHAR name[CProfiler_MaxNameSize] = {}; // 프로파일 샘플 이름.
        int threadID;
        LARGE_INTEGER lStartTime; // 프로파일 샘플 실행 시간.

        __int64 iTotalTime = 0; // 전체 사용시간 카운터 Time. (출력시 호출회수로 나누어 평균 구함)
        /*
        2 배열인 이유는 최소 걸린시간 2개, 최대 걸린시간 2개를 빼서 측정하려고 함, 이 부분은 환경적인 문제가 있을 수 있기 때문이다.
        */
        __int64 iMin[2] = { 999999999999999, }; // 최소 사용시간 카운터 Time. (초단위로 계산하여 저장 / [0] 가장최소 [1] 다음 최소 [2])
        __int64 iMax[2] = { -1, }; // 최대 사용시간 카운터 Time. (초단위로 계산하여 저장 / [0] 가장최대 [1] 다음 최대 [2])
        __int64 iCall = 0; // 누적 호출 횟수.
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
    ThreadSample g_threadSample[1000];//스레드 갯수에 맞춰야함. 혹은 실제 사용 스레드 수 보다 많아야 함.
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

// 전체 스레드 평균을 내기 위한 코드
private:
    bool isCalTotalThread = false;
    std::list<TOTALPROFILE_SAMPLE>* totaldataList;
    //프로파일 데이터 통합하고 정리하는 함수
    void IntegrtingInformation();
    TOTALPROFILE_SAMPLE* FindFuncInTotalDataList(WCHAR * funcName);

};

