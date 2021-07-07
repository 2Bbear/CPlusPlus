#include <list>
#include <windows.h>
using namespace std;
#include "PerformanceCounter.h"



PerformanceCounter::PerformanceCounter(const WCHAR* _str)
{
    ProfileBegin(_str);
}

 PerformanceCounter::~PerformanceCounter()
{
}

void  PerformanceCounter::ProfileBegin(const WCHAR* _str)
{
    
    for (list<PROFILE_SAMPLE*>::iterator itr =datalist.begin();itr!=datalist.end();++itr)
    {
        PROFILE_SAMPLE* target = *itr;
        if (target==nullptr)
        {
            break;
        }
        if (wcscmp(target->name, _str) == 0)
        {
            QueryPerformanceCounter(&target->lStartTime);
            ++target->iCall;
            return;
        }
    }
    //데이터 생성
    PROFILE_SAMPLE * data =new PROFILE_SAMPLE();
    wcscpy_s(data->name, 64, _str);
    QueryPerformanceCounter(&data->lStartTime);
    ++(data->iCall);

    datalist.push_back(data);
}

void  PerformanceCounter::ProfileEnd(const WCHAR* _str)
{
    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency); // 딱히 둘곳이 없어서 이쪽에 둠.

    for (list<PROFILE_SAMPLE*>::iterator itr = datalist.begin(); itr != datalist.end(); ++itr)
    {
        PROFILE_SAMPLE* var = *itr;
        if (var == nullptr)
        {
            break;
        }
        if (wcscmp(var->name, _str) == 0)
        {
            LARGE_INTEGER Endtime;
            __int64 elapsed;
            double duringtime;

            QueryPerformanceCounter(&Endtime);

            elapsed = Endtime.QuadPart - var->lStartTime.QuadPart;
            duringtime = elapsed / (double)Frequency.QuadPart; // 이건 어쩔수 없다 치고.

            duringtime *= 1000;  //ms로 변환
            duringtime *= 1000; //us로 변환
            duringtime *= 1000; //ns로 변환. 이쯤 되면 보통 소수점은 없어진다.
            __int64 calTime = (__int64)duringtime;
            //최대,최소 저장

            //최소
            if (var->iMin[0] > calTime)
            {
                var->iMin[1] = var->iMin[0];
                var->iMin[0] = calTime;
            }
            else if (var->iMin[1] > calTime)
            {
                var->iMin[1] = calTime;
            }
            if (var->iMax[0] < calTime)
            {
                var->iMax[1] = var->iMax[0];
                var->iMax[0] = calTime;
            }
            else if (var->iMax[1] < calTime)
            {
                var->iMax[1] = calTime;
            }

            var->iTotalTime += calTime;

        }
    }
}
