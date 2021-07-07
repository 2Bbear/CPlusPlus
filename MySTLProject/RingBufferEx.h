#pragma once
using namespace std;

#include "stdafx.h"

#define ERRTHROW(_errorType,msg) (RingBufferException(_errorType,msg,__FILE__,__LINE__))
/// @brief ������ Ŭ������ ���� ����ó���� �����մϴ�.
class RingBufferException : public exception
{
public:
	enum class EExceptionType :int
	{
		Default = 0,
		Error_EmptyBuffer,//���۰� �������
		Error_Overflow,//���۰� ���� �� ����
		Error_TargetSizeDifferent,//������ ũ�Ⱑ ���� ����.
		Error_ReadIdxOverMaxIdx, //readIdx�� �迭 �ִ� �ε����� �Ѿ
		Error_WriteIdxOverMaxIdx, // writeIdx�� �迭 �ִ� �ε����� �Ѿ��.
		Error_WrongInd,//�߸��� �ε��� ��ġ
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
/// @brief Enqueu�� �ϸ鼭 ���� write Idx�� readIdx�� ������ �� ����.
class RingBufferEx
{
public:
	int max; // ������ �ִ� �뷮
	unsigned long long readIdx;//�б� ������ ��ġ
	unsigned long long writeIdx;//���� ������ ��ġ
	int curDataSize; //���� ������ ũ��
public:
	char* buffer; // ���� ��ü
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
	/// @brief ���� ũ�� �ʱ�ȭ , ������ �����ʹ� ��� �����˴ϴ�.
	/// @param _reSize 
	/// @return ��������
	bool InitBufferSize(int _reSize)
	{
		//������ �ʱ�ȭ
		curDataSize = 0;
		readIdx = writeIdx = 0;
		free(buffer);

		// �缳��
		max = _reSize;
		buffer = (char*)malloc(max);

		return true;
	}
	/// @brief ���ۿ� ������ �ֱ�
	/// @param _inData ���� ������ ���� ������
	/// @param _inSize ���� ������ ũ��
	/// @return �־��� ������ �� ũ��, 0�� ��� �ֱ� ����
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
	/// @brief ���ۿ��� ������ ����
	/// @param _destPtr ���� ���� ������ ������
	/// @param _outSize ���� ������ ũ��
	/// @return �������� ������ �� ũ�� , 0�� ��� ���� ����
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
				"readIdx���� 0���� �۽��ϴ�.");
		}
#endif // _TEST

		return result;
	}

	//ť�� read ������
	char* GetReadBufferPtr()
	{
		return buffer + readIdx;
	}
	//ť�� write ������
	char* GetWritebufferPtr()
	{
		return buffer + writeIdx;
	}
	/// @brief readIdx �����̴� �Լ�, �ѹ��� ������ �� �ִ� �� �̻��� �����̸� error ������.
	/// @param _readIdx 
	/// @param _moveSize 
	/// @return 
	bool MoveReadIdx(int * _readIdx,int _moveSize) //throw(RingBufferException)
	{
		int temp =(*_readIdx) + _moveSize;
		if (temp ==max) //�ִ� �ε����� �����߱� ������ 0���� �Űܾ���
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
					"readIdx���� 0���� �۽��ϴ�.");
			}
#endif // _TEST
			return true;
		}
		else if (temp >max)
		{
			throw ERRTHROW(RingBufferException::EExceptionType::Error_ReadIdxOverMaxIdx,
				"readIdx�� maxIdx�� �ʰ��߽��ϴ�.");
		}
		return false;
	}
	/// @brief writeIdx�� �����̴� �Լ� �ѹ��� ������ �� �ִ� �� �̻��� �����̸� error�� ������.
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
				"writeIdx�� maxIdx�� �ʰ��߽��ϴ�.");
		}
		return false;
	}

	/// @brief readIdx �����̴� �Լ�, �ѹ��� ��踦 �Ѿ �����δ�. idx�� ������ �� �ִ�.
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
	/// @brief writeIdx �����̴� �Լ�, �ѹ��� ��踦 �Ѿ �����δ�. idx�� �������� ���ٸ� false�� ��ȯ�Ѵ�.
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
	//���ۿ��� �����͸� ��ũ
	int Peek(unsigned char* chpDest, int iSize) //throw(RingBufferException)
	{
		int iReadIdx = (int)readIdx;
		int iWriteIDx = (int)writeIdx;
		if (GetUseSize() < iSize)
		{
			throw ERRTHROW(RingBufferException::EExceptionType::Error_TargetSizeDifferent,"���ϴ� ������ ũ�� ��ŭ ���۾ȿ� �����Ͱ� �����ϴ�.");
			return 0;
		}

		//�ѹ��� �� �� �ִ� ũ�� ���ϱ�
		int directDequeSize = GetDirectDequeSize(iReadIdx, iWriteIDx);
		if (directDequeSize>= iSize)
		{
			memcpy(chpDest, buffer + iReadIdx, iSize);
			return iSize;
		}
		else //2���� ���� ������ ���MoveReadIdxEx
		{
			memcpy(chpDest, buffer + iReadIdx, directDequeSize); 
			MoveReadIdx(&iReadIdx, directDequeSize);

			int remainDequeSize = iSize - directDequeSize;

			memcpy(chpDest+ directDequeSize, buffer + iReadIdx, remainDequeSize);

			return directDequeSize+ remainDequeSize;
		}
		return 0;
	}
	//�ѹ��� �� �� �ִ� �뷮 ���ϱ�
	int GetDirectDequeSize(int _readIdx, int _writeIdx)
	{
		int result = 0;

		if (_readIdx < _writeIdx)
		{
			result = _writeIdx - _readIdx;
		}
		else if(_readIdx > _writeIdx)
		{
			//read���� ������.
			result = max - _readIdx;
		}

		return result;
	}
	//�ѹ��� ���� �� �ִ� �뷮 ���ϱ�
	//�ִ� ���� 1����Ʈ �����ϰ� ��
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
				//read���� ������.
				result = (_readIdx -1) - _writeIdx;
			}
			else if (_readIdx== _writeIdx)
			{
				result = max- _writeIdx;
			}
		}
		return result;
	}
	//������ ���� �� �뷮 ��ȯ
	int GetMaxSize()
	{
		return max;
	}
	//���ۿ� �׿��ִ� ������ �뷮 ��ȯ
	int GetUseSize(void)
	{
		int local_writeIdx = (int)writeIdx;
		int local_readIdx = (int)readIdx;

		if (local_writeIdx > local_readIdx)
		{
			return local_writeIdx - local_readIdx;
		}
		else if (local_writeIdx < local_readIdx) //���� �����Ͱ� ��輱�� �־ ...����� ������ ũ�⸦ ��
		{
			int a = max - local_readIdx; //�迭 �������� ������ ������
			int b = local_writeIdx; // �迭 ���ۺ� ���� writeIdx �������� ������ ������
			return a + b;
		}
		else
		{
			return 0;
		}
	}
	//���ۿ� Enque�� �� �ִ� ��� �ִ� ������ �뷮 ��ȯ
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
				int b = local_readIdx - 1; //������ �ϳ� ����� �ϱ� ������.
				result = a + b;
			}
		}
		else if(local_readIdx > local_writeIdx)
		{
			result = (local_readIdx - local_writeIdx) - 1;
		}
		else//���� ������. �̰��� �ʱ� ���� �̰ų� read�� write�� �� ���� ���� ��Ȳ�϶��� �ǹ��Ѵ�.
		{
			result = max-1; //�ִ�ġ���� ��ĭ ����� ���� ��ȯ�Ѵ�.���ٺ��� ��ĭ ������ϱ� ������
		}
		return result;
	}
	//ť�� ����ֳ���?
	//ť�� �� �ֳ���?
	//ť�� ����
	bool ClearAll()
	{
		curDataSize = 0;
		readIdx = writeIdx = 0;

		return true;
	}
		
};


/*
//��Ƽ������ �׽�Ʈ�ڵ�

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