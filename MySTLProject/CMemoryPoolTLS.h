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
		void* chunkPointer;//ûũ �����͸� ���� �� �ִ� ����, ûũ Ŭ���� ���漱���� ���ڴ� ���ø��� ������ �� ��ܿ;��ؼ� �����Ƽ� �׳� ���̵�� ó����
		U node;
		stCFLMP_Node()
		{
		}
		~stCFLMP_Node()
		{

		}
	};
	/// @brief ������ �����Ѵ�.
	class CMemoryChunk
	{
	public:
		stCFLMP_Node* nodePointerArr;
	private:
		//const int DATAARRSIZE=5000;//ûũ�� ���� ��� ����
		unsigned long allockedArrIdx; // �Ҵ� �� ��� ����
		unsigned long freedArrIdx; // ���� ���� ��� ����
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
			//ûũ ������ �Է��ϱ��ϰ� �ʱ�ȭ ���ֱ�
			for (int idx=0;idx< DATAARRSIZE;++idx)
			{
				nodePointerArr[idx].chunkPointer = this;;
				new(&nodePointerArr[idx])stCFLMP_Node;
			}
		}
		~CMemoryChunk()
		{
			//ûũ ������ ��� ��尡 ����� �� �ֵ��� �ؾ���
			for (int idx = 0; idx < DATAARRSIZE; ++idx)
			{
				nodePointerArr[idx].~stCFLMP_Node();
			}
			free (nodePointerArr);
		}
	public:
		/// @brief �ܼ��� ûũ�� �ʱ� ���·� �ǵ����� �Լ� �޸𸮸� �� �о������ ���� �ƴϴ�.
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
		/// @brief �迭���� �ϳ� ���� �ֱ�
		/// @return ���н� nullptr�� ��ȯ�մϴ�.
		U* Alloc()
		{
			U* result = nullptr;
			unsigned long idx = _InterlockedIncrement(&allockedArrIdx);
			idx -= 1;//0��°�� �����ϱ� ���ؼ� -1 �� ��
			if (idx >= DATAARRSIZE)
			{
				return nullptr;
			}
			result = (U*)(&nodePointerArr[idx].node);// ���� ����ȯ�� ����.

			return result;
		}
		/// @brief �޸𸮸� ��� ���� ó�� �ϱ�
		/// ������ �޸𸮸� ��ȯ�ϸ� �˾Ƽ� �޸� Ǯ�� ���ư�.
		/// @param _target 
		/// @return ���н� 0 ������ 1 ��� ��� ��ȯ�� 3�� ��ȯ��
		int Free(U* _target)
		{
			bool result = false;
			unsigned long idx = _InterlockedIncrement(&freedArrIdx);
			idx -= 1; // 0�� ���� ������ �ϱ� ���ؼ� -1�� ��
			if (idx >= DATAARRSIZE)
			{
				return 0;
			}
			//������ �ε������
			if (idx == DATAARRSIZE - 1)
			{
				//�޸� ��� ��� �ʱ�ȭ ������...����? ��带? ûũ ���� ���� ûũ�� ������� ������� �Ѵ�/
				//ûũ ���� ������ ������ �ٽ� ���� ���̴�.
				allockedArrIdx = 0;
				freedArrIdx = 0;
				// �ڱ� �ڽ��� �ı��϶�� ����� �ܺη� ��������.
				return 3;
			}
			return 1;
		}
	};
private:
	//�޸�Ǯ
	CLockFreeStack<CMemoryChunk>* memoryPool = nullptr;
	int tlsIdx = -1;
public:
	CMemoryPoolTLS(unsigned long _baseNodeCount=100, bool _isDestroyAll=false)
	{
		//tls �ε��� �޾ƿ���
		tlsIdx = TlsAlloc(); //������ ���� �ٸ��� �޾� �� ����
		if (tlsIdx == TLS_OUT_OF_INDEXES)
		{
			CCrashDump::Crash();
		}
		unsigned long chunkCount = (_baseNodeCount / DATAARRSIZE)+1;
		//+1 �� �ϴ� ���� dataarrsize�� baseNodeCount�� ������ �������� ���� ��� Ȥ�� ���� 0�� ��� ���� ûũ��
		// ����� ��������� ���ϱ� ������ +1�� ���� ����� �ִ� ���̴�.
		//�޸�Ǯ ����
		memoryPool = new CLockFreeStack<CMemoryChunk>(chunkCount,false, _isDestroyAll);
		memoryPool->ExternalContructionAll();
	}
	~CMemoryPoolTLS()
	{
		//ûũ ����Ʈ ����
		std::list<CMemoryChunk*>* chunkList = GetChunkList();
		if (chunkList != nullptr)
		{
			typename std::list<CMemoryChunk*>::iterator itr = chunkList->begin();
			//�� ûũ �����ϱ�
			for (; itr != chunkList->end(); ++itr)
			{
				itr = chunkList->erase(itr);
			}
			delete chunkList;
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
public:
	U* Alloc(bool isConstructorCall = false, int _repeatCount = 1)
	{
		std::list<CMemoryChunk*>* chunkList = GetChunkList();
		U* result = nullptr;

		// ��ü ûũ���� �Ҵ� �� �� �ִ� ûũ�� �ٷ� ��Ÿ���ٸ� �ٷ� ûũ�� �޾ƿ´�.
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
		//���� �Ҵ� �޾ƿ� ûũ�� �ϳ��� ���ٸ� �޸�Ǯ�� �� ûũ �޶�� �ؼ� ûũ�� �޾ƿ´�
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
		//ûũ������ ���ϱ�
		stCFLMP_Node* pNode = (stCFLMP_Node*)(((char*)(*_pData)) - sizeof(CMemoryChunk*));
		CMemoryChunk* pChunk = (CMemoryChunk*)pNode->chunkPointer;
		int freeResult = pChunk->Free(*_pData);
		if (freeResult == 0)
		{
			CCrashDump::Crash();
			return false;
		}
		else if (freeResult == 3) // ûũ �ݳ�
		{
			for (typename std::list<CMemoryChunk*>::iterator itr = chunkList->begin(); itr != chunkList->end(); ++itr)
			{
				if (*itr == pChunk)
				{
					(*itr)->ChunkReset();
					//�޸� Ǯ�� ��ȯ
					memoryPool->Free(&(*itr));
					
					//����Ʈ���� ����
					itr = chunkList->erase(itr);
					break;
				}
			}
		}
		return true;
	}
};

