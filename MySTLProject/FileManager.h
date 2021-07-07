#pragma once
#ifndef FileManager_h
#define FileManager_h


#include <iostream>
#include <Windows.h>


namespace FileMangerSpace
{
	enum class EFileResultType :int
	{
		Default = 0,
		Success,
		Fail,
		AlreadyThereFile,
		ThereIsNoFile,
		Fail_CreateFile,
		Fail_CloseFile,
		Fail_MoveFilePointer,
		Fail_WriteData,
		Fail_WriteDataButCanContinue,
	};

	/// @brief 지정 위치에 파일을 만듭니다.
	/// @param _fileFullPath 
	/// @return 파일명이 이미 있다면 AlreadyThereFile을 반환하고 파일을 생성하지 않습니다.
	///      파일이 없다면 성공을 반환하고 해당 위치에 파일을 생성합니다.
	inline EFileResultType MakeFile(const char* _fileFullPath)
	{
		//파일이 존재하는지 찾기
		FILE* pFile= NULL;
		fopen_s(&pFile,_fileFullPath, "r");
		if (pFile == NULL)
		{

			fopen_s(&pFile,_fileFullPath, "w");
			if (pFile == NULL)
			{
				return EFileResultType::Fail_CreateFile;
			}
			if (fclose(pFile) == EOF)
			{
				return EFileResultType::Fail_CloseFile;
			}
			return EFileResultType::Success;
		}
		else
		{
			if (fclose(pFile) == EOF)
			{
				return EFileResultType::Fail_CloseFile;
			}
			return EFileResultType::AlreadyThereFile;
		}
	}
	/// @brief 파일의 데이터를 변경하는 함수
	/// @param _fileFullPath 
	/// @param data 
	/// @return 성공시 Success 반환, 실패시 다양한 에러 열거형을 반환합니다.
	inline EFileResultType ChangeDataInFile(const char* _fileFullPath, const char* data)
	{
		EFileResultType result = EFileResultType::Fail;

		//파일이 존재하는지 찾기
		FILE* pFile = NULL;
		errno_t err = fopen_s(&pFile, _fileFullPath, "r+");
		if (err == 0)
		{
			//printf("The file 'crt_fopen_s.c' was opened\n");
		}
		else
		{
			//printf("The file 'crt_fopen_s.c' was not opened\n");
		}
		if (pFile != NULL)
		{
			if (fclose(pFile) == EOF)
			{
				return EFileResultType::Fail_CloseFile;
			}

			err = fopen_s(&pFile, _fileFullPath, "w");

			int fputsResult = fprintf(pFile, data);
			//int fputsResult = fputs(data, pFile);
			if (fputsResult <0)
			{
				result= EFileResultType::Fail_WriteData;
			}
			if (fclose(pFile)==EOF)
			{
				return EFileResultType::Fail_CloseFile;
			}
			return EFileResultType::Success;
		}
		else
		{
			return EFileResultType::ThereIsNoFile;
		}
	}
	/// @brief 파일 끝에 데이터를 추가하는 함수
	/// @param _fileFullPath 
	/// @param data 
	/// @return 
	inline EFileResultType PushBackDataInFile(const char* _fileFullPath, const char* data)
	{
		EFileResultType result = EFileResultType::Fail;
		//파일이 존재하는지 찾기
		FILE* pFile = NULL;
		fopen_s(&pFile, _fileFullPath, "r+");
		if (pFile != NULL)
		{
			//파일 포인터를 끝으로 이동
			if (fseek(pFile, 0, SEEK_END)!=0)
			{
				result= EFileResultType::Fail_MoveFilePointer;
			}
			else
			{
				int fputsResult = fputs(data, pFile);
				if (fputsResult < 0)
				{
					if (fputsResult == EOF)
					{
						result= EFileResultType::Fail_WriteDataButCanContinue;
					}
					result= EFileResultType::Fail_WriteData;
				}
			}
			
			if (fclose(pFile) == EOF)
			{
				return EFileResultType::Fail_CloseFile;
			}
			return EFileResultType::Success;
		}
		else
		{
			return EFileResultType::ThereIsNoFile;
		}
	}
	inline EFileResultType PushBackDataInFile(const char* _fileFullPath, const wchar_t* data)
	{
		EFileResultType result = EFileResultType::Fail;
		//파일이 존재하는지 찾기
		FILE* pFile = NULL;
		fopen_s(&pFile, _fileFullPath, "r+");
		if (pFile != NULL)
		{
			//파일 포인터를 끝으로 이동
			if (fseek(pFile, 0, SEEK_END) != 0)
			{
				result = EFileResultType::Fail_MoveFilePointer;
			}
			else
			{
				int fputsResult = fputws(data, pFile);
				if (fputsResult < 0)
				{
					if (fputsResult == EOF)
					{
						result = EFileResultType::Fail_WriteDataButCanContinue;
					}
					result = EFileResultType::Fail_WriteData;
				}
			}

			if (fclose(pFile) == EOF)
			{
				return EFileResultType::Fail_CloseFile;
			}
			return EFileResultType::Success;
		}
		else
		{
			return EFileResultType::ThereIsNoFile;
		}
	}
	inline EFileResultType RemoveFile(const char* _fileFullPath)
	{
		return EFileResultType::Fail;
	}

	////내부에서만 사용하는 것들 ==========================================
	//char* UTF8ToANSI(const char* pszCode)
	//{

	//	BSTR    bstrWide;

	//	char* pszAnsi;

	//	int     nLength;



	//	nLength = MultiByteToWideChar(CP_UTF8, 0, pszCode, strlen(pszCode) + 1, NULL, NULL);

	//	bstrWide = SysAllocStringLen(NULL, nLength);



	//	MultiByteToWideChar(CP_UTF8, 0, pszCode, strlen(pszCode) + 1, bstrWide, nLength);



	//	nLength = WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, NULL, 0, NULL, NULL);

	//	pszAnsi = new char[nLength];



	//	WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, pszAnsi, nLength, NULL, NULL);

	//	SysFreeString(bstrWide);



	//	return pszAnsi;

	//}
	//

}

#endif // !FileManager_h