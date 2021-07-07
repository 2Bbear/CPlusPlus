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
#define UNDERFLOWSHORT(block) (*((short*)(((char*)block)+16)))
//블럭과, 전체 크기를 주면 마지막 overflowshort의 변수 형태로 만들어주서 주는 매크로 ,,패딩까지 고려 해야함
#define OVERFLOWSHORT(block,blockTotalsize) (*((short*)((char*)block + (blockTotalsize - SHORT))))
//사용 카운트바로 주는 매크로
#define USECOUNTINT(block) (*((int*)((((char*)block)+8))))
//DATA의 위치를 바로 주는 매크로
#define BL_DATA_P(block) (((char*)block)+16 + SHORT + sizeof(bool)+1 )
//다음 노드를 나타내는 위치를 잡아주는 매크로 objSize는 데이터의 크기
#define NEXT_BL_P(block,objSize) (((char*)block)+SHORT+16+ sizeof(bool)+1+ objSize)
//블럭을 주면 바로 사용여부bool 변수 형태로 만들어서 주는 매크로
#define ISUSEBOOL(block) (*((bool*)(char*)block+16+SHORT))


/*
객체들을 관리해주는 클래스
이미 만들어진 객체를 다시 쓴다는 개념의 메모리 관리자
반드시 넣는 객체 타입이 짝수로 떨어져야 한다. 패딩에 대한 고려를 하지 않음.
짝수가 아닌 경우 잘못하면 바이트 정렬위치가 어긋날 수 있다.
Block 구조
[2 size][1 isuse]+[1 padding]+[4 useCount ]+[? data][4 or 8 nextBlockPtr][2 size]
[8 현재블럭포인터] + [8 사용카운트] + [2 size][1 isuse]+[1 padding]+[? data][4 or 8 nextBlockPtr][2 size]
*/
template <typename DATA>
class CLockFreeMemoryPool 
{
protected:
	__declspec(align(64))struct ST16Long
	{
		unsigned long long  low_topPtr;
		unsigned long long  high_topPtrCount;
		ST16Long()
		{
			low_topPtr = 0;
			high_topPtrCount = 0;
		}
		ST16Long(ST16Long * _ptr)
		{
			if (_ptr==nullptr)
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
	//생성 된 블럭 수
	unsigned long m_iAllocCount;
	// 사용되고 있는 블럭 수
	unsigned long m_iUseCount;
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
		int sub;//해당 함수에서 몇번째 인지
		int repeatCount;
		bool interlockCompareResult;
		const char* func;

	};
	unsigned long long memoryLogIdx = 0;
	unsigned long long memoryLogRepeatCount=0;
	MemoryLogData * MemoryLogArr;
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
		//MemoryLogArr[idx].interlockCompareResult = _interlockCompareResult;
		MemoryLogArr[idx].topPtr = _topPtr;
		MemoryLogArr[idx].topPtrNext = _topPtrNext;
	}
#endif
	
public:
	CLockFreeMemoryPool(unsigned int _blockCount = 0, bool _isPlacementNew = false, bool _isDestroyAll = false)
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
	~CLockFreeMemoryPool()
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

public:
	/// @brief 블럭 하나를 할당받는다.  
	/// @param  없음
	/// @return (DATA *) 데이타 블럭 포인터
	DATA* Alloc(bool isConstructorCall = false, int _repeatCount = 1)
	{
#ifdef _TEST
		_InterlockedExchange(&test_AllocIdx, 0);
#endif // _TEST
		
		
		if (top.low_topPtr == 0)
		{
#ifdef _TEST
			MemoryLog(GetCurrentThreadId(), "Alloc", 1, nullptr, nullptr, false,nullptr,0);
#endif // _TEST
			void* block = nullptr;
			for (int i=0;i<_repeatCount;++i)
			{
#ifdef _TEST
				_InterlockedIncrement(&test_AllocIdx);
#endif // _TEST
				
				// 만들 객체의 사이즈 계산 
				int objSize = ALIGN(DATA);
				//객체 생성
				int totalsize = 8+8+sizeof(short) + sizeof(bool) + 1 + objSize + sizeof(ST16Long*) + sizeof(short);
				block = malloc(totalsize);
				nodeClearList.push_back(block);
				//앞 ST16Long 초기화
				new(block)ST16Long;
				//셀프 포인터 
				SELFPOINTER(block) = (unsigned long long)block;
				
				//앞 쇼트
				UNDERFLOWSHORT(block) = objSize;
				//뒤 쇼트
				OVERFLOWSHORT(block, totalsize) = objSize;
				//사용여부 체크
				ISUSEBOOL(block) = true;
				//다음 포인터 초기화
				void* data = NEXT_BL_P(block, objSize);
				memset(data, 0, sizeof(void*));
				//new(data)ST16Long;
				//데이터 초기화
				if (isPlacementNew == true)
				{
					new(BL_DATA_P(block))DATA;
				}
				//생성자에서 호출한 경우
				if (isConstructorCall == true)
				{
					//사용여부 체크
					ISUSEBOOL(block) = false;
					//카운터
					USECOUNTINT(block) = 0;
					if (top.low_topPtr == 0)
					{ 
						top.low_topPtr = (unsigned long long)block;
					}
					else
					{
						memcpy(data, (void*)&top.low_topPtr, sizeof(void*));//노드 이어붙이기
						top.low_topPtr = (unsigned long long)block;
					}
				}
				else
				{
					top.high_topPtrCount += 1;
					_InterlockedIncrement(&m_iUseCount);
				}
				_InterlockedIncrement(&m_iAllocCount);
#ifdef _TEST
				//만약 
				//정상적인 데이터 인지 확인
				short underSize = UNDERFLOWSHORT(block);
				short overSize = OVERFLOWSHORT(block, totalsize);
				bool isUse = ISUSEBOOL(block);
				if (isUse == true || overSize < 0 || underSize < 0 || underSize != overSize)
				{
					printf("생성에러%lld\n", test_AllocIdx);
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
			MemoryLog(GetCurrentThreadId(), "Alloc", 2, nullptr, nullptr, false,nullptr,0);
#endif // _TEST
			ST16Long nextBlockCopy; //
			ST16Long local_top;
			ST16Long* block = nullptr; // 실제 스택리스트에서 튀어나가는 녀석
			int objSize = ALIGN(DATA);
			bool interlockResult = false;
			do
			{
				memset(&local_top, 0, sizeof(ST16Long));
				memset(&nextBlockCopy, 0, sizeof(ST16Long));
				//top 스탭샷
				local_top.high_topPtrCount = top.high_topPtrCount;
				local_top.low_topPtr = top.low_topPtr;

				block = nullptr;
				block = (ST16Long*)local_top.low_topPtr;
				
				if (block==nullptr)
				{
					continue;
				}
				
				ST16Long*  temp = (ST16Long*)(*((ST16Long**)(NEXT_BL_P(block, objSize)))); // 다음 노드가 있다면 가져와서 
				if (temp == nullptr)
				{
					nextBlockCopy.low_topPtr = 0;
					nextBlockCopy.high_topPtrCount = local_top.high_topPtrCount+1;
				}
				else
				{
					nextBlockCopy.low_topPtr = temp->low_topPtr;
					nextBlockCopy.high_topPtrCount = local_top.high_topPtrCount+1;
				}
				//만약 치환 될 top ptr와 밖으로 나온 블럭의 ptr이 같다면?...이건 문제가 된다.
				if (block == (void*)nextBlockCopy.low_topPtr)
				{
					CCrashDump::Crash();
				}
				interlockResult = _InterlockedCompareExchange128((long long*)&top, nextBlockCopy.high_topPtrCount, nextBlockCopy.low_topPtr, (long long*)&local_top);
			} while (interlockResult ==false);
#ifdef _TEST
			MemoryLog(GetCurrentThreadId(), "Alloc", 3, block, (void*)nextBlockCopy.low_topPtr,
				interlockResult, (void*)NEXT_BL_P((void*)nextBlockCopy.low_topPtr, objSize), nextBlockCopy.high_topPtrCount);
			if (top.high_topPtrCount<=0)
			{
				CCrashDump::Crash();
			}
#endif // _TEST

			if (block==nullptr)
			{
				CCrashDump::Crash();
				return nullptr;
			}
			//상태 체크하기
			//사용 여부 확인
			bool isUse = ISUSEBOOL(block);
			if (isUse == true)
			{
				CCrashDump::Crash();
			}
			//데이터 초기화
			if (isPlacementNew == true)
			{
				new(BL_DATA_P(block))DATA;
			}
			//사용 여부 체크
			ISUSEBOOL(block) = true;
			//사용 카운트 증가 체크
			USECOUNTINT(block) = USECOUNTINT(block) + 1;
			_InterlockedIncrement(&m_iUseCount);

#ifdef _TEST
			//만약 
			int totalsize = 8 + 8 + sizeof(short) + sizeof(bool) + 1 + objSize + sizeof(ST16Long*) + sizeof(short);
			//정상적인 데이터 인지 확인
			short underSize = UNDERFLOWSHORT(block);
			short overSize = OVERFLOWSHORT(block, totalsize);
			isUse = ISUSEBOOL(block);
			if (isUse == false || overSize < 0 || underSize < 0 || underSize != overSize)
			{
				printf("재할당에러 %lld\n", test_AllocIdx);
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
	/// @brief 사용중이던 블럭을 해제한다.
	/// @param pData 블럭 포인터
	/// @return (BOOL) TRUE, FALSE.
	bool Free(DATA** _pData)
	{
		
		DATA* pData = *_pData;
		int objSize = ALIGN(DATA);
		void* block = ((char*)pData) - (8 + 8 + sizeof(short) + sizeof(bool) + 1);


		int totalsize = 8 + 8 + sizeof(short) + sizeof(bool) + 1 + objSize + sizeof(ST16Long*) + sizeof(short);
		//정상적인 데이터 인지 확인
		short underSize = UNDERFLOWSHORT(block);
		short overSize = OVERFLOWSHORT(block, totalsize);
		bool isUse = ISUSEBOOL(block);

		if (isUse == false || overSize < 0 || underSize < 0 || underSize != overSize)
		{
			return false;
		}

		//data 초기화
		if (isPlacementNew == true)
		{
			((DATA*)BL_DATA_P(block))->~DATA();
		}
		//사용 여부 체크
		ISUSEBOOL(block) = false;

		ST16Long local_top;
		ST16Long nextBlockCopy; //
#ifdef _TEST
		MemoryLog(GetCurrentThreadId(), "Free", 1, block, nullptr, false,nullptr,0);
#endif // _TEST
		do
		{
			memset(&local_top, 0, sizeof(ST16Long));
			memset(&nextBlockCopy, 0, sizeof(ST16Long));

			//top 스냅샷
			local_top.high_topPtrCount= top.high_topPtrCount;
			local_top.low_topPtr =top.low_topPtr;

			//해제하는 메모리에 free 이어 붙이기
			memcpy(NEXT_BL_P(block, objSize), &top.low_topPtr, 8);
			//NEXT_BL_P(block, objSize)= (char*)top.low_topPtr;

			nextBlockCopy.low_topPtr = (unsigned long long)block;
			nextBlockCopy.high_topPtrCount = top.high_topPtrCount;

		} while (_InterlockedCompareExchange128((long long*)&top, nextBlockCopy.high_topPtrCount, nextBlockCopy.low_topPtr, (long long*)&local_top) == false);
#ifdef _TEST
		MemoryLog(GetCurrentThreadId(), "Free", 2, block,(void*)nextBlockCopy.low_topPtr, false,
			(void*)NEXT_BL_P((void*)nextBlockCopy.low_topPtr, objSize), nextBlockCopy.high_topPtrCount);
#endif // _TEST
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
#pragma region 락프리스택 큐 _테스트

struct STTest
{
	int data;
	STTest()
	{
		data = 5555;
	}
};
void MomoryPoolBasicTest()
{
	//메모리풀 10만개 잡기
	CLockFreeMemoryPool<STTest> pool(3, true, false); //10만개 만들고 소멸자에서 메모리 반환하지 않음.
	// 스레드 10개 만들기 각각의 스레드에서 1만개의 노드 alloc하기
	STTest* obj1 = pool.Alloc();
	obj1->data = 111;
	STTest* obj2 = pool.Alloc();
	obj2->data = 222;
	STTest* obj3 = pool.Alloc();
	obj3->data = 333;
	STTest* obj4 = pool.Alloc();
	obj4->data = 444;

	int allcoCount = pool.GetAllocCount();
	int useCount = pool.GetUseCount();

	pool.Free(&obj4);
	pool.Free(&obj3);

	STTest* obj5 = pool.Alloc();
	obj5->data = 444;


	allcoCount = pool.GetAllocCount();
	useCount = pool.GetUseCount();
}
#define MAXALLCOCOUNT 100000
#define ALLOCCOUNT 10000
#define THREADCOUNT 10
//메모리풀 10만개 잡기
CLockFreeStack<STTest> pool(MAXALLCOCOUNT, true, false); //10만개 만들고 소멸자에서 메모리 반환하지 않음.
unsigned __stdcall MomoryPoolTestkThread(LPVOID arg)
{
	DWORD threadID = GetCurrentThreadId();
	STTest** objArray=new STTest*[ALLOCCOUNT];
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
		Sleep(1);

		//10000개에 1더하기
		for (int i = 0; i < ALLOCCOUNT; ++i)
		{
			_InterlockedIncrement((unsigned long*)&(objArray[i]->data));
			if (objArray[i]->data != threadID+1)
			{
				CCrashDump::Crash();
			}
		}
		Sleep(1);

		//10000개에 1빼기
		for (int i = 0; i < ALLOCCOUNT; ++i)
		{
			_InterlockedDecrement((unsigned long*)&(objArray[i]->data));
			if (objArray[i]->data != threadID)
			{
				CCrashDump::Crash();
			}
		}

		Sleep(1);
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