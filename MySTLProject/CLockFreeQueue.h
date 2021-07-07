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
#define UNDERFLOWSHORT(block) (*((short*)(((char*)block))))
//����, ��ü ũ�⸦ �ָ� ������ overflowshort�� ���� ���·� ������ּ� �ִ� ��ũ�� ,,�е����� ��� �ؾ���
#define OVERFLOWSHORT(block,blockTotalsize) (*((short*)((char*)block + (blockTotalsize - SHORT))))
//DATA�� ��ġ�� �ٷ� �ִ� ��ũ��
#define BL_DATA_P(block) (((char*)block)+ SHORT + sizeof(bool)+5+sizeof(void*) )
//���� ��带 ��Ÿ���� ��ġ�� ����ִ� ��ũ�� objSize�� �������� ũ��
#define NEXT_BL_P(block,objSize) (((char*)block)+SHORT+ sizeof(bool)+5)
//���� �ָ� �ٷ� ��뿩��bool ���� ���·� ���� �ִ� ��ũ��
#define ISUSEBOOL(block) (*((bool*)(char*)block+SHORT))


/*
��ü���� �������ִ� Ŭ����
�̹� ������� ��ü�� �ٽ� ���ٴ� ������ �޸� ������
�ݵ�� �ִ� ��ü Ÿ���� ¦���� �������� �Ѵ�. �е��� ���� ����� ���� ����.
¦���� �ƴ� ��� �߸��ϸ� ����Ʈ ������ġ�� ��߳� �� �ִ�.
Block ����
[2 size][1 isuse]+[5 padding]+[4 or 8 nextBlockPtr]+[? data]+[2 size]
*/
template <typename DATA>
class CLockFreeQeueu
{
protected:
	/// @brief 
	/// 8 + 8 �ؼ� 16 �������, 16 ���ο� �°� �� ������Ѵ�. 
	/// �̷��� ���� ������ ��輱�� �����Ͱ� ���̰� �Ǿ� cpu���� 2�� ĳ���� �о�� �ϴ� ��Ȳ�� �߻��ϰ�, cpu�� �ٸ� ������� �Ѿ�� �� ���̿� ����ȭ ������ �߻��� �� �ִ�.
	/// @tparam DATA 
	__declspec(align(16))struct ST16Long 
	{
		unsigned long long volatile  low_topPtr;
		unsigned long long  high_topPtrCount;
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
	ST16Long front;
	ST16Long tail;
	bool isPlacementNew;
	list<void*> nodeClearList;
	bool isDestroyAll;
private:
	//���� �� �� ��
	unsigned long m_iAllocCount;
	// ���ǰ� �ִ� �� ��
	unsigned long m_iUseCount;
private:
	//���ø�Ÿ��ũ��
	int objSize = 0;
	//���ũ��
	int nodeSize = 0;

#ifdef _TEST
	const int MEMLOGSIZE = 1000000;
private:
	unsigned long long test_AllocIdx = 0;

	struct MemoryLogData
	{
		DWORD threadID;
		const char* func;
		void* ptr;
		void* insertPtr;
		unsigned long long count;
		int sub;//�ش� �Լ����� ���° ����
		int repeatCount;
		void* subPtr;
	};
	unsigned long long memoryLogIdx = 0;
	unsigned long long memoryLogRepeatCount = 0;
	MemoryLogData* MemoryLogArr;
	void MemoryLog(DWORD _threadID, const char* _func, int _sub,void* _ptr, unsigned long long count, void* insertPtr, void* subPtr)
	{
		unsigned long long idx = 0;
		idx = _InterlockedIncrement(&memoryLogIdx);
		if (idx >= MEMLOGSIZE)
		{
			_InterlockedExchange(&memoryLogIdx, 0);
			_InterlockedIncrement(&memoryLogRepeatCount);
		}
		memset(&MemoryLogArr[idx], 0, sizeof(MemoryLogData));
		MemoryLogArr[idx].threadID = _threadID;
		MemoryLogArr[idx].func = _func;
		MemoryLogArr[idx].sub = _sub;
		MemoryLogArr[idx].repeatCount = (int)memoryLogRepeatCount;
		MemoryLogArr[idx].ptr = _ptr;
		MemoryLogArr[idx].count = count;
		MemoryLogArr[idx].insertPtr = insertPtr;
		MemoryLogArr[idx].subPtr = subPtr;
	}

	bool CheckPtr(void* ptr)
	{
		list<void*>::iterator itr = nodeClearList.begin();
		bool ret = false;
		for (; itr != nodeClearList.end(); ++itr)
		{
			if (*itr == ptr)
			{
				ret = true;
				break;
			}
		}
		
		return ret;
	}
#endif

public:
	CLockFreeQeueu(unsigned int _blockCount = 0, bool _isPlacementNew = false, bool _isDestroyAll = false)
	{
#ifdef _TEST
		MemoryLogArr = new MemoryLogData[MEMLOGSIZE];
#endif // _TEST
		isPlacementNew = _isPlacementNew;
		isDestroyAll = _isDestroyAll;
		//���̳�� ���� ������ ����
		// ���� ��ü�� ������ ��� 
		objSize = ALIGN(DATA);
		//���� ��� ����
		void* block = nullptr;
		nodeSize =sizeof(short) + sizeof(bool) + 5 + sizeof(void*) + objSize  + sizeof(short);
		block = malloc(nodeSize);
		memset(block, 0, nodeSize);
		nodeClearList.push_back(block);
		front.low_topPtr = (unsigned long long)block;
		tail.low_topPtr = (unsigned long long)block;

		for (unsigned int i=0;i< _blockCount;++i)
		{
			//��� ����
			void* node = nullptr;
			node=malloc(nodeSize);
			memset(node, 0, nodeSize);
			nodeClearList.push_back(node);
			Enqueue(node);
			m_iAllocCount++;
		}
	}
	~CLockFreeQeueu()
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
private:
	/// @brief �����ϰ� û�ҵ� ��带 �����̳ʿ� �ִ� �Լ�
	/// @param _node �����ϰ� ����� ���
	bool Enqueue(void * _node)
	{
#ifdef _TEST
		MemoryLog(GetCurrentThreadId(), "Enqueue", 1, nullptr, 0, nullptr,nullptr);
#endif // _TEST
		while (true)
		{
			ST16Long last;
			last.high_topPtrCount = tail.high_topPtrCount;
			last.low_topPtr=tail.low_topPtr;
#ifdef _TEST
			/*if (CheckPtr((void*)last.low_topPtr) == false)
			{
				int a = 10;
			}*/
#endif // _TEST
			void* next = (void*)*((long long*)NEXT_BL_P((char*)last.low_topPtr, objSize));
#ifdef _TEST
			/*if (CheckPtr(next) == false&& next!=nullptr)
			{
				void* next2 = (void*)*((long long*)NEXT_BL_P((char*)last.low_topPtr, objSize));
				int a = 10;
			}*/
#endif // _TEST
			if (last.low_topPtr != tail.low_topPtr)
			{
				printf("EnqHolly molly!");
				continue;
			}
			if (NULL == next)
			{
				if (_InterlockedCompareExchange((unsigned long long *)(NEXT_BL_P((char*)last.low_topPtr, objSize)), (unsigned long long)_node,0)==0)
				{
					ST16Long newTail;
					newTail.high_topPtrCount = last.high_topPtrCount+1;
					newTail.low_topPtr = (unsigned long long)_node;
					_InterlockedCompareExchange128((long long *)&tail, newTail.high_topPtrCount, newTail.low_topPtr, (long long*)&last);
#ifdef _TEST
					MemoryLog(GetCurrentThreadId(), "Enqueue", 2, (void*)last.low_topPtr, 0, (void*)newTail.low_topPtr, next);
#endif // _TEST
					return true;
				}
			}
			else // ��ĭ �̵�
			{
				ST16Long newTail;
				newTail.high_topPtrCount = last.high_topPtrCount + 1;
				newTail.low_topPtr = (unsigned long long)next;
				_InterlockedCompareExchange128((long long*)&tail, newTail.high_topPtrCount, newTail.low_topPtr, (long long*)&last);
#ifdef _TEST
				MemoryLog(GetCurrentThreadId(), "Enqueue", 22, (void*)last.low_topPtr, 0, (void*)newTail.low_topPtr, next);
#endif // _TEST
			}
		}
		return false;
	}

	void* Dequeue()
	{
#ifdef _TEST
		MemoryLog(GetCurrentThreadId(), "Dequeue", 1, nullptr, 0, nullptr,nullptr);
#endif // _TEST
		while (true) 
		{ 
			ST16Long  first;
			first.high_topPtrCount = front.high_topPtrCount;
			first.low_topPtr = front.low_topPtr;
			if (first.low_topPtr != front.low_topPtr)
			{
				continue;
			}
			void* next = (void*)*((long long*)NEXT_BL_P((char*)first.low_topPtr, objSize));


			
			ST16Long last; 
			last.high_topPtrCount = tail.high_topPtrCount;
			last.low_topPtr = tail.low_topPtr;
			if (last.low_topPtr != tail.low_topPtr) // �� �κ� ������ dequeue�� ��Ȱ���� ���� ���̴�.... �̰� ���ٸ� ���� ����������...
			{
				continue;
			}
			void* lastnext = (void*)*((long long*)NEXT_BL_P((char*)last.low_topPtr, objSize));

			
			if (first.low_topPtr == last.low_topPtr) //����Ʈ�� ������ ���� ���.
			{
				if (lastnext ==0) //������ ��尡 ���� ����
				{
					return nullptr;
				}
				else
				{
					ST16Long newTail;
					newTail.high_topPtrCount = last.high_topPtrCount + 1;
					newTail.low_topPtr = (unsigned long long)lastnext;
					_InterlockedCompareExchange128((long long*)&tail, newTail.high_topPtrCount, newTail.low_topPtr, (long long*)&last);
					continue;
				}
			}
			if (next == nullptr)
			{
				continue;//
			}
			ST16Long newFront;
			newFront.high_topPtrCount = first.high_topPtrCount + 1;
			newFront.low_topPtr = (unsigned long long)next;
			if (_InterlockedCompareExchange128((long long*)&front, newFront.high_topPtrCount, newFront.low_topPtr,(long long*)&first)== false)
			{
				continue;
			}

#ifdef _TEST
			MemoryLog(GetCurrentThreadId(), "Dequeue", 2, (void*)first.low_topPtr, 0, (void*)newFront.low_topPtr, nullptr);
#endif // _TEST
			//������ �ʱ�ȭ
			void* targetNode = (void*)first.low_topPtr;
			//�� ��Ʈ
			UNDERFLOWSHORT(targetNode) = objSize;
			//�� ��Ʈ
			OVERFLOWSHORT(targetNode, nodeSize) = objSize;
			//��뿩�� üũ
			ISUSEBOOL(targetNode) = true;
			//���� ������ �ʱ�ȭ
			unsigned long long* data = (unsigned long long*)NEXT_BL_P((char*)targetNode, objSize);
			*data = 0;
			//������ �ʱ�ȭ
			if (isPlacementNew == true)
			{
				new(BL_DATA_P(targetNode))DATA;
			}

			return targetNode;
		}

	}
public:
	/// @brief �� �ϳ��� �Ҵ�޴´�.  
	/// ����� �ϳ� ���� �ްų�, ���� �Ҵ��ؼ� �޴´�.
	/// @param  ����
	/// @return (DATA *) ����Ÿ �� ������
	DATA* Alloc()
	{
		void* node = nullptr;
		while (node==nullptr)
		{
			node = Dequeue();
		}
		_InterlockedIncrement(&m_iUseCount);
		DATA* result = (DATA*)BL_DATA_P(node);
		return result;
	}
	/// @brief ������̴� ���� �����Ѵ�.
	/// @param pData �� ������
	/// @return (BOOL) TRUE, FALSE.
	bool Free(DATA** _pData)
	{
		void* node = ((char*)*_pData) - (sizeof(short) + sizeof(bool) + 5+sizeof(void*));
		//�������� ������ ���� Ȯ��
		short underSize = UNDERFLOWSHORT(node);
		short overSize = OVERFLOWSHORT(node, nodeSize);
		bool isUse = ISUSEBOOL(node);

		if (isUse == false || overSize < 0 || underSize < 0 || underSize != overSize)
		{
			CCrashDump::Crash();
		}
		//data �ʱ�ȭ
		if (isPlacementNew == true)
		{
			((DATA*)BL_DATA_P(node))->~DATA();
		}
		//��� ���� ����
		ISUSEBOOL(node) = false;

		if (ISUSEBOOL(node)==true)
		{
			CCrashDump::Crash();
		}
		Enqueue(node);


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

#pragma region ������ť�׽�Ʈ
struct STTest
{
	int data;
	STTest()
	{
		data = 5555;
	}
};
#define MAXALLCOCOUNT 100000
#define ALLOCCOUNT 10000
#define THREADCOUNT 10
//�޸�Ǯ 10���� ���
CLockFreeQeueu<STTest> pool(MAXALLCOCOUNT, true, false); //10���� ����� �Ҹ��ڿ��� �޸� ��ȯ���� ����.
unsigned __stdcall MomoryPoolTestkThread(LPVOID arg)
{
	DWORD threadID = GetCurrentThreadId();
	STTest** objArray = new STTest * [ALLOCCOUNT];
	memset(objArray, 0, sizeof(STTest*) * ALLOCCOUNT);

	Sleep(3000);
	while (true)
	{
		//10000�� �Ҵ� �ޱ�
		for (int i = 0; i < ALLOCCOUNT; ++i)
		{
			objArray[i] = pool.Alloc();
			objArray[i]->data = threadID;
			if (objArray[i]->data != threadID)
			{
				CCrashDump::Crash();
			}
		}
		Sleep(0);

		//10000���� 1���ϱ�
		for (int i = 0; i < ALLOCCOUNT; ++i)
		{
			_InterlockedIncrement((unsigned long*)&(objArray[i]->data));
			if (objArray[i]->data != threadID + 1)
			{
				CCrashDump::Crash();
			}
		}
		Sleep(0);

		//10000���� 1����
		for (int i = 0; i < ALLOCCOUNT; ++i)
		{
			_InterlockedDecrement((unsigned long*)&(objArray[i]->data));
			if (objArray[i]->data != threadID)
			{
				CCrashDump::Crash();
			}
		}

		Sleep(0);
		//10000�� �����ϱ�
		for (int i = 0; i < ALLOCCOUNT; ++i)
		{
			bool ret = pool.Free(&objArray[i]);
			if (ret == false)
			{
				CCrashDump::Crash();
			}
			if (objArray[i] != nullptr)
			{
				CCrashDump::Crash();
			}
		}
	}
	return 0;

}

void MomoryPoolTest()
{
	CCrashDump dump;
	LogManager::GetInstance().SetlogFileIntervalSec(6000);
	LogManager::GetInstance().SetSaveFile(true);
	// ������ 10�� ����� ������ �����忡�� 1������ ��� alloc�ϱ�
	HANDLE hTestThreads[THREADCOUNT];
	unsigned long thread1AllocCounts[THREADCOUNT] = { 0 };

	for (int i = 0; i < THREADCOUNT; ++i)
	{
		hTestThreads[i] = (HANDLE)_beginthreadex(0, 0, MomoryPoolTestkThread, nullptr, 0, nullptr);
		if (hTestThreads[i] == NULL)
		{
			return;
		}
		CloseHandle(hTestThreads[i]);
	}

	while (true)
	{
		unsigned long  AllocObject = pool.GetAllocCount();
		unsigned long  UseCount = pool.GetUseCount();
		printf("AllocObject %ld  UseCount: %ld \n", AllocObject, UseCount);
		_LOGW(ELogLevel::Level_DEBUG, L"AllocObject %ld  UseCount: %ld \n", AllocObject, UseCount);

		if (AllocObject > MAXALLCOCOUNT)
		{
			_LOGW(ELogLevel::Level_ERROR, L"AllocObject %ld  Over MAXALLCOCOUNT : %d !!!!\n", AllocObject, MAXALLCOCOUNT);
			CCrashDump::Crash();
		}
		if (UseCount > MAXALLCOCOUNT)
		{
			_LOGW(ELogLevel::Level_ERROR, L"UseCount %ld  Over MAXALLCOCOUNT : %d !!!!\n", UseCount, MAXALLCOCOUNT);
			CCrashDump::Crash();
		}
		Sleep(1000);
	}
}

#pragma endregion


*/