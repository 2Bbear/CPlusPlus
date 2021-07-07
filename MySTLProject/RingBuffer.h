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
	int bufferSize; //������ ���� ũ��
	int arraySize; // ���� ������� �迭�� ũ��
	char * values;
	int curUseCount = 0; //���� ����ϰ� �ִ� �뷮
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
	/// @brief writePos�� �����͸� �ִ´�.
	/// @param chpData ������ ������
	/// @param iSize ũ��
	/// @return �ִµ� ������ ũ��
	int Enqueue(char* chpData, int iSize)
	{
		int localWritePos = writePos;
		int localReadPos = readPos;
		//�� �ִ� ���� �ƴ���
		if (IsFull() == true)
		{
			return 0;
		}
		//iSize ��ŭ ���� ������ �ִ���
		if (GetFreeSize() < iSize)
		{
			return 0;
		}
		//�ѹ��� ���� �� �ִ� ���� ���ϱ�===============
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

		//���� �־�� �� �����Ͱ� ���� �ִٸ�
		int remainISize = iSize - cpySize;//(iSize - oneSize);

		if (remainISize > 0)
		{
			oneSize = 0;
			//�ѹ��� ���� �� �ִ� ���� ���ϱ�
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
		//writePos�� readPos�� �����϶��� �ݵ�� �������� �ѹ��� �� ���ļ� �ؾ��Ѵ�.
		writePos = localWritePos;

		return iSize;
	}
	/// @brief ReadPos ���� ����Ÿ ������. ReadPos �̵�.
	/// @param chpDest ������ ������
	/// @param iSize  ũ��
	/// @return �������µ� ������ ũ��
	int Dequeue(char* chpDest, int iSize)
	{
		int localWritePos = writePos;
		int localReadPos = readPos;

		//������ ũ�� ��ŭ ���� ��� �ִ���
		if (GetUseSize() < iSize)
		{
			iSize = GetUseSize();
		}
		//�ѹ��� �� �� �ִ� ���� ���ϱ�===============
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

		//���� �־�� �� �����Ͱ� ���� �ִٸ�
		int remainISize = iSize - cpySize;//(iSize - oneSize);

		if (remainISize > 0)
		{
			oneSize = 0;
			//�ѹ��� �� �� �ִ� ���� ���ϱ�===============
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

	/// @brief ������ ũ�⸦ ����
	/// @param _size 
	void ReSize(int _size)
	{
		int tempArraySize = _size + 1;
		char* tempValues = new char[tempArraySize];
		memset(tempValues, 0, tempArraySize);

		if (arraySize > tempArraySize)
		{
			memcpy(tempValues, values, tempArraySize);
			cout << "ReSize warrning : size ���� �������� �۰� �����߽��ϴ�." << endl;
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
	/// @brief ���� ���� ����ִ� �κ��� ������ �� ũ�⸦ ��ȯ
	/// @param  
	/// @return 
	int GetBufferSize(void)
	{
		return bufferSize;
	}

	/// @brief ���� ����ϰ� �ִ� �������� ũ�⸦ ��Ÿ��.
	/// @param  
	/// @return 
	int GetUseSize(void)
	{
		if (writePos > readPos)
		{
			return writePos - readPos;
		}
		else if (writePos < readPos) //���� �����Ͱ� ��輱�� �־ ...����� ������ ũ�⸦ ��
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
	/// @brief ���� ���ۿ� ���� �뷮 ���
	/// @param  
	/// @return 
	int GetFreeSize(void)
	{
		int curFreeSize = bufferSize - GetUseSize();
		return curFreeSize;
	}
	

	/// @brief ���� �����ͷ� �ܺο��� �ѹ濡 �а� �� �� �ִ� ����, ������ �ʾƾ� ��
	/// @param  
	/// @return ��밡���� �뷮
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

	/// @brief ���� �����ͷ� �ܺο��� �ѹ濡 ������ �� �ִ� ����, ������ �ʾƾ� ��
	/// @param  
	/// @return ��� ������ �뷮
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

	

	/// @brief ReadPos ���� ����Ÿ �о��. ReadPos ����.
	/// @param chpDest ������ ������
	/// @param iSize ũ��
	/// @return ������ ũ��
	int Peek(char* chpDest, int iSize)
	{
		int localWritePos = writePos;
		int localReadPos = readPos;

		//������ ũ�� ��ŭ ���� ��� �ִ���
		if (GetUseSize() < iSize)
		{
			iSize = GetUseSize();
			
		}
		//�ѹ��� �� �� �ִ� ���� ���ϱ�===============
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

		//���� �־�� �� �����Ͱ� ���� �ִٸ�
		int remainISize = iSize - cpySize;//(iSize - oneSize);

		if (remainISize > 0)
		{
			oneSize = 0;
			//�ѹ��� �� �� �ִ� ���� ���ϱ�===============
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

	/// @brief ���ϴ� ���̸�ŭ �б���ġ ���� ���� / ���� ��ġ �̵�
	/// @param iSize 
	void MoveRear(int iSize)
	{
		int localWritePos = writePos;
		int localReadPos = readPos;
		//�� �ִ� ���� �ƴ���
		if (IsFull() == true)
		{
			return ;
		}
		//iSize ��ŭ ���� ������ �ִ���
		if (GetFreeSize() < iSize)
		{
			return ;
		}
		//�ѹ��� ���� �� �ִ� ���� ���ϱ�===============
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

		//���� �־�� �� �����Ͱ� ���� �ִٸ�
		int remainISize = iSize - cpySize;//(iSize - oneSize);

		if (remainISize > 0)
		{
			oneSize = 0;
			//�ѹ��� ���� �� �ִ� ���� ���ϱ�
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
		//writePos�� readPos�� �����϶��� �ݵ�� �������� �ѹ��� �� ���ļ� �ؾ��Ѵ�.
		if (localWritePos==bufferSize)
		{
			localWritePos = 0;
		}
		writePos = localWritePos;
	}

	/// @brief ���ϴ� ���̸�ŭ �б���ġ ���� ���� / ���� ��ġ �̵�
	/// @param iSize 
	/// @return 
	int MoveFront(int iSize)
	{
		int localWritePos = writePos;
		int localReadPos = readPos;

		//������ ũ�� ��ŭ ���� ��� �ִ���
		if (GetUseSize() < iSize)
		{
			iSize = GetUseSize();
		}
		//�ѹ��� �� �� �ִ� ���� ���ϱ�===============
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
		//���� �־�� �� �����Ͱ� ���� �ִٸ�
		int remainISize = iSize - cpySize;//(iSize - oneSize);

		if (remainISize > 0)
		{
			oneSize = 0;
			//�ѹ��� �� �� �ִ� ���� ���ϱ�===============
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

	/// @brief ������ ��� ������ ����
	/// @param  
	void ClearBuffer(void)
	{

	}

	/// @brief ������ front �����͸� ����
	/// send �Լ����� ������� ������. ������ ����϶� �߸��� ���� �������� �� ���Դϴ�.!!
	/// �ݵ�� buffer�� �����ؼ� send�� ����ϼ���.
	/// @param  
	/// @return 
	char* GetFrontBufferPtr(void)
	{
		return values+ readPos;
	}

	/// @brief ������ rearpos �����͸� ����
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
		//writePos�� �迭 �������� ��ġ �� ���� ��
		if (writePos==bufferSize)
		{
			//���� 0��° �ε����� readPos�� �ִٸ�
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




