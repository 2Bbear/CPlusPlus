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
    //������ ����
    PROFILE_SAMPLE * data =new PROFILE_SAMPLE();
    wcscpy_s(data->name, 64, _str);
    QueryPerformanceCounter(&data->lStartTime);
    ++(data->iCall);

    datalist.push_back(data);
}

void  PerformanceCounter::ProfileEnd(const WCHAR* _str)
{
    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency); // ���� �Ѱ��� ��� ���ʿ� ��.

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
            duringtime = elapsed / (double)Frequency.QuadPart; // �̰� ��¿�� ���� ġ��.

            duringtime *= 1000;  //ms�� ��ȯ
            duringtime *= 1000; //us�� ��ȯ
            duringtime *= 1000; //ns�� ��ȯ. ���� �Ǹ� ���� �Ҽ����� ��������.
            __int64 calTime = (__int64)duringtime;
            //�ִ�,�ּ� ����

            //�ּ�
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
