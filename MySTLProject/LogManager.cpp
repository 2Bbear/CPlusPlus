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
	size_t len = _vscwprintf(_szString, args)+2; //이유를 모르겠는데 뒤에 2글자가 잘림
	WCHAR* buffer = new WCHAR[len];
	vswprintf(buffer, len,_szString,args);
	va_end(args);

	//시간 구하기
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
	//성능이 걱정된다면 위의 문장을 사용할 것.
	//wprintf(L"file:{%s} func:{%s} line:{%d} Description : %s \n", (wcsrchr(_fileName, '\\') ? wcsrchr(_fileName, '\\') + 1 : _fileName), _funcName, _line, buffer);
	//wprintf(L"Description : %s \n", buffer);
	
	//만약 전 로그 시간과 어느 정도 차이 나면 파일을 새로 만들 것인지.========================================================================================
	//지금 시간과 전 시간을 비교하기
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
			//...에러나도 뭐 할 수 있는것이 없다..

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

	//파일로 저장
	if (bSaveToFile==true)
	{
		//시간 붙이기

		//로그 붙이기
		FileMangerSpace::PushBackDataInFile(fileName, outBuffer);
	}
	//DB로 저장
	if (bSaveToDB == true)
	{

	}
#ifdef DO_NETWORK
	//로그 모니터링 시스템에 전송
	if (bSendToLogSystem == true)
	{

	}
#endif // DO_NETWORK

	delete[](buffer);
	delete[](outBuffer);
	LeaveCriticalSection(&csLock);
}
