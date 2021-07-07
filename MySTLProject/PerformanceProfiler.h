#pragma once
namespace Twobbear
{
    /*
4번 이상 호출되는 곳에서 사용해야 합니다.

질문
1. __int64 는 포맷형에 넣을때 어떻게 넣어야 하는가
%lld 또는 %164d
*/
#define PROFIL_BEGIN(TagName) ProfileBegin(TagName)
#define PROFIL_END(TagName) ProfileEnd(TagName)
#define PROFIL_SAVELOG SaveLogIntoFile()

#define MaxNameSize 64


/*

사용 형태

void Test_CustomPerformanceProfiler()
{
    PRO_BEGIN(L"Test1");
    Sleep(1000);

    PRO_END(L"Test1");

    PRO_SAVELOG;
}

*/
    struct PROFILE_SAMPLE
    {
        long lFlag; // 프로파일의 사용 여부. (배열시에만)
        WCHAR name[MaxNameSize] = {}; // 프로파일 샘플 이름.

        LARGE_INTEGER lStartTime; // 프로파일 샘플 실행 시간.

        __int64 iTotalTime = 0; // 전체 사용시간 카운터 Time. (출력시 호출회수로 나누어 평균 구함)
        __int64 iMin[2] = { 999999999999999, }; // 최소 사용시간 카운터 Time. (초단위로 계산하여 저장 / [0] 가장최소 [1] 다음 최소 [2])
        __int64 iMax[2] = { -1, }; // 최대 사용시간 카운터 Time. (초단위로 계산하여 저장 / [0] 가장최대 [1] 다음 최대 [2])
        __int64 iCall = 0; // 누적 호출 횟수.
    };

    void ProfileBegin(const WCHAR* _str);
    void ProfileEnd(const WCHAR* _str);
    void TestPrintProfileDataList();

    bool INT64ToPointString(__int64 _arg, char _temp[50]);
    bool SaveLogIntoFile();


    //함수 감싼 클래스
    class SafetyPerformanceProfiler
    {
    public:
        SafetyPerformanceProfiler()
        {
            //
        }
        ~SafetyPerformanceProfiler()
        {

        }

    public:

    };

    class PerformanceProfiler
    {
    public:
        const WCHAR* str;

    public:
        PerformanceProfiler(const WCHAR* _str);
        ~PerformanceProfiler();
    public:
        void DoQueryPerformanceFrequencyTest();
        void TestGetCPUClock();

    };


}
