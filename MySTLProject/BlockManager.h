#pragma once
#include <new.h>
#include <iostream>
#include <assert.h>
#include <list>

	using namespace std;
#define ALIGNMENT 8
#define ALIGN(p) (((sizeof(p)) + (ALIGNMENT-1)) & ~0x7) 
#define SHORT 2
//블럭을 주면 앞 underflowshort의 변수 형태로 만들어서 주는 매크로
#define UNDERFLOWSHORT(block) (*((short*)(block)))
//블럭과, 전체 크기를 주면 마지막 overflowshort의 변수 형태로 만들어주서 주는 매크로 ,,패딩까지 고려 해야함
#define OVERFLOWSHORT(block,blockTotalsize) (*((short*)((char*)block + (blockTotalsize - SHORT))))
//DATA의 위치를 바로 주는 매크로
#define BL_DATA_P(block) (((char*)block) + SHORT + sizeof(bool)+1 )
//다음 노드를 나타내는 위치를 잡아주는 매크로 objSize는 데이터의 크기
#define NEXT_BL_P(block,objSize) (((char*)block)+SHORT+ sizeof(bool)+1+ objSize)
//블럭을 주면 바로 사용여부bool 변수 형태로 만들어서 주는 매크로
#define ISUSEBOOL(block) (*((bool*)(char*)block+SHORT))

/*
객체들을 관리해주는 클래스
이미 만들어진 객체를 다시 쓴다는 개념의 메모리 관리자
반드시 넣는 객체 타입이 짝수로 떨어져야 한다. 패딩에 대한 고려를 하지 않음.
짝수가 아닌 경우 잘못하면 바이트 정렬위치가 어긋날 수 있다.
Block 구조
[2 size][1 isuse][? data][4 nextBlockPtr][2 size]
*/
template <class DATA>
class BlockManager
{
protected:
	struct st_BLOCK_NODE
	{
		st_BLOCK_NODE()
		{
			stpNextBlock = NULL;
		}
		st_BLOCK_NODE(st_BLOCK_NODE* _ptr)
		{
			stpNextBlock = _ptr;
		}
		st_BLOCK_NODE* stpNextBlock;//4
	};
public:
	BlockManager(unsigned int _blockCount=0,bool _isPlacementNew=false, bool _isDestroyAll=false)
	{
		isPlacementNew = _isPlacementNew;
		isDestroyAll = _isDestroyAll;
		if (_blockCount!=0)
		{
			for (unsigned int i =0;i< _blockCount;++i)
			{
				Alloc();
			}
		}
	}
	~BlockManager()
	{
		int remainedObj = 0;
		int deletedObj = 0;
		list<void*>::iterator itr = nodeClearList.begin();
		for (;itr != nodeClearList.end();)
		{
			if ((*itr)==nullptr)
			{
				printf("Object is remained \n");
				assert(0);
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
				if (check==true)
				{
						
					itr=nodeClearList.erase(itr);
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
		printf_s("Deleted Object : %d Remained Object %d\n",deletedObj, remainedObj);
	}
public:
	st_BLOCK_NODE* freeNode=nullptr;
	bool isPlacementNew;
	list<void*> nodeClearList;
	bool isDestroyAll;
private:
	int m_iAllocCount;
	int m_iUseCount;

public:
	/// @brief 블럭 하나를 할당받는다.  
	/// @param  없음
	/// @return (DATA *) 데이타 블럭 포인터
	DATA* Alloc(void)
	{
		if (freeNode==nullptr)
		{
			// 만들 객체의 사이즈 계산 
			int objSize = ALIGN(DATA);
			//객체 생성
			int totalsize = sizeof(short)+sizeof(bool)+1+ objSize + sizeof(st_BLOCK_NODE*) + sizeof(short);
			void * block = malloc(totalsize);
			nodeClearList.push_back(block);
			//앞 쇼트
			UNDERFLOWSHORT(block) = objSize;
			//뒤 쇼트
			OVERFLOWSHORT(block, totalsize)= objSize;
			//사용여부 체크
			ISUSEBOOL(block) = true;
			//다음 포인터 초기화
			void* data = NEXT_BL_P(block,objSize);
			new(data)st_BLOCK_NODE;
			//데이터 초기화
			if (isPlacementNew==true)
			{
				new(BL_DATA_P(block))DATA;
			}
				
			m_iUseCount++;
			m_iAllocCount++;
			return (DATA*)BL_DATA_P(block);
		}
		else
		{
			int objSize = ALIGN(DATA);

			//객체 생성자 호출
			st_BLOCK_NODE* block = freeNode;
			//상태 체크하기
			//사용 여부 확인
			bool isUse = ISUSEBOOL(block);
			if (isUse==true)
			{
				assert(false);
			}
			st_BLOCK_NODE* temp=nullptr;
			temp = *((st_BLOCK_NODE**)(NEXT_BL_P(block, objSize)));
			freeNode = temp;

			//데이터 초기화
			if (isPlacementNew == true)
			{
				new(BL_DATA_P(block))DATA;
			}
			ISUSEBOOL(block) = true;

			m_iUseCount++;
			return (DATA*)BL_DATA_P(block);
		}
	}
	/// @brief 사용중이던 블럭을 해제한다.
	/// @param pData 블럭 포인터
	/// @return (BOOL) TRUE, FALSE.
	bool Free(DATA* pData)
	{
		int objSize = ALIGN(DATA);
		void* block = ((char*)pData) - (sizeof(short)+sizeof(bool)+1);
		int totalsize = sizeof(short) +sizeof(bool)+1 + objSize + sizeof(st_BLOCK_NODE*) + sizeof(short);
		//정상적인 데이터 인지 확인
		short underSize= UNDERFLOWSHORT(block);
		short overSize = OVERFLOWSHORT(block, totalsize);
		bool isUse = ISUSEBOOL(block);

		if (isUse ==false||overSize < 0 || underSize < 0 || underSize != overSize)
		{
			return false;
		}

		//data 초기화
		if (isPlacementNew==true)
		{
			((DATA*)BL_DATA_P(block))->~DATA();
		}
		//사용 여부 체크
		ISUSEBOOL(block) = false;
		//프리노드에 이어붙이기
		void* data = NEXT_BL_P(block, objSize);
		new(data)st_BLOCK_NODE(freeNode);
		//프리노드로 변경
		freeNode = (st_BLOCK_NODE*)block;

		m_iUseCount--;

		return true;
	}
	/// @brief 현재 확보 된 블럭 개수를 얻는다. (메모리풀 내부의 전체 개수)
	/// @param  없음.
	/// @return 메모리 풀 내부 전체 개수
	int	GetAllocCount(void)
	{
		return m_iAllocCount;
	}
	/// @brief 현재 사용중인 블럭 개수를 얻는다.
	/// @param  없음.
	/// @return 사용중인 블럭 개수.
	int		GetUseCount(void)
	{
		return m_iUseCount;
	}
};

