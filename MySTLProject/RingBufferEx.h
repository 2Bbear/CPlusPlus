#pragma once
using namespace std;

#include "stdafx.h"

#define ERRTHROW(_errorType,msg) (RingBufferException(_errorType,msg,__FILE__,__LINE__))
/// @brief 링버퍼 클래스에 대한 예외처리를 정의합니다.
class RingBufferException : public exception
{
public:
	enum class EExceptionType :int
	{
		Default = 0,
		Error_EmptyBuffer,//버퍼가 비어있음
		Error_Overflow,//버퍼가 가득 차 있음
		Error_TargetSizeDifferent,//대상과의 크기가 맞지 않음.
		Error_ReadIdxOverMaxIdx, //readIdx가 배열 최대 인덱스를 넘어슴
		Error_WriteIdxOverMaxIdx, // writeIdx가 배열 최대 인덱스를 넘어섰음.
		Error_WrongInd,//잘못된 인덱스 위치
	};
public:
	RingBufferException(EExceptionType _type, const char* _errorMsg, const char* _fileName, int _line)noexcept
	{
		errorType = _type;
		fileName = _fileName;
		line = _line;
		errorMsg = _errorMsg;
	}
public:
	EExceptionType errorType;
	const char* errorMsg = nullptr;
	const char* fileName = nullptr;
	int line;
};
/// @brief Enqueu를 하면서 절대 write Idx와 readIdx가 같아질 수 없다.
class RingBufferEx
{
public:
	int max; // 버퍼의 최대 용량
	unsigned long long readIdx;//읽기 시작할 위치
	unsigned long long writeIdx;//쓰기 시작할 위치
	int curDataSize; //현재 데이터 크기
public:
	char* buffer; // 버퍼 본체
public:
	RingBufferEx(int _capacity)
	{
		curDataSize = 0;
		readIdx = writeIdx = 0;
		max = _capacity;
		buffer = (char*)malloc(max);
		if (buffer != nullptr)
		{
			memset(buffer, 0, max);
		}
	}
	~RingBufferEx()
	{
		free(buffer);
	}
public:
	/// @brief 버퍼 크기 초기화 , 기존의 데이터는 모두 삭제됩니다.
	/// @param _reSize 
	/// @return 성공여부
	bool InitBufferSize(int _reSize)
	{
		//데이터 초기화
		curDataSize = 0;
		readIdx = writeIdx = 0;
		free(buffer);

		// 재설정
		max = _reSize;
		buffer = (char*)malloc(max);

		return true;
	}
	/// @brief 버퍼에 데이터 넣기
	/// @param _inData 넣을 데이터 시작 포인터
	/// @param _inSize 넣을 데이터 크기
	/// @return 넣어진 데이터 총 크기, 0일 경우 넣기 실패
	int Enque(char * _inDataPtr, int _inSize)// throw(RingBufferException)
	{
		int iWriteIDx = (int)writeIdx;
		int iReadIdx = (int)readIdx;
		int result = 0;

		int dirE = GetDirectEnqueSize(iReadIdx, iWriteIDx);
		if (dirE < _inSize)
		{
			memcpy(buffer+ iWriteIDx, _inDataPtr , dirE);
			MoveWriteIdx(&iWriteIDx, dirE);
			result += dirE;

			int remain = _inSize - dirE;
			memcpy(buffer + iWriteIDx, _inDataPtr+ dirE, remain);
			MoveWriteIdx(&iWriteIDx, remain);
			result += remain;
				
		}
		else
		{
			memcpy(buffer + iWriteIDx, _inDataPtr, _inSize);
			MoveWriteIdx(&iWriteIDx, _inSize);
			result += _inSize;
		}
		_InterlockedExchange(&writeIdx, iWriteIDx);
		return result;
	}
	/// @brief 버퍼에서 데이터 빼기
	/// @param _destPtr 빼서 넣을 목적지 포인터
	/// @param _outSize 빼올 데이터 크기
	/// @return 빼내여진 데이터 총 크기 , 0일 경우 빼기 실패
	int Deque(char * _destPtr, int _outSize)// throw(RingBufferException)
	{
		int iWriteIDx = (int)writeIdx;
		int iReadIdx = (int)readIdx;

		int dirD = GetDirectDequeSize(iReadIdx, iWriteIDx);
		int insertedSize = _outSize;

		int result = 0;

		if (dirD < _outSize)
		{
			int arridx = 0;

			memcpy(_destPtr, buffer+ iReadIdx, dirD);
			MoveReadIdx(&iReadIdx, dirD);
			arridx += dirD;

			result += dirD;

			int remain = _outSize - dirD;
			memcpy(_destPtr + arridx, buffer + iReadIdx, remain);
			MoveReadIdx(&iReadIdx, remain);

			result += remain;

		}
		else
		{
			memcpy(_destPtr, buffer + iReadIdx, _outSize);
			MoveReadIdx(&iReadIdx, _outSize);

			result += _outSize;
		}
		

		_InterlockedExchange(&readIdx, iReadIdx);
#ifdef _TEST
		if (readIdx<0)
		{
			throw ERRTHROW(RingBufferException::EExceptionType::Error_WrongInd,
				"readIdx값이 0보다 작습니다.");
		}
#endif // _TEST

		return result;
	}

	//큐의 read 포인터
	char* GetReadBufferPtr()
	{
		return buffer + readIdx;
	}
	//큐의 write 포인터
	char* GetWritebufferPtr()
	{
		return buffer + writeIdx;
	}
	/// @brief readIdx 움직이는 함수, 한번에 움직일 수 있는 값 이상을 움직이면 error 던진다.
	/// @param _readIdx 
	/// @param _moveSize 
	/// @return 
	bool MoveReadIdx(int * _readIdx,int _moveSize) //throw(RingBufferException)
	{
		int temp =(*_readIdx) + _moveSize;
		if (temp ==max) //최대 인덱스에 도달했기 때문에 0으로 옮겨야함
		{
			(*_readIdx) = 0;
			return true;

		}
		else if (temp< max)
		{
			(*_readIdx) = temp;
#ifdef _TEST
			if (readIdx < 0)
			{
				throw ERRTHROW(RingBufferException::EExceptionType::Error_WrongInd,
					"readIdx값이 0보다 작습니다.");
			}
#endif // _TEST
			return true;
		}
		else if (temp >max)
		{
			throw ERRTHROW(RingBufferException::EExceptionType::Error_ReadIdxOverMaxIdx,
				"readIdx가 maxIdx를 초과했습니다.");
		}
		return false;
	}
	/// @brief writeIdx를 움직이는 함수 한번에 움직일 수 있는 값 이상을 움직이면 error를 던진다.
	/// @param _writeIdx 
	/// @param _moveSize 
	/// @return 
	bool MoveWriteIdx(int * _writeIdx, int _moveSize)//throw(RingBufferException)
	{
		int temp = (*_writeIdx) + _moveSize;
		if (temp==max)
		{
			(*_writeIdx) = 0;
			return true;
		}
		else if (temp < max)
		{
			(*_writeIdx) = temp;
			return true;
		}
		else if (temp > max)
		{
			throw ERRTHROW(RingBufferException::EExceptionType::Error_WriteIdxOverMaxIdx,
				"writeIdx가 maxIdx를 초과했습니다.");
		}
		return false;
	}

	/// @brief readIdx 움직이는 함수, 한번에 경계를 넘어서 움직인다. idx가 같아질 수 있다.
	/// @param _readIdx 
	/// @param _moveSize 
	/// @return 
	bool MoveReadIdxEx(int* _readIdx, int _moveSize)
	{
		int local_readIdx = (int)readIdx;
		int local_writeIdx = (int)writeIdx;
		int direcDequeSize = GetDirectDequeSize(local_readIdx, local_writeIdx);
		if (direcDequeSize >= _moveSize)
		{
			MoveReadIdx(&local_readIdx, _moveSize);
			_InterlockedExchange(&readIdx, local_readIdx);
			return true;
		}
		else
		{
			MoveReadIdx(&local_readIdx, direcDequeSize);
			int remainCopySize = _moveSize - direcDequeSize;
			MoveReadIdx(&local_readIdx, remainCopySize);

			_InterlockedExchange(&readIdx, local_readIdx);
			return true;
		}
	}
	bool MoveReadIdxEx(unsigned long long* _readIdx, int _moveSize)
	{
		int local_readIdx = (int)readIdx;
		int local_writeIdx = (int)writeIdx;
		int direcDequeSize = GetDirectDequeSize(local_readIdx, local_writeIdx);
		if (direcDequeSize >= _moveSize)
		{
			MoveReadIdx(&local_readIdx, _moveSize);
			_InterlockedExchange(&readIdx, local_readIdx);
			return true;
		}
		else
		{
			MoveReadIdx(&local_readIdx, direcDequeSize);
			int remainCopySize = _moveSize - direcDequeSize;
			MoveReadIdx(&local_readIdx, remainCopySize);

			_InterlockedExchange(&readIdx, local_readIdx);
			return true;
		}
	}
	/// @brief writeIdx 움직이는 함수, 한번에 경계를 넘어서 움직인다. idx가 같아질거 같다면 false를 반환한다.
	/// @param _writeIdx 
	/// @param _moveSize 
	/// @return 
	bool MoveWriteIdxEx(int* _writeIdx, int _moveSize)
	{
		int local_readIdx = (int)readIdx;
		int local_writeIdx = (int)writeIdx;

		int direcEnqRecvSize = GetDirectEnqueSize(local_readIdx, local_writeIdx);
		if (direcEnqRecvSize >= _moveSize)
		{
			MoveWriteIdx(&local_writeIdx, _moveSize);
			_InterlockedExchange(&writeIdx, local_writeIdx);
			return true;
		}
		else
		{
			MoveWriteIdx(&local_writeIdx, direcEnqRecvSize);
			int remainMoveSize = _moveSize - direcEnqRecvSize;
			MoveWriteIdx(&local_writeIdx, remainMoveSize);
			_InterlockedExchange(&writeIdx, local_writeIdx);
			return true;
		}
	}
	bool MoveWriteIdxEx(unsigned long long* _writeIdx, int _moveSize)
	{
		int local_readIdx = (int)readIdx;
		int local_writeIdx = (int)(*_writeIdx);

		int direcEnqRecvSize = GetDirectEnqueSize(local_readIdx, local_writeIdx);
		if (direcEnqRecvSize >= _moveSize)
		{
			MoveWriteIdx(&local_writeIdx, _moveSize);
			_InterlockedExchange(&writeIdx, local_writeIdx);
			return true;
		}
		else
		{
			MoveWriteIdx(&local_writeIdx, direcEnqRecvSize);
			int remainMoveSize = _moveSize - direcEnqRecvSize;
			MoveWriteIdx(&local_writeIdx, remainMoveSize);
			_InterlockedExchange(&writeIdx, local_writeIdx);
			return true;
		}
	}
	//버퍼에서 데이터를 피크
	int Peek(unsigned char* chpDest, int iSize) //throw(RingBufferException)
	{
		int iReadIdx = (int)readIdx;
		int iWriteIDx = (int)writeIdx;
		if (GetUseSize() < iSize)
		{
			throw ERRTHROW(RingBufferException::EExceptionType::Error_TargetSizeDifferent,"원하는 데이터 크기 만큼 버퍼안에 데이터가 없습니다.");
			return 0;
		}

		//한번에 뺄 수 있는 크기 구하기
		int directDequeSize = GetDirectDequeSize(iReadIdx, iWriteIDx);
		if (directDequeSize>= iSize)
		{
			memcpy(chpDest, buffer + iReadIdx, iSize);
			return iSize;
		}
		else //2번에 걸쳐 빼야할 경우MoveReadIdxEx
		{
			memcpy(chpDest, buffer + iReadIdx, directDequeSize); 
			MoveReadIdx(&iReadIdx, directDequeSize);

			int remainDequeSize = iSize - directDequeSize;

			memcpy(chpDest+ directDequeSize, buffer + iReadIdx, remainDequeSize);

			return directDequeSize+ remainDequeSize;
		}
		return 0;
	}
	//한번에 뺄 수 있는 용량 구하기
	int GetDirectDequeSize(int _readIdx, int _writeIdx)
	{
		int result = 0;

		if (_readIdx < _writeIdx)
		{
			result = _writeIdx - _readIdx;
		}
		else if(_readIdx > _writeIdx)
		{
			//read에서 끝까지.
			result = max - _readIdx;
		}

		return result;
	}
	//한번에 넣을 수 있는 용량 구하기
	//넣는 것은 1바이트 부족하게 들어감
	int GetDirectEnqueSize(int _readIdx, int _writeIdx)
	{
		int result = 0;
		if (_readIdx ==0)
		{
			result = (max - 1) - _writeIdx;
		}
		else
		{
			if (_readIdx < _writeIdx)
			{
				result =  max- _writeIdx;
			}
			else if (_readIdx > _writeIdx)
			{
				//read에서 끝까지.
				result = (_readIdx -1) - _writeIdx;
			}
			else if (_readIdx== _writeIdx)
			{
				result = max- _writeIdx;
			}
		}
		return result;
	}
	//버퍼의 현재 총 용량 반환
	int GetMaxSize()
	{
		return max;
	}
	//버퍼에 쌓여있는 데이터 용량 반환
	int GetUseSize(void)
	{
		int local_writeIdx = (int)writeIdx;
		int local_readIdx = (int)readIdx;

		if (local_writeIdx > local_readIdx)
		{
			return local_writeIdx - local_readIdx;
		}
		else if (local_writeIdx < local_readIdx) //만약 데이터가 경계선상에 있어도 ...저장된 데이터 크기를 줌
		{
			int a = max - local_readIdx; //배열 끝까지의 데이터 사이즈
			int b = local_writeIdx; // 배열 시작부 부터 writeIdx 전까지의 데이터 사이즈
			return a + b;
		}
		else
		{
			return 0;
		}
	}
	//버퍼에 Enque할 수 있는 비어 있는 데이터 용량 반환
	int GetFreeSize()
	{
		int result = 0;
		int local_writeIdx = (int)writeIdx;
		int local_readIdx = (int)readIdx;

		if (local_readIdx < local_writeIdx)
		{
			if (local_readIdx ==0)
			{
				result = (max - 1) - local_writeIdx;
			}
			else
			{
				int a = max - local_writeIdx;
				int b = local_readIdx - 1; //공간을 하나 비워야 하기 때문에.
				result = a + b;
			}
		}
		else if(local_readIdx > local_writeIdx)
		{
			result = (local_readIdx - local_writeIdx) - 1;
		}
		else//둘이 같을때. 이경우는 초기 상태 이거나 read가 write를 다 따라 잡은 상황일때를 의미한다.
		{
			result = max-1; //최대치에서 한칸 비워진 값을 반환한다.쓰다보면 한칸 비워야하기 때문에
		}
		return result;
	}
	//큐가 비어있나요?
	//큐가 차 있나요?
	//큐를 비우기
	bool ClearAll()
	{
		curDataSize = 0;
		readIdx = writeIdx = 0;

		return true;
	}
		
};


/*
//멀티스레드 테스트코드

unsigned long long pushCount = 0;
unsigned long long pullbackCount = 0;
RingBufferEx ring(100000);
unsigned __stdcall PushThread(LPVOID arg)
{
	srand(time(0));
	while (true)
	{
		int count = rand() % 100;
		for (int i=0;i<count;++i)
		{
			char packet[10] = { "abcdefghi" };
			memcpy(ring.GetWritebufferPtr(), packet,10);
			ring.MoveWriteIdxEx(&ring.writeIdx, 10);
			_InterlockedIncrement(&pushCount);
		}


	}

	return 0;
}
unsigned __stdcall PullBackThread(LPVOID arg)
{
	while (true)
	{
		int useSize = ring.GetUseSize();
		if (useSize >0)
		{
			int count = useSize / 10;
			if ((useSize%10)!=0)
			{
				int error = 10;
			}
			int success = 0;
			for (int i=0;i< count;++i)
			{
				char packet[10] = { 0 };
				memcpy(packet, ring.GetReadBufferPtr(), 10);
				ring.MoveReadIdxEx(&ring.readIdx,  10);
				if (strcmp(packet,"abcdefghi")!=0)
				{
					int error = 10;
				}
				else
				{
					++success;
				}
				_InterlockedIncrement(&pullbackCount);
			}
		}
	}
	return 0;
}
void RingBufferTest()
{

	HANDLE hPushThread;
	hPushThread = (HANDLE)_beginthreadex(0, 0, PushThread, NULL, 0, nullptr);
	if (hPushThread == NULL)
	{
		return;
	}
	CloseHandle(hPushThread);

	HANDLE hPullBackThread;
	hPullBackThread = (HANDLE)_beginthreadex(0, 0, PullBackThread, NULL, 0, nullptr);
	if (hPullBackThread == NULL)
	{
		return;
	}
	CloseHandle(hPullBackThread);

	while (true)
	{
		std::string str;
		int totalLine = 23;
		str += format_string("pushCount         : %lld\n", pushCount); --totalLine;
		str += format_string("pullbackCount     : %lld\n", pullbackCount); --totalLine;
		for (int i = 0; i < totalLine; i++)
		{
			str += std::string("\n");
		}
		cout << str.c_str();
		InterlockedExchange(&pushCount, 0);
		InterlockedExchange(&pullbackCount, 0);
		Sleep(1000);
	}
}
*/