
#include<Windows.h>
#include<iostream>
#include<list>
#include <wchar.h>

using namespace std;

#pragma comment(lib, "winmm.lib")


#include "PerformanceProfiler.h"
#include "FileManager.h"
using namespace Twobbear;
static const  char *filename = "../PerformanceProfileLog.txt"; //로그 파일 이름
std::list<PROFILE_SAMPLE> datalist;

#define SaveUnderPoint4 1000


void Twobbear::ProfileBegin(const WCHAR* _str)
{
    //데이터 생성
    for (PROFILE_SAMPLE& var : datalist)
    {
        if (wcscmp(var.name, _str) == 0)
        {
            QueryPerformanceCounter(&var.lStartTime);
            ++var.iCall;
            return;
        }
    }
    PROFILE_SAMPLE data;
    wcscpy_s(data.name, 64, _str);
    QueryPerformanceCounter(&data.lStartTime);
    ++data.iCall;

    datalist.push_back(data);


}
void Twobbear::ProfileEnd(const WCHAR* _str)
{
    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency); // 딱히 둘곳이 없어서 이쪽에 둠.

    for (PROFILE_SAMPLE& var : datalist)
    {
        if (wcscmp(var.name, _str) == 0)
        {
            LARGE_INTEGER Endtime;
            __int64 elapsed;
            double duringtime;

            QueryPerformanceCounter(&Endtime);

            elapsed = Endtime.QuadPart - var.lStartTime.QuadPart;
            duringtime = elapsed / (double)Frequency.QuadPart; // 이건 어쩔수 없다 치고.

            duringtime *= 1000;  //ms로 변환
            duringtime *= 1000; //us로 변환
            duringtime *= 1000; //ns로 변환. 이쯤 되면 보통 소수점은 없어진다.
            __int64 calTime = (__int64)duringtime;
            //최대,최소 저장

            //최소
            if (var.iMin[0] > calTime)
            {
                var.iMin[1] = var.iMin[0];
                var.iMin[0] = calTime;
            }
            else if (var.iMin[1] > calTime)
            {
                var.iMin[1] = calTime;
            }
            if (var.iMax[0] < calTime)
            {
                var.iMax[1] = var.iMax[0];
                var.iMax[0] = calTime;
            }
            else if (var.iMax[1] < calTime)
            {
                var.iMax[1] = calTime;
            }

            var.iTotalTime += calTime;

        }
    }
}
bool Twobbear::INT64ToPointString(__int64 _arg, char _temp[50])
{
    __int64 num3 = _arg / 1000000;
    _arg -= num3 * 1000000;
    __int64 num9 = _arg / 1;
    sprintf_s(_temp, 50, "%lld.%06lld", num3, num9);
    //sprintf_s(_temp,50,"%s%d",_temp,num9);
    return true;

}
void Twobbear::TestPrintProfileDataList()
{
    for (PROFILE_SAMPLE& var : datalist)
    {
        std::wcout << "이름 : " << var.name << std::endl;
        char temp[50] = { 0 };
        Twobbear::INT64ToPointString(var.iTotalTime, temp);
        std::cout << "총시간 : " << temp << std::endl;
        char temp2[50] = { 0 };
        Twobbear::INT64ToPointString(var.iMin[0], temp);
        Twobbear::INT64ToPointString(var.iMin[1], temp2);

        std::cout << "최소0 : " << temp << "최소1 : " << temp2 << std::endl;

        Twobbear::INT64ToPointString(var.iMax[0], temp);
        Twobbear::INT64ToPointString(var.iMax[1], temp2);
        std::cout << "최대0 : " << temp << "최대1 : " << temp2 << std::endl;


        Twobbear::INT64ToPointString((var.iTotalTime - (var.iMin[0] + var.iMin[1] + var.iMax[0] + var.iMax[1])) / (var.iCall - 4), temp);

        std::cout << "평균 : " << temp << std::endl;

    }
}
bool Twobbear::SaveLogIntoFile()
{
    //Name  |     Average  |        Min   |        Max   |      Call |

    FileMangerSpace::MakeFile(filename);
    FileMangerSpace::PushBackDataInFile(filename,"Name                          |\tAverage\t\t\t\t|\tMin\t\t\t\t\t|\tMax\t\t\t\t\t|\tCall\t|\n");

    for (PROFILE_SAMPLE& var : datalist)
    {
        wchar_t wstrArr[256] = {0};
        __int64 re = (var.iTotalTime - (var.iMin[0] + var.iMin[1] + var.iMax[0] + var.iMax[1])) / (var.iCall - 4);

        double avg = (double)(re / 1000000);
        double avgn = (double)(re % 1000000);
        double min = (double)(var.iMin[0] / 1000000);
        double minn = (double)(var.iMin[0] % 1000000);
        double max = (double)(var.iMax[0] / 1000000);
        double maxn = (double)(var.iMax[0] % 1000000);

        swprintf(wstrArr, 256,L"%30s|\t%.0f.%06.0f ms\t|\t%.0f.%06.0f ms\t\t|\t%.0f.%06.0f ms\t\t|\t%I64d\n",var.name,
            avg,avgn,
            min,minn,
            max,maxn,
            var.iCall);

        FileMangerSpace::PushBackDataInFile(filename, wstrArr);
    }

    return false;
}



PerformanceProfiler::PerformanceProfiler(const WCHAR* _str)
{
    Twobbear::ProfileBegin(_str);
    str = _str;
}
PerformanceProfiler::~PerformanceProfiler()
{
    Twobbear::ProfileEnd(str);
    //timeEndPeriod(1);
}
void PerformanceProfiler::DoQueryPerformanceFrequencyTest()
{
    LARGE_INTEGER Frequency;
    LARGE_INTEGER BeginTime;
    LARGE_INTEGER Endtime;
    __int64 elapsed;
    double duringtime;

    //프로그램이나 클래스 시작부분에
    QueryPerformanceFrequency(&Frequency);

    //사용하고자 하는 부분에 다음 코딩
    QueryPerformanceCounter(&BeginTime);

    //////////////////////////////
    //프로그램 코드
    Sleep(1000);
    //////////////////////////////

    QueryPerformanceCounter(&Endtime);
    elapsed = Endtime.QuadPart - BeginTime.QuadPart;
    duringtime = (double)elapsed / (double)Frequency.QuadPart;

    duringtime *= 1000;  //ms로 변환
    std::cout << duringtime << std::endl;
}
void PerformanceProfiler::TestGetCPUClock()
{
    while (true)
    {
        LARGE_INTEGER Endtime;
        //프로그램이나 클래스 시작부분에
        QueryPerformanceCounter(&Endtime);

        std::cout << Endtime.QuadPart / 10000 << std::endl;

        Sleep(100);
    }

}