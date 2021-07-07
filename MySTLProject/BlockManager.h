#pragma once
#include <new.h>
#include <iostream>
#include <assert.h>
#include <list>

	using namespace std;
#define ALIGNMENT 8
#define ALIGN(p) (((sizeof(p)) + (ALIGNMENT-1)) & ~0x7) 
#define SHORT 2
//���� �ָ� �� underflowshort�� ���� ���·� ���� �ִ� ��ũ��
#define UNDERFLOWSHORT(block) (*((short*)(block)))
//����, ��ü ũ�⸦ �ָ� ������ overflowshort�� ���� ���·� ������ּ� �ִ� ��ũ�� ,,�е����� ��� �ؾ���
#define OVERFLOWSHORT(block,blockTotalsize) (*((short*)((char*)block + (blockTotalsize - SHORT))))
//DATA�� ��ġ�� �ٷ� �ִ� ��ũ��
#define BL_DATA_P(block) (((char*)block) + SHORT + sizeof(bool)+1 )
//���� ��带 ��Ÿ���� ��ġ�� ����ִ� ��ũ�� objSize�� �������� ũ��
#define NEXT_BL_P(block,objSize) (((char*)block)+SHORT+ sizeof(bool)+1+ objSize)
//���� �ָ� �ٷ� ��뿩��bool ���� ���·� ���� �ִ� ��ũ��
#define ISUSEBOOL(block) (*((bool*)(char*)block+SHORT))

/*
��ü���� �������ִ� Ŭ����
�̹� ������� ��ü�� �ٽ� ���ٴ� ������ �޸� ������
�ݵ�� �ִ� ��ü Ÿ���� ¦���� �������� �Ѵ�. �е��� ���� ����� ���� ����.
¦���� �ƴ� ��� �߸��ϸ� ����Ʈ ������ġ�� ��߳� �� �ִ�.
Block ����
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
				//��� ���� Ȯ���ϰ� ���� ����������
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
	/// @brief �� �ϳ��� �Ҵ�޴´�.  
	/// @param  ����
	/// @return (DATA *) ����Ÿ �� ������
	DATA* Alloc(void)
	{
		if (freeNode==nullptr)
		{
			// ���� ��ü�� ������ ��� 
			int objSize = ALIGN(DATA);
			//��ü ����
			int totalsize = sizeof(short)+sizeof(bool)+1+ objSize + sizeof(st_BLOCK_NODE*) + sizeof(short);
			void * block = malloc(totalsize);
			nodeClearList.push_back(block);
			//�� ��Ʈ
			UNDERFLOWSHORT(block) = objSize;
			//�� ��Ʈ
			OVERFLOWSHORT(block, totalsize)= objSize;
			//��뿩�� üũ
			ISUSEBOOL(block) = true;
			//���� ������ �ʱ�ȭ
			void* data = NEXT_BL_P(block,objSize);
			new(data)st_BLOCK_NODE;
			//������ �ʱ�ȭ
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

			//��ü ������ ȣ��
			st_BLOCK_NODE* block = freeNode;
			//���� üũ�ϱ�
			//��� ���� Ȯ��
			bool isUse = ISUSEBOOL(block);
			if (isUse==true)
			{
				assert(false);
			}
			st_BLOCK_NODE* temp=nullptr;
			temp = *((st_BLOCK_NODE**)(NEXT_BL_P(block, objSize)));
			freeNode = temp;

			//������ �ʱ�ȭ
			if (isPlacementNew == true)
			{
				new(BL_DATA_P(block))DATA;
			}
			ISUSEBOOL(block) = true;

			m_iUseCount++;
			return (DATA*)BL_DATA_P(block);
		}
	}
	/// @brief ������̴� ���� �����Ѵ�.
	/// @param pData �� ������
	/// @return (BOOL) TRUE, FALSE.
	bool Free(DATA* pData)
	{
		int objSize = ALIGN(DATA);
		void* block = ((char*)pData) - (sizeof(short)+sizeof(bool)+1);
		int totalsize = sizeof(short) +sizeof(bool)+1 + objSize + sizeof(st_BLOCK_NODE*) + sizeof(short);
		//�������� ������ ���� Ȯ��
		short underSize= UNDERFLOWSHORT(block);
		short overSize = OVERFLOWSHORT(block, totalsize);
		bool isUse = ISUSEBOOL(block);

		if (isUse ==false||overSize < 0 || underSize < 0 || underSize != overSize)
		{
			return false;
		}

		//data �ʱ�ȭ
		if (isPlacementNew==true)
		{
			((DATA*)BL_DATA_P(block))->~DATA();
		}
		//��� ���� üũ
		ISUSEBOOL(block) = false;
		//������忡 �̾���̱�
		void* data = NEXT_BL_P(block, objSize);
		new(data)st_BLOCK_NODE(freeNode);
		//�������� ����
		freeNode = (st_BLOCK_NODE*)block;

		m_iUseCount--;

		return true;
	}
	/// @brief ���� Ȯ�� �� �� ������ ��´�. (�޸�Ǯ ������ ��ü ����)
	/// @param  ����.
	/// @return �޸� Ǯ ���� ��ü ����
	int	GetAllocCount(void)
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
};

