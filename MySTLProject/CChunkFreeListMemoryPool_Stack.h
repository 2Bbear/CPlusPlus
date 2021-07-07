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
		int sub;//�ش� �Լ����� ���° ����
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
		void* chunkPointer;//ûũ �����͸� ���� �� �ִ� ����, ûũ Ŭ���� ���漱���� ���ڴ� ���ø��� ������ �� ��ܿ;��ؼ� �����Ƽ� �׳� ���̵�� ó����
		//�е��� �󸶰� ���°ž�?.
		U node;
	};
	//�޸�Ǯ
	CLockFreeStack<stCFLMP_Node> * memoryPool=nullptr;
	int chunkSize;
	int tlsIdx = -1;
private:
	/// @brief ���� �ܺο��� �޸𸮸� �Ҵ� �ϰ�, �����ϴ°͵� �����
	/// stNode �迭�� ���� ����
	/// ûũ ��ü�� ������ �����ϰ� �����ϰ� �� ���̴�. TLS�� �ִ� ChunkList�� �־� ���� ���̱� �����̴�.
	/// �� alloc�� ��� ����ȭ ������ ����. �� free �Ҷ��� �޸� Ǯ�� ���� ���� ���̱� ������ ����ȭ �۾��� ���־�� �Ѵ�.
	/// ûũ�� �Ҵ��� �޾Ƽ� ��带 ���⸸ �� ���̰�, ��Ȱ�� �ϴ� ���� ���� ���� ���̴�!
	class CMemoryChunk
	{
	private:
		CChunkFreeListMemoryPool_Stack* super; // �� Ŭ������ ������ Ŭ���� ������
		const int DATAARRSIZE;//ûũ�� ���� ��� ����
		unsigned long allockedArrIdx ; // �Ҵ� �� ��� ����
		unsigned long freedArrIdx ; // ���� ���� ��� ����
	public:
		stCFLMP_Node** nodePointerArr = nullptr; 
	public:
		CMemoryChunk(int _chunkSize , CChunkFreeListMemoryPool_Stack * _super):DATAARRSIZE(_chunkSize)
		{
			freedArrIdx = 0;
			allockedArrIdx = 0;
			nodePointerArr = (stCFLMP_Node**)malloc(sizeof(stCFLMP_Node*) * _chunkSize);
			super = _super;

			//�迭 ������ �ְ� ���ο��� �Ҵ� �ް� �ϱ�
			if (_super->memoryPool->AllocArr(DATAARRSIZE, nodePointerArr) == false)
			{
				CCrashDump::Crash();
			}
			for (int idx=0;idx< DATAARRSIZE;++idx)
			{
				nodePointerArr[idx]->chunkPointer = this;//������� ûũ �ּ� �־��ֱ�
			}
			//�θ𿡰Լ� ��� �Ҵ� �ޱ�
			//for (int idx=0;idx< DATAARRSIZE;++idx)
			//{
			//	nodePointerArr[idx] = _super->memoryPool->Alloc();
			//	if (nodePointerArr[idx]==nullptr)
			//	{
			//		CCrashDump::Crash();
			//	}
			//	nodePointerArr[idx]->chunkPointer = this;//������� ûũ �ּ� �־��ֱ�
			//}
		}
		~CMemoryChunk()
		{
			//nodePointerArr ����
			if (nodePointerArr !=nullptr)
			{
				for (int idx = 0; idx < DATAARRSIZE; ++idx)
				{
					if (nodePointerArr[idx] != nullptr) // �� �̹� free�� �� ���� ���¶�� �� nullptr�̶� �ݹ� ���� ���̴�.
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
		/// @brief �迭���� �ϳ� ���� �ֱ�
		/// @return ���н� nullptr�� ��ȯ�մϴ�.
		U* Alloc()
		{
			U* result = nullptr;
			unsigned long idx = _InterlockedIncrement(&allockedArrIdx);
			idx -= 1;//0��°�� �����ϱ� ���ؼ� -1 �� ��
			if (idx>= DATAARRSIZE)
			{
				return nullptr;
			}
			result=(U*)(&nodePointerArr[idx]->node);// ���� ����ȯ�� ����.
			
			return result;
		}
		/// @brief �޸𸮸� ��� ���� ó�� �ϱ�
		/// ������ �޸𸮸� ��ȯ�ϸ� �˾Ƽ� �޸� Ǯ�� ���ư�.
		/// @param _target 
		/// @return ���н� 0 ������ 1 ��� ��� ��ȯ�� 3�� ��ȯ��
		int Free(U * _target)
		{
			bool result = false;
			unsigned long idx = _InterlockedIncrement(&freedArrIdx);
			idx -= 1; // 0�� ���� ������ �ϱ� ���ؼ� -1�� ��
			if (idx>=DATAARRSIZE)
			{
				return 0;
			}
			//������ �ε������
			if (idx == DATAARRSIZE-1)
			{
				//�޸� ��� ��� ��ȯ�ϱ�
				for (int idx=0;idx< DATAARRSIZE;++idx)
				{
					super->memoryPool->Free(&nodePointerArr[idx]);
				}
				// �ڱ� �ڽ��� �ı��϶�� ����� �ܺη� ��������.
				return 3;
			}
			return 1;
		}
	};
public:
	/// @brief ������
	/// @param _blockCount 
	/// @param _isPlacementNew 
	/// @param _isDestroyAll 
	CChunkFreeListMemoryPool_Stack(unsigned int _blockCount = 0, unsigned int _chunkSize=100, bool _isPlacementNew = false, bool _isDestroyAll = false)
	{
		//tls �ε��� �޾ƿ���
		tlsIdx = TlsAlloc(); //������ ���� �ٸ��� �޾� �� ����
		if (tlsIdx== TLS_OUT_OF_INDEXES)
		{
			CCrashDump::Crash();
		}
		//�޸�Ǯ ����
		memoryPool = new CLockFreeStack<stCFLMP_Node>(_blockCount, _isPlacementNew, _isDestroyAll);
		//ûũ ����
		chunkSize = _chunkSize;

	}
	~CChunkFreeListMemoryPool_Stack()
	{
		//ûũ ����Ʈ ����
		std::list<CMemoryChunk*>* chunkList = GetChunkList();
		if (chunkList !=nullptr)
		{
			typename std::list<CMemoryChunk*>::iterator itr = chunkList->begin();
			//�� ûũ �����ϱ�
			for (; itr!=chunkList->end();++itr)
			{
				delete *itr;
			}
		}
		//�޸� Ǯ ����
		delete memoryPool;
	}
private:
	/// @brief tls�� �־�� ûũ ����Ʈ ��������
	/// @return 
	std::list<CMemoryChunk*>* GetChunkList()
	{
		std::list<CMemoryChunk*>* p = (std::list<CMemoryChunk*>*)TlsGetValue(tlsIdx);
		if (p == nullptr) // ���ο� ûũ ����Ʈ�� �Ҵ�.
		{
			p = new std::list<CMemoryChunk*>();
			TlsSetValue(tlsIdx, p);
		}
		return p;
	}
	//=================================TLS ûũ���===============================================================
public:
	/// @brief ��ü�� �޸�Ǯ���� �������� �Լ�
	/// @param isConstructorCall 
	/// @param _repeatCount 
	/// @return 
	U* Alloc(bool isConstructorCall = false, int _repeatCount = 1)
	{
		std::list<CMemoryChunk*>* chunkList = GetChunkList();
		U* result = nullptr;
		// ��ü ûũ���� �Ҵ� �� �� �ִ� ûũ�� �ٷ� ��Ÿ���ٸ� �ٷ� ûũ�� �޾ƿ´�.
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
		//���� �Ҵ� �޾ƿ� ûũ�� �ϳ��� ���ٸ� ûũ�� ���� �Ҵ��ؼ� ����Ʈ�� �߰����ش�.
		CMemoryChunk* pNewChunk = new CMemoryChunk(chunkSize,this);
		chunkList->push_back(pNewChunk);
		result = pNewChunk->Alloc();
		if (result == nullptr)
		{
			CCrashDump::Crash();
		}
		return result;
	}
	/// @brief ��ü�� �޸� Ǯ�� ��ȯ�ϴ� �Լ�
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
		//ûũ������ ���ϱ�
		stCFLMP_Node* pNode = (stCFLMP_Node*)(((char*)(*_pData)) - sizeof(CMemoryChunk*));
		CMemoryChunk* pChunk = (CMemoryChunk*)pNode->chunkPointer;
		int freeResult = pChunk->Free(*_pData);
		if (freeResult ==0)
		{
			CCrashDump::Crash();
		}
		else if (freeResult == 3) // ûũ ����
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