#pragma once
#ifndef LogManager_h
#define LogManager_h

#include "../../../MySTLProject/MySTLProject/MySTLProject/FileManager.h"
#define _LOGW(errorLevel, wStr, ...) (LogManager::GetInstance().Log(TEXT(__FILE__),TEXT(__FUNCTION__),__LINE__,errorLevel,wStr,##__VA_ARGS__))

enum class ELogLevel : int
{
	Default=0, //이건 앞을 나타내는 용도입니다. 항상 앞에 있어야 해요
	Level_DEBUG,
	Level_SYSTEM ,
	Level_NOTICE,
	Level_WARNING,
	Level_ERROR,
	Last // 이건 마지막 나타내는 용도입니다 항상 밑에 잇어야 해요
};
/// @brief 로그를 관리하고 출력을 할 수 있도록 도와주는 클래스
class LogManager
{
public:
	CRITICAL_SECTION csLock;
private:
	int iLogLevel;//출력 저장 대상의 로그 레벨
	WCHAR szLogBuff[1024];//로그 저장시 필요한 임시 버퍼
	static const int FILENAMESIZE=256; // 로그 파일 이름 길이
	char fileName[FILENAMESIZE];//로그 파일 이름
	time_t logFileNameCurTime;// 현재 파일 이름에 들어가는 시간을 나타냄.
	int logFileIntervalSec=600;// 로그파일 생성 간격 초.
private:
	bool bSaveToFile;
	bool bSaveToDB ;
	bool bSendToLogSystem ;
public:
	static LogManager& GetInstance()
	{
		static LogManager instance;
		return instance;
	}
private:
	LogManager()
	{
		iLogLevel = (int)ELogLevel::Level_DEBUG;
		bSaveToFile = false;
		bSaveToDB = false;
		bSendToLogSystem = false;
		//
		InitializeCriticalSection(&csLock);
		//파일 이름 현재 시간으로 정하기
		logFileNameCurTime = time(NULL);
		struct tm tmLogFileNameCurTime; 
		errno_t error;
		error = localtime_s(&tmLogFileNameCurTime, &logFileNameCurTime);
		if (error!=0)
		{
			//...에러나도 뭐 할 수 있는것이 없다..
			
			strcmp(fileName,"fileNameError.txt");
		}
		else
		{
			strftime(fileName,FILENAMESIZE, "%Y-%m-%d-%H-%M-%S.txt", &tmLogFileNameCurTime);
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
public:
	void Log(const WCHAR* _fileName, const WCHAR* _funcName, int _line, ELogLevel _logLevel, const WCHAR* _szString,...);
	void SetLogLevel(ELogLevel _logLevel)
	{
		iLogLevel = (int)_logLevel;
	}
	void SetSaveFile(bool _isSave)
	{
		bSaveToFile = _isSave;
	}
	int GetLogLevel()
	{
		return iLogLevel;
	}
	void SetlogFileIntervalSec(int _sec)
	{
		logFileIntervalSec = _sec;
	}

};

#endif