#pragma once
namespace Twobbear
{
    /*
4�� �̻� ȣ��Ǵ� ������ ����ؾ� �մϴ�.

����
1. __int64 �� �������� ������ ��� �־�� �ϴ°�
%lld �Ǵ� %164d
*/
#define PROFIL_BEGIN(TagName) ProfileBegin(TagName)
#define PROFIL_END(TagName) ProfileEnd(TagName)
#define PROFIL_SAVELOG SaveLogIntoFile()

#define MaxNameSize 64


/*

��� ����

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
        long lFlag; // ���������� ��� ����. (�迭�ÿ���)
        WCHAR name[MaxNameSize] = {}; // �������� ���� �̸�.

        LARGE_INTEGER lStartTime; // �������� ���� ���� �ð�.

        __int64 iTotalTime = 0; // ��ü ���ð� ī���� Time. (��½� ȣ��ȸ���� ������ ��� ����)
        __int64 iMin[2] = { 999999999999999, }; // �ּ� ���ð� ī���� Time. (�ʴ����� ����Ͽ� ���� / [0] �����ּ� [1] ���� �ּ� [2])
        __int64 iMax[2] = { -1, }; // �ִ� ���ð� ī���� Time. (�ʴ����� ����Ͽ� ���� / [0] �����ִ� [1] ���� �ִ� [2])
        __int64 iCall = 0; // ���� ȣ�� Ƚ��.
    };

    void ProfileBegin(const WCHAR* _str);
    void ProfileEnd(const WCHAR* _str);
    void TestPrintProfileDataList();

    bool INT64ToPointString(__int64 _arg, char _temp[50]);
    bool SaveLogIntoFile();


    //�Լ� ���� Ŭ����
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
