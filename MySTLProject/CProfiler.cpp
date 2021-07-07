#include "stdafx.h"

#include "FileManager.h"
#include "CProfiler.h"
using namespace FileMangerSpace;

static const  char* filename = "../PerformanceProfileLog.txt"; //로그 파일 이름


CProfiler::CProfiler(bool _isCalTotalThread )
{
    tlsIdx=TlsAlloc();
    isCalTotalThread = _isCalTotalThread;
}

CProfiler::~CProfiler()
{
}

CProfiler::ThreadSample* CProfiler::GetSample()
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
void CProfiler::ProfileBegin(const WCHAR* _str)
{
    ThreadSample* p = GetSample();
    if (p == nullptr){ return; }
    //데이터 생성
    for (PROFILE_SAMPLE& var : p->datalist)
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
    data.threadID = GetCurrentThreadId();
    QueryPerformanceCounter(&data.lStartTime);
    ++data.iCall;

    p->datalist.push_back(data);
}
void CProfiler::ProfileEnd(const WCHAR* _str)
{
    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency); // 딱히 둘곳이 없어서 이쪽에 둠.
    ThreadSample* p = GetSample();
    if (p == nullptr) { return; }
    for (PROFILE_SAMPLE& var : p->datalist)
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
bool CProfiler::INT64ToPointString(__int64 _arg, char _temp[50])
{
    __int64 num3 = _arg / 1000000;
    _arg -= num3 * 1000000;
    __int64 num9 = _arg / 1;
    sprintf_s(_temp, 50, "%lld.%06lld", num3, num9);
    //sprintf_s(_temp,50,"%s%d",_temp,num9);
    return true;

}

void CProfiler::TestPrintProfileDataList()
{
    ThreadSample* p = GetSample();
    if (p == nullptr) { return; }
    for (PROFILE_SAMPLE& var : p->datalist)
    {
        std::wcout << "이름 : " << var.name << std::endl;
        char temp[50] = { 0 };
        INT64ToPointString(var.iTotalTime, temp);
        std::cout << "총시간 : " << temp << std::endl;
        char temp2[50] = { 0 };
        INT64ToPointString(var.iMin[0], temp);
        INT64ToPointString(var.iMin[1], temp2);

        std::cout << "최소0 : " << temp << "최소1 : " << temp2 << std::endl;

        INT64ToPointString(var.iMax[0], temp);
        INT64ToPointString(var.iMax[1], temp2);
        std::cout << "최대0 : " << temp << "최대1 : " << temp2 << std::endl;


        INT64ToPointString((var.iTotalTime - (var.iMin[0] + var.iMin[1] + var.iMax[0] + var.iMax[1])) / (var.iCall - 4), temp);

        std::cout << "평균 : " << temp << std::endl;

    }
}
bool CProfiler::SaveLogIntoFile()
{
    //threadID | Name  |     Average  |        Min   |        Max   |      Call |
    MakeFile(filename);
    ChangeDataInFile(filename, "threadID|   Name                        | Average              | Min                  | Max                 | Call\n");
    for (int i=0;i< threadSampleIdx;i++)
    {
        PushBackDataInFile(filename, L"=============================================================================================================\n");

        ThreadSample* p = &g_threadSample[i];
        for (PROFILE_SAMPLE& var : p->datalist)
        {
            wchar_t wstrArr[400] = { 0 };
            __int64 re = 0;
            if (var.iCall !=0)
            {
                //최소 2개, 최대 2개를 뺄것이기 때문에 4개가 되었는지 확인한다.
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
                , var.name
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

        //스레드 통합 평균 구하기
        IntegrtingInformation();
        for (TOTALPROFILE_SAMPLE& target : totaldataList)
        {
            
            wchar_t wstrArr[400] = { 0 };
            __int64 re = 0;
            if (target.sample.iCall != 0)
            {
                //스레드 통합의 경우 최대, 최소 2개씩을 빼지 않을 것이다.
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
                , target.sample.name
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

void CProfiler::IntegrtingInformation()
{
    //스레드 별로
    for (int i = 0; i < threadSampleIdx; i++)
    {
        ThreadSample* p = &g_threadSample[i];
        //함수 별로
        for (PROFILE_SAMPLE& var : p->datalist)
        {
            //같은 이름을 한 함수가 있는지.
            TOTALPROFILE_SAMPLE* pSample = FindFuncInTotalDataList(var.name);
            if(pSample==nullptr)
            {
                //새로 넣어주기
                TOTALPROFILE_SAMPLE inputdata;
                inputdata.count = 0;
                inputdata.sample = var;
                totaldataList.push_back(inputdata);
            }
            else
            {
                //기존 값에 더하기
                pSample->sample.iCall += var.iCall;
                pSample->sample.iTotalTime += var.iTotalTime;
                pSample->count++;

                pSample->sample.iMax[0] = (pSample->sample.iMax[0]>var.iMax[0]) ? (pSample->sample.iMax[0]) : (var.iMax[0]);
                pSample->sample.iMin[0] = (pSample->sample.iMin[0] < var.iMin[0]) ? (pSample->sample.iMin[0]) : (var.iMin[0]);
                

            }
        }
    }
}

CProfiler::TOTALPROFILE_SAMPLE* CProfiler::FindFuncInTotalDataList(WCHAR* funcName)
{
    for (TOTALPROFILE_SAMPLE& var : totaldataList)
    {
        if (wcscmp(var.sample.name, funcName) == 0)
        {
            return &var;
        }
    }

    return nullptr;
}


