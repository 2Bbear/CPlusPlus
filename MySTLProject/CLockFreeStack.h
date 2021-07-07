#pragma once

#include <new.h>
#include <iostream>
#include <list>
#include "CCrashDump.h"
using namespace std;
#define ALIGNMENT 8
#define ALIGN(p) (((sizeof(p)) + (ALIGNMENT-1)) & ~0x7) 
#define SHORT 2
//���� �ָ� �Ǿ� ����� �������� ������ �ִ� ��ũ��
#define SELFPOINTER(block) (*((unsigned long long*)((((char*)block)))))
//���� �ָ� �� underflowshort�� ���� ���·� ���� �ִ� ��ũ��
#define UNDERFLOWSHORT(block) (*((short*)(((char*)block)+16)))
//����, ��ü ũ�⸦ �ָ� ������ overflowshort�� ���� ���·� ������ּ� �ִ� ��ũ�� ,,�е����� ��� �ؾ���
#define OVERFLOWSHORT(block,blockTotalsize) (*((short*)((char*)block + (blockTotalsize - SHORT))))
//��� ī��Ʈ�ٷ� �ִ� ��ũ��
#define USECOUNTINT(block) (*((int*)((((char*)block)+8))))
//DATA�� ��ġ�� �ٷ� �ִ� ��ũ��
#define BL_DATA_P(block) (((char*)block)+16 + SHORT + sizeof(bool)+1 )
//���� ��带 ��Ÿ���� ��ġ�� ����ִ� ��ũ�� objSize�� �������� ũ��
#define NEXT_BL_P(block,objSize) (((char*)block)+SHORT+16+ sizeof(bool)+1+ objSize)
//���� �ָ� �ٷ� ��뿩��bool ���� ���·� ���� �ִ� ��ũ��
#define ISUSEBOOL(block) (*((bool*)(char*)block+16+SHORT))

/*
��ü���� �������ִ� Ŭ����
�̹� ������� ��ü�� �ٽ� ���ٴ� ������ �޸� ������
�ݵ�� �ִ� ��ü Ÿ���� ¦���� �������� �Ѵ�. �е��� ���� ����� ���� ����.
¦���� �ƴ� ��� �߸��ϸ� ����Ʈ ������ġ�� ��߳� �� �ִ�.
Block ����
[2 size][1 isuse]+[1 padding]+[4 useCount ]+[? data][4 or 8 nextBlockPtr][2 size]
[8 �����������] + [8 ���ī��Ʈ] + [2 size][1 isuse]+[1 padding]+[? data][4 or 8 nextBlockPtr][2 size]
*/
template <typename DATA>
class CLockFreeStack
{
protected:
	__declspec(align(16))struct ST16Long //���ʿ� _InterlockedCompareExchange128 �� ���� ���̴� 16����Ʈ ������ �ؾ��Ѵ�.
	{
		unsigned long long  low_topPtr; // 8����Ʈ
		unsigned long long  high_topPtrCount; // 8����Ʈ...
		ST16Long()
		{
			low_topPtr = 0;
			high_topPtrCount = 0;
		}
		ST16Long(ST16Long* _ptr)
		{
			if (_ptr == nullptr)
			{
				low_topPtr = 0;
				high_topPtrCount = 0;
			}
			else
			{
				low_topPtr = _ptr->low_topPtr;
				high_topPtrCount = _ptr->high_topPtrCount;
			}

		}
	};
public:
	ST16Long  top;
	bool isPlacementNew;
	list<void*> nodeClearList;
	bool isDestroyAll;
private:
	//���� �� �� ��
	__declspec(align(32)) unsigned long m_iAllocCount;
	// ���ǰ� �ִ� �� ��
	__declspec(align(32)) unsigned long m_iUseCount;
private:
#ifdef _TEST
	unsigned long long test_AllocIdx = 0;

	struct MemoryLogData
	{
		void* blockPtr;
		void* topPtr;
		void* topPtrNext;
		DWORD threadID;
		int count;
		int sub;//�ش� �Լ����� ���° ����
		int repeatCount;
		bool interlockCompareResult;
		const char* func;

	};
	__declspec(align(32))  unsigned long long memoryLogIdx = 0;
	__declspec(align(32))  unsigned long long memoryLogRepeatCount = 0;
	MemoryLogData* MemoryLogArr;
	void MemoryLog(DWORD _threadID, const char* _func, int _sub, void* _blockPtr = nullptr, void* _topPtr = nullptr, bool _interlockCompareResult = false, void* _topPtrNext = nullptr, int _count = 0)
	{
		unsigned long long idx = 0;
		idx = _InterlockedIncrement(&memoryLogIdx);
		if (idx >= 100000)
		{
			_InterlockedExchange(&memoryLogIdx, 0);
			_InterlockedIncrement(&memoryLogRepeatCount);
		}
		memset(&MemoryLogArr[idx], 0, sizeof(MemoryLogData));
		MemoryLogArr[idx].threadID = _threadID;
		MemoryLogArr[idx].func = _func;
		MemoryLogArr[idx].sub = _sub;
		MemoryLogArr[idx].repeatCount = (int)memoryLogRepeatCount;
		MemoryLogArr[idx].blockPtr = _blockPtr;
		MemoryLogArr[idx].count = _count;
		MemoryLogArr[idx].topPtr = _topPtr;
		MemoryLogArr[idx].topPtrNext = _topPtrNext;
	}
#endif

public:
	CLockFreeStack(unsigned int _blockCount = 0, bool _isPlacementNew = false, bool _isDestroyAll = false)
	{
		MemoryLogArr = new MemoryLogData[100000];
		memset(&top, 0, sizeof(ST16Long));
		isPlacementNew = _isPlacementNew;
		isDestroyAll = _isDestroyAll;
		if (_blockCount != 0)
		{
			Alloc(true, _blockCount);
		}
	}
	~CLockFreeStack()
	{
		int remainedObj = 0;
		int deletedObj = 0;
		list<void*>::iterator itr = nodeClearList.begin();
		for (; itr != nodeClearList.end();)
		{
			if ((*itr) == nullptr)
			{
				printf("Object is remained \n");
				CCrashDump::Crash();
				++itr;
				continue;
			}
			if (isDestroyAll == true)
			{
				free((*itr));
				deletedObj++;
				++itr;
			}
			else
			{
				//��� ���� Ȯ���ϰ� ���� ����������
				bool check = ISUSEBOOL((*itr));
				if (check == true)
				{
					itr = nodeClearList.erase(itr);
					remainedObj++;
				}
				else
				{
					free((*itr));
					itr = nodeClearList.erase(itr);
					deletedObj++;
				}
			}
		}
		printf_s("Deleted Object : %d Remained Object %d\n", deletedObj, remainedObj);
	}

public:
	/// @brief �ܺο��� ������ ��� ��ҿ� ���� �����ڸ� ȣ�� ��Ű�� �Լ�
	void ExternalContructionAll()
	{
		list<void*>::iterator itr = nodeClearList.begin();
		for (; itr != nodeClearList.end();++itr)
		{
			DATA* element = nullptr;
			element = (DATA*)BL_DATA_P(*itr);
			if (element != nullptr)
			{
				new(element)DATA;
			}
		}
	}
	/// @brief �� �ϳ��� �Ҵ�޴´�.  
	/// @param  ����
	/// @return (DATA *) ����Ÿ �� ������
	DATA* Alloc(bool isConstructorCall = false, int _repeatCount = 1)
	{
#ifdef _TEST
		_InterlockedExchange(&test_AllocIdx, 0);
#endif // _TEST
		if (top.low_topPtr == 0)
		{
#ifdef _TEST
			MemoryLog(GetCurrentThreadId(), "Alloc", 1, nullptr, nullptr, false, nullptr, 0);
#endif // _TEST
			void* block = nullptr;
			for (int i = 0; i < _repeatCount; ++i)
			{
#ifdef _TEST
				_InterlockedIncrement(&test_AllocIdx);
#endif // _TEST
				// ���� ��ü�� ������ ��� 
				int objSize = ALIGN(DATA);
				//��ü ����
				int totalsize = 8 + 8 + sizeof(short) + sizeof(bool) + 1 + objSize + sizeof(ST16Long*) + sizeof(short);
				block = malloc(totalsize);
				nodeClearList.push_back(block);
				//�� ST16Long �ʱ�ȭ
				new(block)ST16Long;
				//���� ������ 
				SELFPOINTER(block) = (unsigned long long)block;

				//�� ��Ʈ
				UNDERFLOWSHORT(block) = objSize;
				//�� ��Ʈ
				OVERFLOWSHORT(block, totalsize) = objSize;
				
				//���� ������ �ʱ�ȭ
				void* data = NEXT_BL_P(block, objSize);
				memset(data, 0, sizeof(void*));
				//������ �ʱ�ȭ
				if (isPlacementNew == true)
				{
					new(BL_DATA_P(block))DATA;
				}
				//�����ڿ��� ȣ���� ���
				if (isConstructorCall == true)
				{
					//��뿩�� üũ
					ISUSEBOOL(block) = false;
					//ī����
					USECOUNTINT(block) = 0;
					if (top.low_topPtr == 0)
					{
						top.low_topPtr = (unsigned long long)block;
					}
					else
					{
						memcpy(data, (void*)&top.low_topPtr, sizeof(void*));//��� �̾���̱�
						top.low_topPtr = (unsigned long long)block;
					}
				}
				else
				{
					//��뿩�� üũ
					ISUSEBOOL(block) = true;
					top.high_topPtrCount += 1;
					_InterlockedIncrement(&m_iUseCount);
				}
				_InterlockedIncrement(&m_iAllocCount);
#ifdef _TEST
				//���� 
				//�������� ������ ���� Ȯ��
				short underSize = UNDERFLOWSHORT(block);
				short overSize = OVERFLOWSHORT(block, totalsize);
				if (overSize < 0 || underSize < 0 || underSize != overSize)
				{
					printf("��������%lld\n", test_AllocIdx);
					CCrashDump::Crash();
				}
#endif // _TEST
			}
			return (DATA*)BL_DATA_P(block);
		}
		else
		{
#ifdef _TEST
			_InterlockedIncrement(&test_AllocIdx);
			MemoryLog(GetCurrentThreadId(), "Alloc", 2, nullptr, nullptr, false, nullptr, 0);
#endif // _TEST
			ST16Long nextBlockCopy; //
			ST16Long local_top;
			ST16Long* block = nullptr; // ���� ���ø���Ʈ���� Ƣ����� �༮
			int objSize = ALIGN(DATA);
			bool interlockResult = false;
			do
			{
				memset(&local_top, 0, sizeof(ST16Long));
				memset(&nextBlockCopy, 0, sizeof(ST16Long));
				//top ���Ǽ�
				local_top.high_topPtrCount = top.high_topPtrCount;
				local_top.low_topPtr = top.low_topPtr;

				block = nullptr;
				block = (ST16Long*)local_top.low_topPtr;

				if (block == nullptr)
				{
					continue;
				}

				ST16Long* temp = (ST16Long*)(*((ST16Long**)(NEXT_BL_P(block, objSize)))); // ���� ��尡 �ִٸ� �����ͼ� 
				if (temp == nullptr)
				{
					nextBlockCopy.low_topPtr = 0;
					nextBlockCopy.high_topPtrCount = local_top.high_topPtrCount + 1;
				}
				else
				{
					nextBlockCopy.low_topPtr = temp->low_topPtr;
					nextBlockCopy.high_topPtrCount = local_top.high_topPtrCount + 1;
				}
				//���� ġȯ �� top ptr�� ������ ���� ���� ptr�� ���ٸ�?...�̰� ������ �ȴ�.
				if (block == (void*)nextBlockCopy.low_topPtr)
				{
					CCrashDump::Crash();
				}
				interlockResult = _InterlockedCompareExchange128((long long*)&top, nextBlockCopy.high_topPtrCount, nextBlockCopy.low_topPtr, (long long*)&local_top);
			} while (interlockResult == false);
#ifdef _TEST
			MemoryLog(GetCurrentThreadId(), "Alloc", 3, block, (void*)nextBlockCopy.low_topPtr,
				interlockResult, (void*)NEXT_BL_P((void*)nextBlockCopy.low_topPtr, objSize), nextBlockCopy.high_topPtrCount);
			if (top.high_topPtrCount <= 0)
			{
				CCrashDump::Crash();
			}
#endif // _TEST

			if (block == nullptr)
			{
				CCrashDump::Crash();
				return nullptr;
			}
			//���� üũ�ϱ�
			//��� ���� Ȯ��
			bool isUse = ISUSEBOOL(block);
			if (isUse == true)
			{
				CCrashDump::Crash();
			}
			//������ �ʱ�ȭ
			if (isPlacementNew == true)
			{
				new(BL_DATA_P(block))DATA;
			}
			//��� ���� üũ
			ISUSEBOOL(block) = true;
			//��� ī��Ʈ ���� üũ
			USECOUNTINT(block) = USECOUNTINT(block) + 1;
			_InterlockedIncrement(&m_iUseCount);

#ifdef _TEST
			//���� 
			int totalsize = 8 + 8 + sizeof(short) + sizeof(bool) + 1 + objSize + sizeof(ST16Long*) + sizeof(short);
			//�������� ������ ���� Ȯ��
			short underSize = UNDERFLOWSHORT(block);
			short overSize = OVERFLOWSHORT(block, totalsize);
			isUse = ISUSEBOOL(block);
			if (isUse == false || overSize < 0 || underSize < 0 || underSize != overSize)
			{
				printf("���Ҵ翡�� %lld\n", test_AllocIdx);
				CCrashDump::Crash();
			}
#endif // _TEST
#ifdef _TEST
			MemoryLog(GetCurrentThreadId(), "Alloc", 4, block, (void*)nextBlockCopy.low_topPtr, interlockResult,
				(void*)NEXT_BL_P((void*)nextBlockCopy.low_topPtr, objSize), nextBlockCopy.high_topPtrCount);
#endif // _TEST
			return (DATA*)BL_DATA_P(block);
		}
	}
	/// @brief ������̴� ���� �����Ѵ�.
	/// @param pData �� ������
	/// @return (BOOL) TRUE, FALSE.
	bool Free(DATA** _pData)
	{

		DATA* pData = *_pData;
		int objSize = ALIGN(DATA);
		void* block = ((char*)pData) - (8 + 8 + sizeof(short) + sizeof(bool) + 1);


		int totalsize = 8 + 8 + sizeof(short) + sizeof(bool) + 1 + objSize + sizeof(ST16Long*) + sizeof(short);
		//�������� ������ ���� Ȯ��
		short underSize = UNDERFLOWSHORT(block);
		short overSize = OVERFLOWSHORT(block, totalsize);
		bool isUse = ISUSEBOOL(block);

		if (isUse == false || overSize < 0 || underSize < 0 || underSize != overSize)
		{
			return false;
		}

		//data �ʱ�ȭ
		if (isPlacementNew == true)
		{
			((DATA*)BL_DATA_P(block))->~DATA();
		}
		//��� ���� üũ
		ISUSEBOOL(block) = false;

		ST16Long local_top;
		ST16Long nextBlockCopy; //
#ifdef _TEST
		MemoryLog(GetCurrentThreadId(), "Free", 1, block, nullptr, false, nullptr, 0);
#endif // _TEST
		do
		{
			memset(&local_top, 0, sizeof(ST16Long));
			memset(&nextBlockCopy, 0, sizeof(ST16Long));

			//top ������
			local_top.high_topPtrCount = top.high_topPtrCount;
			local_top.low_topPtr = top.low_topPtr;

			//�����ϴ� �޸𸮿� free �̾� ���̱�
			memcpy(NEXT_BL_P(block, objSize), &top.low_topPtr, 8);
			//NEXT_BL_P(block, objSize)= (char*)top.low_topPtr;

			nextBlockCopy.low_topPtr = (unsigned long long)block;
			nextBlockCopy.high_topPtrCount = top.high_topPtrCount;

		} while (_InterlockedCompareExchange128((long long*)&top, nextBlockCopy.high_topPtrCount, nextBlockCopy.low_topPtr, (long long*)&local_top) == false);
#ifdef _TEST
		MemoryLog(GetCurrentThreadId(), "Free", 2, block, (void*)nextBlockCopy.low_topPtr, false,
			(void*)NEXT_BL_P((void*)nextBlockCopy.low_topPtr, objSize), nextBlockCopy.high_topPtrCount);
#endif // _TEST
		_InterlockedDecrement(&m_iUseCount);
		*_pData = nullptr;

		return true;
	}
	/// @brief ���� Ȯ�� �� �� ������ ��´�. (�޸�Ǯ ������ ��ü ����)
	/// @param  ����.
	/// @return �޸� Ǯ ���� ��ü ����
	unsigned long	GetAllocCount(void)
	{
		return m_iAllocCount;
	}
	/// @brief ���� ������� �� ������ ��´�.
	/// @param  ����.
	/// @return ������� �� ����.
	unsigned long		GetUseCount(void)
	{
		return m_iUseCount;
	}
};

/*

#include "stdafx.h"
#include "CLockFreeStack.h"
#include "CProfiler.h"

#define MAXALLCOCOUNT 100000
#define ALLOCCOUNT 10000
#define THREADCOUNT 10

CProfiler profiler(true);

struct Human
{
public:
	int age = 0;
	char name[256] = { 0 };
	int attack = 0;
	int defence = 0;
	Human()
	{
		age = 24;
		attack = 55;
		defence = 76;
	}

};

CLockFreeStack<Human> *lfs;

unsigned __stdcall ThreadFunc(LPVOID arg)
{
	srand((unsigned int)time(0));
	int rk = rand() % 100;
	Human* arrHuman[ALLOCCOUNT];
	Sleep(1000);

	profiler.ProfileBegin(L"ThreadFunc");

	//�Ҵ�
	for (size_t j=0;j<100;++j)
	{
		for (size_t i = 0; i < ALLOCCOUNT; ++i)
		{
			arrHuman[i] = lfs->Alloc(true);
			arrHuman[i]->age = rk;
		}
		//���
		int check = rk * 2;
		for (size_t i = 0; i < ALLOCCOUNT; ++i)
		{
			if (rk != arrHuman[i]->age)
			{
				CCrashDump::Crash();
			}
			arrHuman[i]->age = arrHuman[i]->age + rk;

			if (check != arrHuman[i]->age)
			{
				CCrashDump::Crash();
			}

		}
		//����
		for (size_t i = 0; i < ALLOCCOUNT; ++i)
		{
			lfs->Free(&arrHuman[i]);
		}

	}

	profiler.ProfileEnd(L"ThreadFunc");
	return 0;
}
void Test()
{
	HANDLE hTestThreads[THREADCOUNT];
	for (int i = 0; i < THREADCOUNT; ++i)
	{
		hTestThreads[i] = (HANDLE)_beginthreadex(0, 0, ThreadFunc, nullptr, 0, nullptr);
		if (hTestThreads[i] == NULL)
		{
			return;
		}
	}

	for (int i = 0; i < THREADCOUNT; ++i)
	{
		WaitForSingleObjectEx(hTestThreads[i], INFINITE, false);
	}
}

int main()
{
	lfs = new CLockFreeStack<Human>(MAXALLCOCOUNT);
	Test();

	profiler.SaveLogIntoFile();
	return 0;
}

*/