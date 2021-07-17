#include <iostream>
#include "CThreadProfiler.h"
#include "FileManager.h"
using namespace FileMangerSpace;
using namespace std;
static const  char* filename = "../PerformanceProfileLog.txt"; //�α� ���� �̸�

CThreadProfiler::CThreadProfiler(bool _isCalTotalThread )
{
    tlsIdx=TlsAlloc();
    isCalTotalThread = _isCalTotalThread;

    totaldataList = nullptr;
    totaldataList = new std::list<TOTALPROFILE_SAMPLE>();
}

CThreadProfiler::~CThreadProfiler()
{
    if (totaldataList!=nullptr)
    {
        delete totaldataList;
    }
}

CThreadProfiler::ThreadSample* CThreadProfiler::GetSample()
{
    ThreadSample* p = (ThreadSample*)TlsGetValue(tlsIdx);
    if (p == nullptr)
    {
        p = &g_threadSample[_InterlockedIncrement(&threadSampleIdx)-1];
        p->threadID = GetCurrentThreadId();
        TlsSetValue(tlsIdx, p);
    }
    return p;
}
void CThreadProfiler::ProfileBegin(const WCHAR* _str)
{
    ThreadSample* p = GetSample();
    if (p == nullptr){ return; }
    //������ ����
    for (PROFILE_SAMPLE& var : *(p->datalist))
    {
        if (wcscmp(var.profileName, _str) == 0)
        {
            QueryPerformanceCounter(&var.lStartTime);
            ++var.iCall;
            return;
        }
    }
    PROFILE_SAMPLE data;
    wcscpy_s(data.profileName, 64, _str);
    data.threadID = GetCurrentThreadId();
    QueryPerformanceCounter(&data.lStartTime);
    ++data.iCall;

    p->datalist->push_back(data);
}
void CThreadProfiler::ProfileEnd(const WCHAR* _str)
{
    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency); // ���� �Ѱ��� ��� ���ʿ� ��.
    ThreadSample* p = GetSample();
    if (p == nullptr) { return; }
    for (PROFILE_SAMPLE& var : *(p->datalist))
    {
        if (wcscmp(var.profileName, _str) == 0)
        {
            LARGE_INTEGER Endtime;
            __int64 elapsed;
            double duringtime;

            QueryPerformanceCounter(&Endtime);

            elapsed = Endtime.QuadPart - var.lStartTime.QuadPart;
            duringtime = elapsed / (double)Frequency.QuadPart; // �̰� ��¿�� ���� ġ��.

            duringtime *= 1000;  //ms�� ��ȯ
            duringtime *= 1000; //us�� ��ȯ
            duringtime *= 1000; //ns�� ��ȯ. ���� �Ǹ� ���� �Ҽ����� ��������.
            __int64 calTime = (__int64)duringtime;
            //�ִ�,�ּ� ����

            //�ּ�
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
bool CThreadProfiler::INT64ToPointString(__int64 _arg, char _temp[50])
{
    __int64 num3 = _arg / 1000000;
    _arg -= num3 * 1000000;
    __int64 num9 = _arg / 1;
    sprintf_s(_temp, 50, "%lld.%06lld", num3, num9);
    //sprintf_s(_temp,50,"%s%d",_temp,num9);
    return true;

}

void CThreadProfiler::TestPrintProfileDataList()
{
    ThreadSample* p = GetSample();
    if (p == nullptr) { return; }
    for (PROFILE_SAMPLE& var : *(p->datalist))
    {
        std::wcout << "�̸� : " << var.profileName << std::endl;
        char temp[50] = { 0 };
        INT64ToPointString(var.iTotalTime, temp);
        std::cout << "�ѽð� : " << temp << std::endl;
        char temp2[50] = { 0 };
        INT64ToPointString(var.iMin[0], temp);
        INT64ToPointString(var.iMin[1], temp2);

        std::cout << "�ּ�0 : " << temp << "�ּ�1 : " << temp2 << std::endl;

        INT64ToPointString(var.iMax[0], temp);
        INT64ToPointString(var.iMax[1], temp2);
        std::cout << "�ִ�0 : " << temp << "�ִ�1 : " << temp2 << std::endl;


        INT64ToPointString((var.iTotalTime - (var.iMin[0] + var.iMin[1] + var.iMax[0] + var.iMax[1])) / (var.iCall - 4), temp);

        std::cout << "��� : " << temp << std::endl;

    }
}
bool CThreadProfiler::SaveLogIntoFile()
{
    //threadID | Name  |     Average  |        Min   |        Max   |      Call |
    MakeFile(filename);
    ChangeDataInFile(filename, "threadID|   Name                        | Average              | Min                  | Max                 | Call\n");
    for (int i=0;i< threadSampleIdx;i++)
    {
        PushBackDataInFile(filename, L"=============================================================================================================\n");

        ThreadSample* p = &g_threadSample[i];
        for (PROFILE_SAMPLE& var : *(p->datalist))
        {
            wchar_t wstrArr[400] = { 0 };
            __int64 re = 0;
            if (var.iCall !=0)
            {
                //�ּ� 2��, �ִ� 2���� �����̱� ������ 4���� �Ǿ����� Ȯ���Ѵ�.
                re = (var.iCall > 4) ? ((var.iTotalTime - (var.iMin[0] + var.iMin[1] + var.iMax[0] + var.iMax[1])) / (var.iCall)) : ((var.iTotalTime) / (var.iCall));
                
            }

            double avg = (re > 0) ? (double)(re / 1000000) : (0);
            double avgn = (re > 0) ? (double)(re % 1000000) : (0);
            double min = (double)(var.iMin[0] / 1000000);
            double minn = (double)(var.iMin[0] % 1000000);
            double max = (double)(var.iMax[0] / 1000000);
            double maxn = (double)(var.iMax[0] % 1000000);

            swprintf(wstrArr, 400, L"%8.0i| %30s| %10.0f.%06.0f ms | %10.0f.%06.0f ms |%10.0f.%06.0f ms |%I64d\n"
                , var.threadID
                , var.profileName
                , avg
                , avgn
                , min
                , minn
                , max
                , maxn,
                var.iCall
            ); 
            PushBackDataInFile(filename, wstrArr);
        }
    }
    if (isCalTotalThread==true)
    {
        PushBackDataInFile(filename, L"*********************TOTAL THREAD*****************************************************************************\n");

        //������ ���� ��� ���ϱ�
        IntegrtingInformation();
        for (TOTALPROFILE_SAMPLE& target : *totaldataList)
        {
            
            wchar_t wstrArr[400] = { 0 };
            __int64 re = 0;
            if (target.sample.iCall != 0)
            {
                //������ ������ ��� �ִ�, �ּ� 2������ ���� ���� ���̴�.
                re = (target.sample.iTotalTime) / (target.sample.iCall);

            }
            double avg = (re>0) ? (double)(re / 1000000) :(0) ;
            double avgn = (re > 0)? (double)(re % 1000000) :(0) ;
            double min = (double)(target.sample.iMin[0] / 1000000);
            double minn = (double)(target.sample.iMin[0] % 1000000);
            double max = (double)(target.sample.iMax[0] / 1000000);
            double maxn = (double)(target.sample.iMax[0] % 1000000);

            swprintf(wstrArr, 400, L"%8.0i| %30s| %10.0f.%06.0f ms | %10.0f.%06.0f ms |%10.0f.%06.0f ms |%I64d\n"
                , 0
                , target.sample.profileName
                , avg
                , avgn
                , min
                , minn
                , max
                , maxn,
                target.sample.iCall
            );
            PushBackDataInFile(filename, wstrArr);
        }
    }
   

    return true;
}

void CThreadProfiler::IntegrtingInformation()
{
    //������ ����
    for (int i = 0; i < threadSampleIdx; i++)
    {
        ThreadSample* p = &g_threadSample[i];
        //�Լ� ����
        for (PROFILE_SAMPLE& var : *(p->datalist))
        {
            //���� �̸��� �� �Լ��� �ִ���.
            TOTALPROFILE_SAMPLE* pSample = FindFuncInTotalDataList(var.profileName);
            if(pSample==nullptr)
            {
                //���� �־��ֱ�
                TOTALPROFILE_SAMPLE inputdata;
                inputdata.count = 0;
                inputdata.sample = var;
                totaldataList->push_back(inputdata);
            }
            else
            {
                //���� ���� ���ϱ�
                pSample->sample.iCall += var.iCall;
                pSample->sample.iTotalTime += var.iTotalTime;
                pSample->count++;

                pSample->sample.iMax[0] = (pSample->sample.iMax[0]>var.iMax[0]) ? (pSample->sample.iMax[0]) : (var.iMax[0]);
                pSample->sample.iMin[0] = (pSample->sample.iMin[0] < var.iMin[0]) ? (pSample->sample.iMin[0]) : (var.iMin[0]);
                

            }
        }
    }
}

CThreadProfiler::TOTALPROFILE_SAMPLE* CThreadProfiler::FindFuncInTotalDataList(WCHAR* funcName)
{
    for (TOTALPROFILE_SAMPLE& var : *totaldataList)
    {
        if (wcscmp(var.sample.profileName, funcName) == 0)
        {
            return &var;
        }
    }

    return nullptr;
}

CThreadProfiler::ThreadSample::ThreadSample()
{
    datalist = nullptr;
    datalist = new std::list<PROFILE_SAMPLE>();
}

CThreadProfiler::ThreadSample::~ThreadSample()
{
    if (datalist!=nullptr)
    {
        delete datalist;
    }
}
