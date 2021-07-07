#pragma once
#ifndef LogManager_h
#define LogManager_h

#include "../../../MySTLProject/MySTLProject/MySTLProject/FileManager.h"
#define _LOGW(errorLevel, wStr, ...) (LogManager::GetInstance().Log(TEXT(__FILE__),TEXT(__FUNCTION__),__LINE__,errorLevel,wStr,##__VA_ARGS__))

enum class ELogLevel : int
{
	Default=0, //�̰� ���� ��Ÿ���� �뵵�Դϴ�. �׻� �տ� �־�� �ؿ�
	Level_DEBUG,
	Level_SYSTEM ,
	Level_NOTICE,
	Level_WARNING,
	Level_ERROR,
	Last // �̰� ������ ��Ÿ���� �뵵�Դϴ� �׻� �ؿ� �վ�� �ؿ�
};
/// @brief �α׸� �����ϰ� ����� �� �� �ֵ��� �����ִ� Ŭ����
class LogManager
{
public:
	CRITICAL_SECTION csLock;
private:
	int iLogLevel;//��� ���� ����� �α� ����
	WCHAR szLogBuff[1024];//�α� ����� �ʿ��� �ӽ� ����
	static const int FILENAMESIZE=256; // �α� ���� �̸� ����
	char fileName[FILENAMESIZE];//�α� ���� �̸�
	time_t logFileNameCurTime;// ���� ���� �̸��� ���� �ð��� ��Ÿ��.
	int logFileIntervalSec=600;// �α����� ���� ���� ��.
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
		//���� �̸� ���� �ð����� ���ϱ�
		logFileNameCurTime = time(NULL);
		struct tm tmLogFileNameCurTime; 
		errno_t error;
		error = localtime_s(&tmLogFileNameCurTime, &logFileNameCurTime);
		if (error!=0)
		{
			//...�������� �� �� �� �ִ°��� ����..
			
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