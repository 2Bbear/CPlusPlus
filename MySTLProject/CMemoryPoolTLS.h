#pragma once
#include "stdafx.h"
#include "CLockFreeStack.h"

/*
CProfiler profiler(true);

struct Human
{
public:
	int age=0;
	char name[256] = {0};
	int attack=0;
	int defence=0;
	Human()
	{
		age = 24;
		attack = 55;
		defence = 76;
	}

};

#define MAXALLCOCOUNT 200000
#define ALLOCCOUNT 2000
#define THREADCOUNT 100
//CLockFreeMemoryPool<Human> pool(MAXALLCOCOUNT, true, false);
CMemoryPoolTLS<Human> pool(MAXALLCOCOUNT);
unsigned __stdcall ThreadFunc(LPVOID arg)
{
	srand((unsigned int)time(0));
	int rk = rand()%100;

	Human** objArray = new Human * [ALLOCCOUNT];
	memset(objArray, 0, sizeof(Human*) * ALLOCCOUNT);
	Sleep(1000);

	profiler.ProfileBegin(L"ThreadFunc");

	for (int j=0;j<100;++j)
	{
		for (unsigned long i = 0; i < ALLOCCOUNT; i++)
		{
			profiler.ProfileBegin(L"main-Alloc");
			Human* human = pool.Alloc();
			profiler.ProfileEnd(L"main-Alloc");
			if (human == nullptr)
			{
				CCrashDump::Crash();
			}
			objArray[i] = human;
			human->age = i + 1+ rk;
			human->attack = i + 1 + rk;
			human->defence = i + 1 + rk;

			if (human->age != i + 1 + rk || human->attack != i + 1 + rk || human->defence != i + 1 + rk)
			{
				CCrashDump::Crash();
			}
		}

		Sleep(1);
		for (unsigned long i = 0; i < ALLOCCOUNT; i++)
		{
			Human* human = objArray[i];
			human->age += 10;
			human->attack += 10;
			human->defence += 10;

			if (human->age != i + 1 + rk + 10 || human->attack != i + 1 + rk + 10 || human->defence != i + 1 + rk + 10)
			{
				CCrashDump::Crash();
			}
		}

		Sleep(1);
		for (unsigned long i = 0; i < ALLOCCOUNT; i++)
		{
			profiler.ProfileBegin(L"main-Free");
			bool ret = pool.Free(&objArray[i]);
			profiler.ProfileEnd(L"main-Free");
			if (ret == false)
			{
				CCrashDump::Crash();
			}
		}
	}

	profiler.ProfileEnd(L"ThreadFunc");
	return 0;
}
void Test()
{
	profiler.ProfileBegin(L"Test");
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
		WaitForSingleObjectEx(hTestThreads[i],INFINITE,false);
	}
	profiler.ProfileEnd(L"Test");
}
int main()
{
	Test();
	profiler.SaveLogIntoFile();
	return 0;
}

*/
const unsigned long DATAARRSIZE = 10;
template <typename U>
class CMemoryPoolTLS
{
private:
	struct stCFLMP_Node
	{
		void* chunkPointer;//청크 포인터를 넣을 수 있는 공간, 청크 클래스 전방선언을 하자니 템플릿을 앞으로 또 당겨와야해서 귀찮아서 그냥 보이드로 처리함
		U node;
		stCFLMP_Node()
		{
		}
		~stCFLMP_Node()
		{

		}
	};
	/// @brief 노드들은 관리한다.
	class CMemoryChunk
	{
	public:
		stCFLMP_Node* nodePointerArr;
	private:
		//const int DATAARRSIZE=5000;//청크가 갖는 노드 갯수
		unsigned long allockedArrIdx; // 할당 한 노드 갯수
		unsigned long freedArrIdx; // 돌려 받은 노드 갯수
	public:
		CMemoryChunk()
		{
			freedArrIdx = 0;
			allockedArrIdx = 0;
			nodePointerArr = (stCFLMP_Node*)malloc(sizeof(stCFLMP_Node) * DATAARRSIZE);
			if (nodePointerArr==nullptr)
			{
				CCrashDump::Crash();
			}
			//청크 포인터 입력하기하고 초기화 해주기
			for (int idx=0;idx< DATAARRSIZE;++idx)
			{
				nodePointerArr[idx].chunkPointer = this;;
				new(&nodePointerArr[idx])stCFLMP_Node;
			}
		}
		~CMemoryChunk()
		{
			//청크 내부의 모든 노드가 사라질 수 있도록 해야함
			for (int idx = 0; idx < DATAARRSIZE; ++idx)
			{
				nodePointerArr[idx].~stCFLMP_Node();
			}
			free (nodePointerArr);
		}
	public:
		/// @brief 단순히 청크를 초기 상태로 되돌리는 함수 메모리를 다 밀어버리는 것은 아니다.
		/// @return 
		void ChunkReset()
		{
			allockedArrIdx = 0;
			freedArrIdx = 0;
		}
		bool IsAbleAlloc()
		{
			if (allockedArrIdx <= DATAARRSIZE - 1)
			{
				return true;
			}
			return false;
		}
		/// @brief 배열에서 하나 빼서 주기
		/// @return 실패시 nullptr을 반환합니다.
		U* Alloc()
		{
			U* result = nullptr;
			unsigned long idx = _InterlockedIncrement(&allockedArrIdx);
			idx -= 1;//0번째에 접근하기 위해서 -1 을 함
			if (idx >= DATAARRSIZE)
			{
				return nullptr;
			}
			result = (U*)(&nodePointerArr[idx].node);// 강제 형변환을 해줌.

			return result;
		}
		/// @brief 메모리를 사용 끝남 처리 하기
		/// 마지막 메모리를 반환하면 알아서 메모리 풀로 돌아감.
		/// @param _target 
		/// @return 실패시 0 성공시 1 모든 노드 반환시 3을 반환함
		int Free(U* _target)
		{
			bool result = false;
			unsigned long idx = _InterlockedIncrement(&freedArrIdx);
			idx -= 1; // 0번 부터 접근을 하기 위해서 -1을 함
			if (idx >= DATAARRSIZE)
			{
				return 0;
			}
			//마지막 인덱스라면
			if (idx == DATAARRSIZE - 1)
			{
				//메모리 모든 노드 초기화 였지만...굳이? 노드를? 청크 안의 노드는 청크가 사라질때 사라져야 한다/
				//청크 안의 노드들은 어차피 다시 쓰일 것이다.
				allockedArrIdx = 0;
				freedArrIdx = 0;
				// 자기 자신을 파괴하라는 명령을 외부로 보내야함.
				return 3;
			}
			return 1;
		}
	};
private:
	//메모리풀
	CLockFreeStack<CMemoryChunk>* memoryPool = nullptr;
	int tlsIdx = -1;
public:
	CMemoryPoolTLS(unsigned long _baseNodeCount=100, bool _isDestroyAll=false)
	{
		//tls 인덱스 받아오기
		tlsIdx = TlsAlloc(); //스레드 별로 다르게 받아 올 것임
		if (tlsIdx == TLS_OUT_OF_INDEXES)
		{
			CCrashDump::Crash();
		}
		unsigned long chunkCount = (_baseNodeCount / DATAARRSIZE)+1;
		//+1 을 하는 이유 dataarrsize가 baseNodeCount를 나눌때 나머지가 남는 경우 혹은 몫이 0인 경우 시작 청크가
		// 충분히 만들어지지 못하기 때문에 +1을 통해 만들어 주는 것이다.
		//메모리풀 셋팅
		memoryPool = new CLockFreeStack<CMemoryChunk>(chunkCount,false, _isDestroyAll);
		memoryPool->ExternalContructionAll();
	}
	~CMemoryPoolTLS()
	{
		//청크 리스트 비우기
		std::list<CMemoryChunk*>* chunkList = GetChunkList();
		if (chunkList != nullptr)
		{
			typename std::list<CMemoryChunk*>::iterator itr = chunkList->begin();
			//각 청크 삭제하기
			for (; itr != chunkList->end(); ++itr)
			{
				itr = chunkList->erase(itr);
			}
			delete chunkList;
		}
		//메모리 풀 삭제
		delete memoryPool;
	}
private:
	/// @brief tls에 넣어둔 청크 리스트 가져오기
	/// @return 
	std::list<CMemoryChunk*>* GetChunkList()
	{
		std::list<CMemoryChunk*>* p = (std::list<CMemoryChunk*>*)TlsGetValue(tlsIdx);
		if (p == nullptr) // 새로운 청크 리스트를 할당.
		{
			p = new std::list<CMemoryChunk*>();
			TlsSetValue(tlsIdx, p);
		}
		return p;
	}
public:
	U* Alloc(bool isConstructorCall = false, int _repeatCount = 1)
	{
		std::list<CMemoryChunk*>* chunkList = GetChunkList();
		U* result = nullptr;

		// 전체 청크에서 할당 할 수 있는 청크가 바로 나타난다면 바로 청크를 받아온다.
		typename std::list<CMemoryChunk*>::iterator itr = chunkList->begin();
		for (; itr != chunkList->end(); ++itr)
		{
			if (*itr != nullptr)
			{
				if ((*itr)->IsAbleAlloc() == true)
				{
					result = (*itr)->Alloc();
					if (result == nullptr)
					{
						CCrashDump::Crash();
					}
					return result;
				}
			}
		}
		//만약 할당 받아올 청크가 하나도 없다면 메모리풀에 새 청크 달라고 해서 청크를 받아온다
		CMemoryChunk* pNewChunk = memoryPool->Alloc();
		chunkList->push_back(pNewChunk);
		result = pNewChunk->Alloc();
		if (result == nullptr)
		{
			CCrashDump::Crash();
		}

		return result;
	}
	bool Free(U** _pData)
	{
		std::list<CMemoryChunk*>* chunkList = GetChunkList();
		if (chunkList == nullptr)
		{
			CCrashDump::Crash();
			return false;
		}
		U* target = *_pData;
		if (target ==nullptr)
		{
			return false;
		}
		//청크포인터 구하기
		stCFLMP_Node* pNode = (stCFLMP_Node*)(((char*)(*_pData)) - sizeof(CMemoryChunk*));
		CMemoryChunk* pChunk = (CMemoryChunk*)pNode->chunkPointer;
		int freeResult = pChunk->Free(*_pData);
		if (freeResult == 0)
		{
			CCrashDump::Crash();
			return false;
		}
		else if (freeResult == 3) // 청크 반납
		{
			for (typename std::list<CMemoryChunk*>::iterator itr = chunkList->begin(); itr != chunkList->end(); ++itr)
			{
				if (*itr == pChunk)
				{
					(*itr)->ChunkReset();
					//메모리 풀에 반환
					memoryPool->Free(&(*itr));
					
					//리스트에서 제거
					itr = chunkList->erase(itr);
					break;
				}
			}
		}
		return true;
	}
};

