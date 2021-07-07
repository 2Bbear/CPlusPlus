#include "stdafx.h"
#include "LogManager.h"


void LogManager::Log(const WCHAR* _fileName, const WCHAR* _funcName, int _line, ELogLevel _logLevel, const WCHAR* _szString,  ...)
{
	EnterCriticalSection(&csLock);
	if (iLogLevel>(int)_logLevel)
	{
		LeaveCriticalSection(&csLock);
		return;
	}
	
	va_list args;
	va_start(args, _szString);
	size_t len = _vscwprintf(_szString, args)+2; //������ �𸣰ڴµ� �ڿ� 2���ڰ� �߸�
	WCHAR* buffer = new WCHAR[len];
	vswprintf(buffer, len,_szString,args);
	va_end(args);

	//�ð� ���ϱ�
	wchar_t* outBuffer = new wchar_t[len+82];
	memset(outBuffer, 0, sizeof(wchar_t) * (len + 82));
	time_t curTime = time(NULL);
	struct tm tmCurTime;
	errno_t error;
	error = localtime_s(&tmCurTime, &curTime);
	if (error == 0)
	{
		std::wcsftime(outBuffer, 82, L"(%Y-%m-%d-%H-%M-%S) : ", &tmCurTime);
	}
	lstrcatW(outBuffer, buffer);
	//

	//wprintf(L"file:{%s} func:{%s} line:{%d} Description : %s",_fileName , _funcName, _line, buffer);
	//������ �����ȴٸ� ���� ������ ����� ��.
	//wprintf(L"file:{%s} func:{%s} line:{%d} Description : %s \n", (wcsrchr(_fileName, '\\') ? wcsrchr(_fileName, '\\') + 1 : _fileName), _funcName, _line, buffer);
	//wprintf(L"Description : %s \n", buffer);
	
	//���� �� �α� �ð��� ��� ���� ���� ���� ������ ���� ���� ������.========================================================================================
	//���� �ð��� �� �ð��� ���ϱ�
	double diffSecs = 0;
	diffSecs = difftime(curTime, logFileNameCurTime);
	if (logFileIntervalSec < diffSecs)
	{
		logFileNameCurTime = curTime;
		struct tm tmLogFileNameCurTime;
		errno_t error;
		error = localtime_s(&tmLogFileNameCurTime, &logFileNameCurTime);
		if (error != 0)
		{
			//...�������� �� �� �� �ִ°��� ����..

			strcmp(fileName, "fileNameError.txt");
		}
		else
		{
			std::strftime(fileName, FILENAMESIZE, "%Y-%m-%d-%H-%M-%S.txt", &tmLogFileNameCurTime);
		}

		FileMangerSpace::EFileResultType result = FileMangerSpace::MakeFile(fileName);
		if (result == FileMangerSpace::EFileResultType::AlreadyThereFile || result == FileMangerSpace::EFileResultType::Success)
		{

		}
		else
		{
			printf("Create File fail. what sould i do? ...OTL \n");
		}
	}
	//=============================================================================================================================

	//���Ϸ� ����
	if (bSaveToFile==true)
	{
		//�ð� ���̱�

		//�α� ���̱�
		FileMangerSpace::PushBackDataInFile(fileName, outBuffer);
	}
	//DB�� ����
	if (bSaveToDB == true)
	{

	}
#ifdef DO_NETWORK
	//�α� ����͸� �ý��ۿ� ����
	if (bSendToLogSystem == true)
	{

	}
#endif // DO_NETWORK

	delete[](buffer);
	delete[](outBuffer);
	LeaveCriticalSection(&csLock);
}
