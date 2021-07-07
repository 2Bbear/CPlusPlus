#pragma once
#include <new.h>
#include <iostream>

namespace twobbearlib
{


	/*---------------------------------------------------------------

	procademy MemoryPool.

	�޸� Ǯ Ŭ���� (������Ʈ Ǯ / ��������Ʈ)
	Ư�� ����Ÿ(����ü,Ŭ����,����)�� ������ �Ҵ� �� ��������.

	- ����.

	procademy::CMemoryPool<DATA> MemPool(300, FALSE);
	DATA *pData = MemPool.Alloc();

	pData ���

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
			//�ʱ� �� ������ŭ �����Ҵ�
			if (isPlacementNew==true)
			{
			}
			else
			{

			}
		}
		virtual ~ExplicitFreeList()
		{
			//��� �޸� ����. ���� ����� ��?
		}
	public:
		/// @brief �� �ϳ��� �Ҵ�޴´�.  
		/// @param  ����
		/// @return (DATA *) ����Ÿ �� ������
		DATA* Alloc(void)
		{
			if (isPlacementNew ==true)
			{

			}
			else
			{

			}
		}
		/// @brief ������̴� ���� �����Ѵ�.
		/// @param pData �� ������
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
		/// @brief ���� Ȯ�� �� �� ������ ��´�. (�޸�Ǯ ������ ��ü ����)
		/// @param  ����.
		/// @return �޸� Ǯ ���� ��ü ����
		int		GetAllocCount(void) 
		{ 
			return m_iAllocCount; 
		}
		/// @brief ���� ������� �� ������ ��´�.
		/// @param  ����.
		/// @return ������� �� ����.
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


