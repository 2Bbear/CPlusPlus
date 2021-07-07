#pragma once
#include <new.h>
#include <iostream>

namespace twobbearlib
{


	/*---------------------------------------------------------------

	procademy MemoryPool.

	메모리 풀 클래스 (오브젝트 풀 / 프리리스트)
	특정 데이타(구조체,클래스,변수)를 일정량 할당 후 나눠쓴다.

	- 사용법.

	procademy::CMemoryPool<DATA> MemPool(300, FALSE);
	DATA *pData = MemPool.Alloc();

	pData 사용

	MemPool.Free(pData);


	----------------------------------------------------------------*/

	template <class DATA>
	class ExplicitFreeList
	{
	private:
		struct st_BLOCK_NODE
		{
			st_BLOCK_NODE()
			{
				stpNextBlock = NULL;
			}
			st_BLOCK_NODE* stpNextBlock;
		};
		
	public:
		ExplicitFreeList(int _iBlockNum, bool _bPlacementNew =false)
		{
			isPlacementNew = _bPlacementNew;
			//초기 블럭 개수만큼 동적할당
			if (isPlacementNew==true)
			{
			}
			else
			{

			}
		}
		virtual ~ExplicitFreeList()
		{
			//모든 메모리 해제. 굳이 해줘야 해?
		}
	public:
		/// @brief 블럭 하나를 할당받는다.  
		/// @param  없음
		/// @return (DATA *) 데이타 블럭 포인터
		DATA* Alloc(void)
		{
			if (isPlacementNew ==true)
			{

			}
			else
			{

			}
		}
		/// @brief 사용중이던 블럭을 해제한다.
		/// @param pData 블럭 포인터
		/// @return (BOOL) TRUE, FALSE.
		bool	Free(DATA* pData)
		{
			if (isPlacementNew == true)
			{

			}
			else
			{

			}
		}
		/// @brief 현재 확보 된 블럭 개수를 얻는다. (메모리풀 내부의 전체 개수)
		/// @param  없음.
		/// @return 메모리 풀 내부 전체 개수
		int		GetAllocCount(void) 
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
	public:
		st_BLOCK_NODE* _pFreeNode;
	private:
		bool isPlacementNew = false;
		int m_iAllocCount;
		int m_iUseCount;
	};
}


