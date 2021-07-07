#pragma once
#include <list>
/// @brief 스레드 안전하지 않은 성능측정기입니다. 스레드 안전하게 바꿔야 해요!!
class PerformanceCounter
{
public:
	struct PROFILE_SAMPLE
	{
		long lFlag; // 프로파일의 사용 여부. (배열시에만)
		WCHAR name[64] = {}; // 프로파일 샘플 이름.

		LARGE_INTEGER lStartTime; // 프로파일 샘플 실행 시간.

		__int64 iTotalTime = 0; // 전체 사용시간 카운터 Time. (출력시 호출회수로 나누어 평균 구함)
		__int64 iMin[2] = { 999999999999999, }; // 최소 사용시간 카운터 Time. (초단위로 계산하여 저장 / [0] 가장최소 [1] 다음 최소 [2])
		__int64 iMax[2] = { -1, }; // 최대 사용시간 카운터 Time. (초단위로 계산하여 저장 / [0] 가장최대 [1] 다음 최대 [2])
		__int64 iCall = 0; // 누적 호출 횟수.
	};
private:
	LARGE_INTEGER m_liPerfFreq; //초당 카운트 수
	LARGE_INTEGER m_liPerfStart; //시작 카운트

public:
	std::list<PROFILE_SAMPLE*> datalist;
public:
	PerformanceCounter(const WCHAR* _str);
	~PerformanceCounter();
	void ProfileBegin(const WCHAR* _str);
	void ProfileEnd(const WCHAR* _str);
public:
	/*PerformanceCounter()
	{
		QueryPerformanceFrequency(&m_liPerfFreq);
		Start();
	}
	void Start()
	{
		QueryPerformanceCounter(&m_liPerfStart);
	}*/
	/// @brief Start 함수 호출 이후 경과된 시간을 밀리초 단위로 반환
	/// @return 
	/*__int64 Now() const
	{
		LARGE_INTEGER liPerfNow;
		QueryPerformanceCounter(&liPerfNow);
		return (((liPerfNow.QuadPart - m_liPerfStart.QuadPart) * 1000) / m_liPerfFreq.QuadPart);
	}
	/// @brief 함수 호출 이후 경과된 시간을 마이크로초 단위로 반환
	/// @return 
	__int64 NowInMicro() const
	{
		LARGE_INTEGER liPerfNow;
		QueryPerformanceCounter(&liPerfNow);
		return (((liPerfNow.QuadPart - m_liPerfStart.QuadPart) * 1000000) / m_liPerfFreq.QuadPart);
	}*/
};

