#pragma once
#include <Windows.h>
#include <iostream>
#include <psapi.h>
#pragma warning(push)
#pragma warning(disable: 4091)

#include <DbgHelp.h>
#pragma warning(pop)

#pragma comment(lib,"Dbghelp.lib")

/// @brief ũ���� ȣ���ϸ� ���� ���� ����� �Լ�, �ݵ�� �����ѹ� �ϰ� �ؾ��մϴ�.
class CCrashDump
{
public:
	static long _DumpCount;
public:
	CCrashDump()
	{
		printf("dumpCostructor\n");
		_DumpCount = 0;

		_invalid_parameter_handler oldHandler = NULL;
		_invalid_parameter_handler newHandler=NULL;
		
		newHandler = myInvalidParameterHandler;

		oldHandler = _set_invalid_parameter_handler(newHandler);
		_CrtSetReportMode(_CRT_WARN,0);//CRT ���� �޼��� ǥ�� �ߴ�, �ٷ� ������ ������
		_CrtSetReportMode(_CRT_ASSERT, 0);//CRT ���� �޼��� ǥ�� �ߴ�, �ٷ� ������ ������
		_CrtSetReportMode(_CRT_ERROR, 0);//CRT ���� �޼��� ǥ�� �ߴ�, �ٷ� ������ ������

		_CrtSetReportHook(_custom_Report_hook);

		//pure virtual function called ���� �ڵ鷯�� ����� ���� �Լ��� ��ȸ��Ų��.
		_set_purecall_handler(myPureCallHandler);

		SetHandlerDump();
	}
	~CCrashDump()
	{

	}
public:
	static LONG WINAPI MyExceptionFilter(__in PEXCEPTION_POINTERS pExceptionPointer)
	{
		int iWorkingMemory = 0;
		SYSTEMTIME	stNowTime;
		long DumpCount = InterlockedIncrement(&_DumpCount);

		//���� ���μ����� �޸� ��뷮�� ���´�
		HANDLE hProcess = 0;
		PROCESS_MEMORY_COUNTERS pmc;

		hProcess = GetCurrentProcess();
		if (NULL == hProcess)
		{
			return 0;
		}

		if (GetProcessMemoryInfo(hProcess,&pmc,sizeof(pmc)))
		{
			iWorkingMemory = (int)(pmc.WorkingSetSize / 1024 / 1024);

		}
		CloseHandle(hProcess);

		//���� ��¥�� �ð��� �˾ƿ´�.

		WCHAR filename[MAX_PATH];

		GetLocalTime(&stNowTime);
		wsprintf(filename, L"Dump_%d%02d%02d_%02d.%02d.%02d_%d_%dMB.dmp",
			stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour,
			stNowTime.wMinute, stNowTime.wSecond,DumpCount,iWorkingMemory);

		wprintf(L"\n\n\n!!! Crash Error!!! %d.%d.%d / %d:%dL%d \n",
			stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour,
			stNowTime.wMinute, stNowTime.wSecond);
		wprintf(L"Noew SavedumpFile..\n");

		HANDLE hDumpFile = ::CreateFile(filename,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hDumpFile != INVALID_HANDLE_VALUE)
		{
			_MINIDUMP_EXCEPTION_INFORMATION MinidumpExceptionInformation;

			MinidumpExceptionInformation.ThreadId = ::GetCurrentThreadId();
			MinidumpExceptionInformation.ExceptionPointers = pExceptionPointer;
			MinidumpExceptionInformation.ClientPointers = TRUE;

			MiniDumpWriteDump(GetCurrentProcess(),
				GetCurrentProcessId(),
				hDumpFile,
				MiniDumpWithFullMemory,
				&MinidumpExceptionInformation,
				NULL,
				NULL);

			CloseHandle(hDumpFile);

			wprintf(L"CrashDump Save Finish!");
		}
		return EXCEPTION_EXECUTE_HANDLER;

	}
public:
	static void Crash()
	{
		printf("Crash\n");
		int* p = nullptr;
		*p = 0;
	}
	static void SetHandlerDump()
	{
		SetUnhandledExceptionFilter(MyExceptionFilter);
	}

	static int _custom_Report_hook(int ireporttype, char* message, int* returnvalue)
	{
		Crash();
		return true;
	}
	static void myInvalidParameterHandler(const wchar_t * expression, const wchar_t * function,const wchar_t * file, unsigned int line, uintptr_t pReserved)
	{
		Crash();
	}
	static void myPureCallHandler()
	{
		Crash();
	}
};