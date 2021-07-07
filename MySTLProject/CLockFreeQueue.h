#pragma once

#include <new.h>
#include <iostream>
#include <list>
#include "CCrashDump.h"
using namespace std;
#define ALIGNMENT 8
#define ALIGN(p) (((sizeof(p)) + (ALIGNMENT-1)) & ~0x7) 
#define SHORT 2
//블럭을 주면 맨앞 현재블럭 포인터의 변수를 주는 매크로
#define SELFPOINTER(block) (*((unsigned long long*)((((char*)block)))))
//블럭을 주면 앞 underflowshort의 변수 형태로 만들어서 주는 매크로
#define UNDERFLOWSHORT(block) (*((short*)(((char*)block))))
//블럭과, 전체 크기를 주면 마지막 overflowshort의 변수 형태로 만들어주서 주는 매크로 ,,패딩까지 고려 해야함
#define OVERFLOWSHORT(block,blockTotalsize) (*((short*)((char*)block + (blockTotalsize - SHORT))))
//DATA의 위치를 바로 주는 매크로
#define BL_DATA_P(block) (((char*)block)+ SHORT + sizeof(bool)+5+sizeof(void*) )
//다음 노드를 나타내는 위치를 잡아주는 매크로 objSize는 데이터의 크기
#define NEXT_BL_P(block,objSize) (((char*)block)+SHORT+ sizeof(bool)+5)
//블럭을 주면 바로 사용여부bool 변수 형태로 만들어서 주는 매크로
#define ISUSEBOOL(block) (*((bool*)(char*)block+SHORT))


/*
객체들을 관리해주는 클래스
이미 만들어진 객체를 다시 쓴다는 개념의 메모리 관리자
반드시 넣는 객체 타입이 짝수로 떨어져야 한다. 패딩에 대한 고려를 하지 않음.
짝수가 아닌 경우 잘못하면 바이트 정렬위치가 어긋날 수 있다.
Block 구조
[2 size][1 isuse]+[5 padding]+[4 or 8 nextBlockPtr]+[? data]+[2 size]
*/
template <typename DATA>
class CLockFreeQeueu
{
protected:
	/// @brief 
	/// 8 + 8 해서 16 사이즈니, 16 라인에 맞게 끔 해줘야한다. 
	/// 이렇게 하지 않으면 경계선에 데이터가 놓이게 되어 cpu에서 2번 캐쉬를 읽어야 하는 상황이 발생하고, cpu가 다른 스레드로 넘어가면 그 사이에 동기화 문제가 발생할 수 있다.
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
	//생성 된 블럭 수
	unsigned long m_iAllocCount;
	// 사용되고 있는 블럭 수
	unsigned long m_iUseCount;
private:
	//템플릿타입크기
	int objSize = 0;
	//노드크기
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
		int sub;//해당 함수에서 몇번째 인지
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
		//더미노드 만들어서 연결해 놓기
		// 만들 객체의 사이즈 계산 
		objSize = ALIGN(DATA);
		//더미 노드 생성
		void* block = nullptr;
		nodeSize =sizeof(short) + sizeof(bool) + 5 + sizeof(void*) + objSize  + sizeof(short);
		block = malloc(nodeSize);
		memset(block, 0, nodeSize);
		nodeClearList.push_back(block);
		front.low_topPtr = (unsigned long long)block;
		tail.low_topPtr = (unsigned long long)block;

		for (unsigned int i=0;i< _blockCount;++i)
		{
			//노드 생성
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
				//사용 여부 확인하고 전부 지워버리기
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
	/// @brief 깨끗하게 청소된 노드를 컨테이너에 넣는 함수
	/// @param _node 깨끗하게 비워진 노드
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
			else // 한칸 이동
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
			if (last.low_topPtr != tail.low_topPtr) // 이 부분 때문에 dequeue가 원활하지 않을 것이다.... 이게 없다면 좀더 빠르겠지만...
			{
				continue;
			}
			void* lastnext = (void*)*((long long*)NEXT_BL_P((char*)last.low_topPtr, objSize));

			
			if (first.low_topPtr == last.low_topPtr) //프론트와 테일이 같은 경우.
			{
				if (lastnext ==0) //내어줄 노드가 없는 것임
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
			//생성자 초기화
			void* targetNode = (void*)first.low_topPtr;
			//앞 쇼트
			UNDERFLOWSHORT(targetNode) = objSize;
			//뒤 쇼트
			OVERFLOWSHORT(targetNode, nodeSize) = objSize;
			//사용여부 체크
			ISUSEBOOL(targetNode) = true;
			//다음 포인터 초기화
			unsigned long long* data = (unsigned long long*)NEXT_BL_P((char*)targetNode, objSize);
			*data = 0;
			//데이터 초기화
			if (isPlacementNew == true)
			{
				new(BL_DATA_P(targetNode))DATA;
			}

			return targetNode;
		}

	}
public:
	/// @brief 블럭 하나를 할당받는다.  
	/// 블록을 하나 빼서 받거나, 새로 할당해서 받는다.
	/// @param  없음
	/// @return (DATA *) 데이타 블럭 포인터
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
	/// @brief 사용중이던 블럭을 해제한다.
	/// @param pData 블럭 포인터
	/// @return (BOOL) TRUE, FALSE.
	bool Free(DATA** _pData)
	{
		void* node = ((char*)*_pData) - (sizeof(short) + sizeof(bool) + 5+sizeof(void*));
		//정상적인 데이터 인지 확인
		short underSize = UNDERFLOWSHORT(node);
		short overSize = OVERFLOWSHORT(node, nodeSize);
		bool isUse = ISUSEBOOL(node);

		if (isUse == false || overSize < 0 || underSize < 0 || underSize != overSize)
		{
			CCrashDump::Crash();
		}
		//data 초기화
		if (isPlacementNew == true)
		{
			((DATA*)BL_DATA_P(node))->~DATA();
		}
		//사용 여부 변경
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
	/// @brief 현재 확보 된 블럭 개수를 얻는다. (메모리풀 내부의 전체 개수)
	/// @param  없음.
	/// @return 메모리 풀 내부 전체 개수
	unsigned long	GetAllocCount(void)
	{
		return m_iAllocCount;
	}
	/// @brief 현재 사용중인 블럭 개수를 얻는다.
	/// @param  없음.
	/// @return 사용중인 블럭 개수.
	unsigned long		GetUseCount(void)
	{
		return m_iUseCount;
	}
};



/*

#pragma region 락프리큐테스트
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
//메모리풀 10만개 잡기
CLockFreeQeueu<STTest> pool(MAXALLCOCOUNT, true, false); //10만개 만들고 소멸자에서 메모리 반환하지 않음.
unsigned __stdcall MomoryPoolTestkThread(LPVOID arg)
{
	DWORD threadID = GetCurrentThreadId();
	STTest** objArray = new STTest * [ALLOCCOUNT];
	memset(objArray, 0, sizeof(STTest*) * ALLOCCOUNT);

	Sleep(3000);
	while (true)
	{
		//10000개 할당 받기
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

		//10000개에 1더하기
		for (int i = 0; i < ALLOCCOUNT; ++i)
		{
			_InterlockedIncrement((unsigned long*)&(objArray[i]->data));
			if (objArray[i]->data != threadID + 1)
			{
				CCrashDump::Crash();
			}
		}
		Sleep(0);

		//10000개에 1빼기
		for (int i = 0; i < ALLOCCOUNT; ++i)
		{
			_InterlockedDecrement((unsigned long*)&(objArray[i]->data));
			if (objArray[i]->data != threadID)
			{
				CCrashDump::Crash();
			}
		}

		Sleep(0);
		//10000개 해제하기
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
	// 스레드 10개 만들기 각각의 스레드에서 1만개의 노드 alloc하기
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