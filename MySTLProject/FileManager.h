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

	/// @brief ���� ��ġ�� ������ ����ϴ�.
	/// @param _fileFullPath 
	/// @return ���ϸ��� �̹� �ִٸ� AlreadyThereFile�� ��ȯ�ϰ� ������ �������� �ʽ��ϴ�.
	///      ������ ���ٸ� ������ ��ȯ�ϰ� �ش� ��ġ�� ������ �����մϴ�.
	inline EFileResultType MakeFile(const char* _fileFullPath)
	{
		//������ �����ϴ��� ã��
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
	/// @brief ������ �����͸� �����ϴ� �Լ�
	/// @param _fileFullPath 
	/// @param data 
	/// @return ������ Success ��ȯ, ���н� �پ��� ���� �������� ��ȯ�մϴ�.
	inline EFileResultType ChangeDataInFile(const char* _fileFullPath, const char* data)
	{
		EFileResultType result = EFileResultType::Fail;

		//������ �����ϴ��� ã��
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
	/// @brief ���� ���� �����͸� �߰��ϴ� �Լ�
	/// @param _fileFullPath 
	/// @param data 
	/// @return 
	inline EFileResultType PushBackDataInFile(const char* _fileFullPath, const char* data)
	{
		EFileResultType result = EFileResultType::Fail;
		//������ �����ϴ��� ã��
		FILE* pFile = NULL;
		fopen_s(&pFile, _fileFullPath, "r+");
		if (pFile != NULL)
		{
			//���� �����͸� ������ �̵�
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
		//������ �����ϴ��� ã��
		FILE* pFile = NULL;
		fopen_s(&pFile, _fileFullPath, "r+");
		if (pFile != NULL)
		{
			//���� �����͸� ������ �̵�
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

	////���ο����� ����ϴ� �͵� ==========================================
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