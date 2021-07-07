#include <iostream>
#include <Windows.h>

#include "FileManager.h"

FileMangerSpace::EFileResultType FileMangerSpace::MakeFile(const char* _fileFullPath)
{
	//파일이 존재하는지 찾기
	FILE* pFile = NULL;
	fopen_s(&pFile, _fileFullPath, "r");
	if (pFile == NULL)
	{

		fopen_s(&pFile, _fileFullPath, "w");
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

FileMangerSpace::EFileResultType FileMangerSpace::ChangeDataInFile(const char* _fileFullPath, const char* data)
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
		if (fputsResult < 0)
		{
			result = EFileResultType::Fail_WriteData;
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

FileMangerSpace::EFileResultType FileMangerSpace::PushBackDataInFile(const char* _fileFullPath, const char* data)
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
			int fputsResult = fputs(data, pFile);
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

FileMangerSpace::EFileResultType FileMangerSpace::PushBackDataInFile(const char* _fileFullPath, const wchar_t* data)
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

FileMangerSpace::EFileResultType FileMangerSpace::RemoveFile(const char* _fileFullPath)
{
	return EFileResultType::Fail;
}
