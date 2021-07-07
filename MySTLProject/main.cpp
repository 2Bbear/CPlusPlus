#include "stdafx.h"
#include "CLockFreeStack.h"
#include "CProfiler.h"

#define MAXALLCOCOUNT 100000
#define ALLOCCOUNT 10000
#define THREADCOUNT 10

CProfiler profiler(true);

struct Human
{
public:
	int age = 0;
	char name[256] = { 0 };
	int attack = 0;
	int defence = 0;
	Human()
	{
		age = 24;
		attack = 55;
		defence = 76;
	}

};

CLockFreeStack<Human>* lfs;

unsigned __stdcall ThreadFunc(LPVOID arg)
{
	srand((unsigned int)time(0));
	int rk = rand() % 100;
	Human* arrHuman[ALLOCCOUNT];
	Sleep(1000);

	profiler.ProfileBegin(L"ThreadFunc");

	//할당
	for (size_t j = 0; j < 100; ++j)
	{
		for (size_t i = 0; i < ALLOCCOUNT; ++i)
		{
			arrHuman[i] = lfs->Alloc(true);
			arrHuman[i]->age = rk;
		}
		//사용
		int check = rk * 2;
		for (size_t i = 0; i < ALLOCCOUNT; ++i)
		{
			if (rk != arrHuman[i]->age)
			{
				CCrashDump::Crash();
			}
			arrHuman[i]->age = arrHuman[i]->age + rk;

			if (check != arrHuman[i]->age)
			{
				CCrashDump::Crash();
			}

		}
		//제거
		for (size_t i = 0; i < ALLOCCOUNT; ++i)
		{
			lfs->Free(&arrHuman[i]);
		}

	}

	profiler.ProfileEnd(L"ThreadFunc");
	return 0;
}
void Test()
{
	HANDLE hTestThreads[THREADCOUNT];
	for (int i = 0; i < THREADCOUNT; ++i)
	{
		hTestThreads[i] = (HANDLE)_beginthreadex(0, 0, ThreadFunc, nullptr, 0, nullptr);
		if (hTestThreads[i] == NULL)
		{
			return;
		}
	}

	for (int i = 0; i < THREADCOUNT; ++i)
	{
		WaitForSingleObjectEx(hTestThreads[i], INFINITE, false);
	}
}

int main()
{
	lfs = new CLockFreeStack<Human>(MAXALLCOCOUNT);
	Test();

	profiler.SaveLogIntoFile();
	return 0;
}
