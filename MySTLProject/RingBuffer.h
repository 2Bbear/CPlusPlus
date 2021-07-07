#pragma once
#include <iostream>
#include <Windows.h>
#include <assert.h>
using namespace std;
class RingBuffer
{
public:
	int readPos;
	int writePos;
	int bufferSize; //유저가 원한 크기
	int arraySize; // 실제 만들어진 배열의 크기
	char * values;
	int curUseCount = 0; //현재 사용하고 있는 용량
public:
	RingBuffer() 
	{
		bufferSize = 0;
		values = nullptr;
		readPos = 0;
		writePos = 0;
		arraySize = 0;
	}
	RingBuffer(int _buffSize) 
	{
		bufferSize = _buffSize;
		arraySize = bufferSize + 1;
		values = new char[arraySize];
		memset(values,0, bufferSize);
		readPos = 0;
		writePos = 0;
	}
	~RingBuffer()
	{
		delete[] values;
		values = nullptr;
	}
public:
	char* GetValuesPtr()
	{
		return values;
	}
	int GetRearPointIndex()
	{
		return writePos;
	}
	int GetFrontPointIndex()
	{
		return readPos;
	}
	/// @brief writePos에 데이터를 넣는다.
	/// @param chpData 데이터 포인터
	/// @param iSize 크기
	/// @return 넣는데 성공한 크기
	int Enqueue(char* chpData, int iSize)
	{
		int localWritePos = writePos;
		int localReadPos = readPos;
		//차 있는 것이 아닌지
		if (IsFull() == true)
		{
			return 0;
		}
		//iSize 만큼 넣을 공간이 있는지
		if (GetFreeSize() < iSize)
		{
			return 0;
		}
		//한번에 넣을 수 있는 공간 구하기===============
		int oneSize = 0;
		if (localReadPos > localWritePos) 
		{
			oneSize= GetFreeSize();
		}
		else 
		{
			if (localReadPos != 0)
			{
				oneSize = arraySize - localWritePos;
			}
			else
			{
				oneSize = bufferSize - localWritePos;
			}
			
		}
		//==============================================
		int cpySize = 0;
		if (oneSize> iSize)
		{
			cpySize = iSize;
		}
		else
		{
			cpySize = oneSize;
		}
		if (cpySize == 0) { return 0; }
		
		memcpy(values+localWritePos,chpData,cpySize);

		localWritePos += cpySize;
		chpData += cpySize;

		if (localWritePos >= arraySize)
		{
			localWritePos = 0;
		}

		//아직 넣어야 할 데이터가 남아 있다면
		int remainISize = iSize - cpySize;//(iSize - oneSize);

		if (remainISize > 0)
		{
			oneSize = 0;
			//한번에 넣을 수 있는 공간 구하기
			if (localReadPos > localWritePos)
			{
				oneSize = GetFreeSize();
			}
			else
			{
				if (localReadPos != 0)
				{
					oneSize = arraySize - localWritePos;
				}
				else
				{
					oneSize = bufferSize - localWritePos;
				}
			}
			//===============================================
			if (oneSize > remainISize)
			{
				cpySize = remainISize;
			}
			else
			{
				cpySize = oneSize;
			}
			memcpy(values + localWritePos, chpData, cpySize);
			localWritePos += cpySize;

			if (localWritePos >= arraySize)
			{
				localWritePos = 0;
			}
		}
		//writePos와 readPos를 움직일때는 반드시 마지막에 한번에 다 합쳐서 해야한다.
		writePos = localWritePos;

		return iSize;
	}
	/// @brief ReadPos 에서 데이타 가져옴. ReadPos 이동.
	/// @param chpDest 데이터 포인터
	/// @param iSize  크기
	/// @return 가져오는데 성공한 크기
	int Dequeue(char* chpDest, int iSize)
	{
		int localWritePos = writePos;
		int localReadPos = readPos;

		//가져올 크기 만큼 값이 들어 있는지
		if (GetUseSize() < iSize)
		{
			iSize = GetUseSize();
		}
		//한번에 뺄 수 있는 공간 구하기===============
		int oneSize = 0;
		if (localReadPos > localWritePos)
		{
			oneSize = arraySize - localReadPos;
		}
		else
		{
			oneSize = localWritePos - localReadPos;
		}
		
		//==============================================
		int cpySize = 0;
		if (oneSize > iSize)
		{
			cpySize = iSize;
		}
		else
		{
			cpySize = oneSize;
		}
		if (cpySize == 0) { return 0; }

		memcpy(chpDest, values + localReadPos, cpySize);

		localReadPos += cpySize;
		chpDest += cpySize;

		if (localReadPos >= arraySize)
		{
			localReadPos = 0;
		}

		//아직 넣어야 할 데이터가 남아 있다면
		int remainISize = iSize - cpySize;//(iSize - oneSize);

		if (remainISize > 0)
		{
			oneSize = 0;
			//한번에 뺄 수 있는 공간 구하기===============
			int oneSize = 0;
			if (localReadPos > localWritePos)
			{
				oneSize = arraySize - localReadPos;
			}
			else
			{
				oneSize = localWritePos - localReadPos;
			}

			//==============================================
			if (oneSize > remainISize)
			{
				cpySize = remainISize;
			}
			else
			{
				cpySize = oneSize;
			}
			memcpy(chpDest, values + localReadPos, cpySize);
			localReadPos += cpySize;

			if (localReadPos >= arraySize)
			{
				localReadPos = 0;
			}
		}

		
		readPos = localReadPos;

		return iSize;
	}

	/// @brief 버퍼의 크기를 조절
	/// @param _size 
	void ReSize(int _size)
	{
		int tempArraySize = _size + 1;
		char* tempValues = new char[tempArraySize];
		memset(tempValues, 0, tempArraySize);

		if (arraySize > tempArraySize)
		{
			memcpy(tempValues, values, tempArraySize);
			cout << "ReSize warrning : size 값을 기존보다 작게 변경했습니다." << endl;
		}
		else
		{
			memcpy(tempValues, values, bufferSize);
		}
		delete[] values;
		bufferSize = _size;
		arraySize = tempArraySize;
		values = tempValues;
		readPos = 0;
		writePos = 0;
	}
	/// @brief 현재 버퍼 비어있는 부분을 포함한 총 크기를 반환
	/// @param  
	/// @return 
	int GetBufferSize(void)
	{
		return bufferSize;
	}

	/// @brief 현재 사용하고 있는 데이터의 크기를 나타냄.
	/// @param  
	/// @return 
	int GetUseSize(void)
	{
		if (writePos > readPos)
		{
			return writePos - readPos;
		}
		else if (writePos < readPos) //만약 데이터가 경계선상에 있어도 ...저장된 데이터 크기를 줌
		{
			int a = arraySize - readPos;
			int b = writePos;
			return a + b;
		}
		else
		{
			return 0;
		}
	}
	/// @brief 현재 버퍼에 남은 용량 얻기
	/// @param  
	/// @return 
	int GetFreeSize(void)
	{
		int curFreeSize = bufferSize - GetUseSize();
		return curFreeSize;
	}
	

	/// @brief 버퍼 포인터로 외부에서 한방에 읽고 쓸 수 있는 길이, 끊기지 않아야 함
	/// @param  
	/// @return 사용가능한 용량
	int DirectEnqueueSize(void)
	{
		int result = 0;
		if (readPos > writePos)
		{
			result = GetFreeSize();
		}
		else if (readPos<writePos)
		{
			result= bufferSize - writePos;
		}
		else
		{
			result= bufferSize- writePos;
		}
		return result;
	}

	/// @brief 버퍼 포인터로 외부에서 한방에 내보낼 수 있는 길이, 끊기지 않아야 함
	/// @param  
	/// @return 사용 가능한 용량
	int DirectDequeueSize(void)
	{
		if (readPos > writePos)
		{
			return bufferSize - readPos;
		}
		else if (readPos < writePos)
		{
			return writePos - readPos;
		}
		return 0;
	}

	

	/// @brief ReadPos 에서 데이타 읽어옴. ReadPos 고정.
	/// @param chpDest 데이터 포인터
	/// @param iSize 크기
	/// @return 가져온 크기
	int Peek(char* chpDest, int iSize)
	{
		int localWritePos = writePos;
		int localReadPos = readPos;

		//가져올 크기 만큼 값이 들어 있는지
		if (GetUseSize() < iSize)
		{
			iSize = GetUseSize();
			
		}
		//한번에 뺄 수 있는 공간 구하기===============
		int oneSize = 0;
		if (localReadPos > localWritePos)
		{
			oneSize = arraySize - localReadPos;
		}
		else
		{
			oneSize = localWritePos - localReadPos;
		}

		//==============================================
		int cpySize = 0;
		if (oneSize > iSize)
		{
			cpySize = iSize;
		}
		else
		{
			cpySize = oneSize;
		}
		if (cpySize == 0) { return 0; }

		memcpy(chpDest, values + localReadPos, cpySize);

		localReadPos += cpySize;
		chpDest += cpySize;

		if (localReadPos >= arraySize)
		{
			localReadPos = 0;
		}

		//아직 넣어야 할 데이터가 남아 있다면
		int remainISize = iSize - cpySize;//(iSize - oneSize);

		if (remainISize > 0)
		{
			oneSize = 0;
			//한번에 뺄 수 있는 공간 구하기===============
			int oneSize = 0;
			if (localReadPos > localWritePos)
			{
				oneSize = arraySize - localReadPos;
			}
			else
			{
				oneSize = localWritePos - localReadPos;
			}

			//==============================================
			if (oneSize > remainISize)
			{
				cpySize = remainISize;
			}
			else
			{
				cpySize = oneSize;
			}
			memcpy(chpDest, values + localReadPos, cpySize);
			localReadPos += cpySize;

			if (localReadPos >= arraySize)
			{
				localReadPos = 0;
			}
		}


		//readPos = localReadPos;

		return iSize;
	}

	/// @brief 원하는 길이만큼 읽기위치 에서 삭제 / 쓰기 위치 이동
	/// @param iSize 
	void MoveRear(int iSize)
	{
		int localWritePos = writePos;
		int localReadPos = readPos;
		//차 있는 것이 아닌지
		if (IsFull() == true)
		{
			return ;
		}
		//iSize 만큼 넣을 공간이 있는지
		if (GetFreeSize() < iSize)
		{
			return ;
		}
		//한번에 넣을 수 있는 공간 구하기===============
		int oneSize = 0;
		if (localReadPos > localWritePos)
		{
			oneSize = GetFreeSize();
		}
		else
		{
			if (localReadPos != 0)
			{
				oneSize = arraySize - localWritePos;
			}
			else
			{
				oneSize = bufferSize - localWritePos;
			}

		}
		//==============================================
		int cpySize = 0;
		if (oneSize > iSize)
		{
			cpySize = iSize;
		}
		else
		{
			cpySize = oneSize;
		}
		if (cpySize == 0) { return ; }

		localWritePos += cpySize;

		if (localWritePos >= arraySize)
		{
			localWritePos = 0;
		}

		//아직 넣어야 할 데이터가 남아 있다면
		int remainISize = iSize - cpySize;//(iSize - oneSize);

		if (remainISize > 0)
		{
			oneSize = 0;
			//한번에 넣을 수 있는 공간 구하기
			if (localReadPos > localWritePos)
			{
				oneSize = GetFreeSize();
			}
			else
			{
				if (localReadPos != 0)
				{
					oneSize = arraySize - localWritePos;
				}
				else
				{
					oneSize = bufferSize - localWritePos;
				}
			}
			//===============================================
			if (oneSize > remainISize)
			{
				cpySize = remainISize;
			}
			else
			{
				cpySize = oneSize;
			}
			localWritePos += cpySize;

			if (localWritePos >= arraySize)
			{
				localWritePos = 0;
			}
		}
		//writePos와 readPos를 움직일때는 반드시 마지막에 한번에 다 합쳐서 해야한다.
		if (localWritePos==bufferSize)
		{
			localWritePos = 0;
		}
		writePos = localWritePos;
	}

	/// @brief 원하는 길이만큼 읽기위치 에서 삭제 / 쓰기 위치 이동
	/// @param iSize 
	/// @return 
	int MoveFront(int iSize)
	{
		int localWritePos = writePos;
		int localReadPos = readPos;

		//가져올 크기 만큼 값이 들어 있는지
		if (GetUseSize() < iSize)
		{
			iSize = GetUseSize();
		}
		//한번에 뺄 수 있는 공간 구하기===============
		int oneSize = 0;
		if (localReadPos > localWritePos)
		{
			oneSize = arraySize - localReadPos;
		}
		else
		{
			oneSize = localWritePos - localReadPos;
		}

		//==============================================

		int cpySize = 0;
		if (oneSize > iSize)
		{
			cpySize = iSize;
		}
		else
		{
			cpySize = oneSize;
		}
		if (cpySize == 0) { return 0; }

		localReadPos += cpySize;
		if (localReadPos>=arraySize)
		{
			localReadPos = 0;
		}
		//아직 넣어야 할 데이터가 남아 있다면
		int remainISize = iSize - cpySize;//(iSize - oneSize);

		if (remainISize > 0)
		{
			oneSize = 0;
			//한번에 뺄 수 있는 공간 구하기===============
			int oneSize = 0;
			if (localReadPos > localWritePos)
			{
				oneSize = arraySize - localReadPos;
			}
			else
			{
				oneSize = localWritePos - localReadPos;
			}

			//==============================================
			if (oneSize > remainISize)
			{
				cpySize = remainISize;
			}
			else
			{
				cpySize = oneSize;
			}
			localReadPos += cpySize;

			if (localReadPos >= arraySize)
			{
				localReadPos = 0;
			}
		}
		if (localReadPos == bufferSize)
		{
			localReadPos = 0;
		}
		readPos = localReadPos;


		return 0;
	}

	/// @brief 버퍼의 모든 데이터 삭제
	/// @param  
	void ClearBuffer(void)
	{

	}

	/// @brief 버퍼의 front 포인터를 얻음
	/// send 함수에는 사용하지 마세요. 데이터 경계일때 잘못된 값을 가져오게 될 것입니다.!!
	/// 반드시 buffer를 경유해서 send를 사용하세요.
	/// @param  
	/// @return 
	char* GetFrontBufferPtr(void)
	{
		return values+ readPos;
	}

	/// @brief 버퍼의 rearpos 포인터를 얻음
	/// @param  
	/// @return 
	char* GetRearBufferPtr(void)
	{
		return values +writePos;
	}
public:
	void Initial(int _bufferSize)
	{
		
		delete[] values;

		bufferSize = _bufferSize;
		arraySize = bufferSize + 1;
		values = new char[arraySize];
		memset(values, 0, bufferSize);
		readPos = 0;
		writePos = 0;
	}
	bool IsFull()
	{
		//writePos가 배열 마지막에 위치 해 있을 때
		if (writePos==bufferSize)
		{
			//만약 0번째 인덱스에 readPos가 있다면
			if (readPos==0)
			{
				return true;
			}
		}
		else
		{
			if ((writePos+1)==readPos)
			{
				return true;
			}
		}

		return false;
	}
	
};




