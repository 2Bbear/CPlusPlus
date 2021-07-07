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

	/// @brief ���� ��ġ�� ������ ����ϴ�.
	/// @param _fileFullPath 
	/// @return ���ϸ��� �̹� �ִٸ� AlreadyThereFile�� ��ȯ�ϰ� ������ �������� �ʽ��ϴ�.
	///      ������ ���ٸ� ������ ��ȯ�ϰ� �ش� ��ġ�� ������ �����մϴ�.
	EFileResultType MakeFile(const char* _fileFullPath);
	/// @brief ������ �����͸� �����ϴ� �Լ�
	/// @param _fileFullPath 
	/// @param data 
	/// @return ������ Success ��ȯ, ���н� �پ��� ���� �������� ��ȯ�մϴ�.
	EFileResultType ChangeDataInFile(const char* _fileFullPath, const char* data);
	
	/// @brief ���� ���� �����͸� �߰��ϴ� �Լ�
	/// @param _fileFullPath 
	/// @param data 
	/// @return 
	EFileResultType PushBackDataInFile(const char* _fileFullPath, const char* data);
	
	EFileResultType PushBackDataInFile(const char* _fileFullPath, const wchar_t* data);
	
	EFileResultType RemoveFile(const char* _fileFullPath);
	

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