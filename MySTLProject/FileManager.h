#pragma once
#ifndef FileManager_h
#define FileManager_h

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
	EFileResultType MakeFile(const char* _fileFullPath);
	/// @brief 파일의 데이터를 변경하는 함수
	/// @param _fileFullPath 
	/// @param data 
	/// @return 성공시 Success 반환, 실패시 다양한 에러 열거형을 반환합니다.
	EFileResultType ChangeDataInFile(const char* _fileFullPath, const char* data);
	
	/// @brief 파일 끝에 데이터를 추가하는 함수
	/// @param _fileFullPath 
	/// @param data 
	/// @return 
	EFileResultType PushBackDataInFile(const char* _fileFullPath, const char* data);
	
	EFileResultType PushBackDataInFile(const char* _fileFullPath, const wchar_t* data);
	
	EFileResultType RemoveFile(const char* _fileFullPath);
	

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