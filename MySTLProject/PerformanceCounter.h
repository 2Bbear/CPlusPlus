#pragma once
#include <list>
/// @brief ������ �������� ���� �����������Դϴ�. ������ �����ϰ� �ٲ�� �ؿ�!!
class PerformanceCounter
{
public:
	struct PROFILE_SAMPLE
	{
		long lFlag; // ���������� ��� ����. (�迭�ÿ���)
		WCHAR name[64] = {}; // �������� ���� �̸�.

		LARGE_INTEGER lStartTime; // �������� ���� ���� �ð�.

		__int64 iTotalTime = 0; // ��ü ���ð� ī���� Time. (��½� ȣ��ȸ���� ������ ��� ����)
		__int64 iMin[2] = { 999999999999999, }; // �ּ� ���ð� ī���� Time. (�ʴ����� ����Ͽ� ���� / [0] �����ּ� [1] ���� �ּ� [2])
		__int64 iMax[2] = { -1, }; // �ִ� ���ð� ī���� Time. (�ʴ����� ����Ͽ� ���� / [0] �����ִ� [1] ���� �ִ� [2])
		__int64 iCall = 0; // ���� ȣ�� Ƚ��.
	};
private:
	LARGE_INTEGER m_liPerfFreq; //�ʴ� ī��Ʈ ��
	LARGE_INTEGER m_liPerfStart; //���� ī��Ʈ

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
	/// @brief Start �Լ� ȣ�� ���� ����� �ð��� �и��� ������ ��ȯ
	/// @return 
	/*__int64 Now() const
	{
		LARGE_INTEGER liPerfNow;
		QueryPerformanceCounter(&liPerfNow);
		return (((liPerfNow.QuadPart - m_liPerfStart.QuadPart) * 1000) / m_liPerfFreq.QuadPart);
	}
	/// @brief �Լ� ȣ�� ���� ����� �ð��� ����ũ���� ������ ��ȯ
	/// @return 
	__int64 NowInMicro() const
	{
		LARGE_INTEGER liPerfNow;
		QueryPerformanceCounter(&liPerfNow);
		return (((liPerfNow.QuadPart - m_liPerfStart.QuadPart) * 1000000) / m_liPerfFreq.QuadPart);
	}*/
};

