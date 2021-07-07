#pragma once
#include "stdafx.h"
#include "CLockFreeStack.h"




template <typename U>
class CChunkFreeListMemoryPool_Stack
{

#ifdef _TEST
	const int MEMLOGSIZE = 1000000;
private:
	unsigned long long test_AllocIdx = 0;

	struct MemoryLogData
	{
		DWORD threadID;
		const char* func;
		int sub;//해당 함수에서 몇번째 인지
		int repeatCount;



	};
	unsigned long long memoryLogIdx = 0;
	unsigned long long memoryLogRepeatCount = 0;
	MemoryLogData* MemoryLogArr;
	void MemoryLog(DWORD _threadID, const char* _func, int _sub)
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


	}
#endif

private:
	struct stCFLMP_Node
	{
		void* chunkPointer;//청크 포인터를 넣을 수 있는 공간, 청크 클래스 전방선언을 하자니 템플릿을 앞으로 또 당겨와야해서 귀찮아서 그냥 보이드로 처리함
		//패딩이 얼마가 들어가는거야?.
		U node;
	};
	//메모리풀
	CLockFreeStack<stCFLMP_Node> * memoryPool=nullptr;
	int chunkSize;
	int tlsIdx = -1;
private:
	/// @brief 실제 외부에서 메모리를 할당 하고, 프리하는것도 담당함
	/// stNode 배열을 갖고 있음
	/// 청크 자체는 스레드 고유하게 존재하게 될 것이다. TLS에 있는 ChunkList에 넣어 놓을 것이기 때문이다.
	/// 즉 alloc의 경우 동기화 걱정이 없다. 단 free 할때는 메모리 풀에 돌려 놓는 것이기 때문에 동기화 작업을 해주어야 한다.
	/// 청크는 할당을 받아서 노드를 빼기만 할 것이고, 재활용 하는 일은 하지 않을 것이다!
	class CMemoryChunk
	{
	private:
		CChunkFreeListMemoryPool_Stack* super; // 이 클래스를 소유한 클래스 포인터
		const int DATAARRSIZE;//청크가 갖는 노드 갯수
		unsigned long allockedArrIdx ; // 할당 한 노드 갯수
		unsigned long freedArrIdx ; // 돌려 받은 노드 갯수
	public:
		stCFLMP_Node** nodePointerArr = nullptr; 
	public:
		CMemoryChunk(int _chunkSize , CChunkFreeListMemoryPool_Stack * _super):DATAARRSIZE(_chunkSize)
		{
			freedArrIdx = 0;
			allockedArrIdx = 0;
			nodePointerArr = (stCFLMP_Node**)malloc(sizeof(stCFLMP_Node*) * _chunkSize);
			super = _super;

			//배열 포인터 주고 내부에서 할당 받게 하기
			if (_super->memoryPool->AllocArr(DATAARRSIZE, nodePointerArr) == false)
			{
				CCrashDump::Crash();
			}
			for (int idx=0;idx< DATAARRSIZE;++idx)
			{
				nodePointerArr[idx]->chunkPointer = this;//만들어진 청크 주소 넣어주기
			}
			//부모에게서 노드 할당 받기
			//for (int idx=0;idx< DATAARRSIZE;++idx)
			//{
			//	nodePointerArr[idx] = _super->memoryPool->Alloc();
			//	if (nodePointerArr[idx]==nullptr)
			//	{
			//		CCrashDump::Crash();
			//	}
			//	nodePointerArr[idx]->chunkPointer = this;//만들어진 청크 주소 넣어주기
			//}
		}
		~CMemoryChunk()
		{
			//nodePointerArr 해제
			if (nodePointerArr !=nullptr)
			{
				for (int idx = 0; idx < DATAARRSIZE; ++idx)
				{
					if (nodePointerArr[idx] != nullptr) // 뭐 이미 free를 다 해준 상태라면 다 nullptr이라서 금방 끝날 것이다.
					{
						super->memoryPool->Free(&nodePointerArr[idx]);
					}
				}
				free(nodePointerArr);
			}
		}
	
	public:
		bool IsAbleAlloc()
		{
			if (allockedArrIdx <=DATAARRSIZE-1)
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
			if (idx>= DATAARRSIZE)
			{
				return nullptr;
			}
			result=(U*)(&nodePointerArr[idx]->node);// 강제 형변환을 해줌.
			
			return result;
		}
		/// @brief 메모리를 사용 끝남 처리 하기
		/// 마지막 메모리를 반환하면 알아서 메모리 풀로 돌아감.
		/// @param _target 
		/// @return 실패시 0 성공시 1 모든 노드 반환시 3을 반환함
		int Free(U * _target)
		{
			bool result = false;
			unsigned long idx = _InterlockedIncrement(&freedArrIdx);
			idx -= 1; // 0번 부터 접근을 하기 위해서 -1을 함
			if (idx>=DATAARRSIZE)
			{
				return 0;
			}
			//마지막 인덱스라면
			if (idx == DATAARRSIZE-1)
			{
				//메모리 모든 노드 반환하기
				for (int idx=0;idx< DATAARRSIZE;++idx)
				{
					super->memoryPool->Free(&nodePointerArr[idx]);
				}
				// 자기 자신을 파괴하라는 명령을 외부로 보내야함.
				return 3;
			}
			return 1;
		}
	};
public:
	/// @brief 생성자
	/// @param _blockCount 
	/// @param _isPlacementNew 
	/// @param _isDestroyAll 
	CChunkFreeListMemoryPool_Stack(unsigned int _blockCount = 0, unsigned int _chunkSize=100, bool _isPlacementNew = false, bool _isDestroyAll = false)
	{
		//tls 인덱스 받아오기
		tlsIdx = TlsAlloc(); //스레드 별로 다르게 받아 올 것임
		if (tlsIdx== TLS_OUT_OF_INDEXES)
		{
			CCrashDump::Crash();
		}
		//메모리풀 셋팅
		memoryPool = new CLockFreeStack<stCFLMP_Node>(_blockCount, _isPlacementNew, _isDestroyAll);
		//청크 셋팅
		chunkSize = _chunkSize;

	}
	~CChunkFreeListMemoryPool_Stack()
	{
		//청크 리스트 비우기
		std::list<CMemoryChunk*>* chunkList = GetChunkList();
		if (chunkList !=nullptr)
		{
			typename std::list<CMemoryChunk*>::iterator itr = chunkList->begin();
			//각 청크 삭제하기
			for (; itr!=chunkList->end();++itr)
			{
				delete *itr;
			}
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
	//=================================TLS 청크기능===============================================================
public:
	/// @brief 객체를 메모리풀에서 가져오는 함수
	/// @param isConstructorCall 
	/// @param _repeatCount 
	/// @return 
	U* Alloc(bool isConstructorCall = false, int _repeatCount = 1)
	{
		std::list<CMemoryChunk*>* chunkList = GetChunkList();
		U* result = nullptr;
		// 전체 청크에서 할당 할 수 있는 청크가 바로 나타난다면 바로 청크를 받아온다.
		typename std::list<CMemoryChunk*>::iterator itr= chunkList->begin();
		for (;itr!= chunkList->end();++itr)
		{
			if (*itr!=nullptr)
			{
				if ((*itr)->IsAbleAlloc()==true)
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
		//만약 할당 받아올 청크가 하나도 없다면 청크를 새로 할당해서 리스트에 추가해준다.
		CMemoryChunk* pNewChunk = new CMemoryChunk(chunkSize,this);
		chunkList->push_back(pNewChunk);
		result = pNewChunk->Alloc();
		if (result == nullptr)
		{
			CCrashDump::Crash();
		}
		return result;
	}
	/// @brief 객체를 메모리 풀에 반환하는 함수
	/// @param _pData 
	/// @return 
	bool Free(U** _pData)
	{
		std::list<CMemoryChunk*>* chunkList = GetChunkList();
		if (chunkList ==nullptr)
		{
			CCrashDump::Crash();
			return false;
		}
		//청크포인터 구하기
		stCFLMP_Node* pNode = (stCFLMP_Node*)(((char*)(*_pData)) - sizeof(CMemoryChunk*));
		CMemoryChunk* pChunk = (CMemoryChunk*)pNode->chunkPointer;
		int freeResult = pChunk->Free(*_pData);
		if (freeResult ==0)
		{
			CCrashDump::Crash();
		}
		else if (freeResult == 3) // 청크 해제
		{
			for (typename std::list<CMemoryChunk*>::iterator itr = chunkList->begin();itr!= chunkList->end();++itr)
			{
				if (*itr==nullptr)
				{
					int a = 10;
				}
				if (*itr== pChunk)
				{
					delete pChunk;
					itr=chunkList->erase(itr);
				}
			}
		}
		return true;
	}
};