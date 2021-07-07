#pragma once
#include <list>
#include <functional>
#include <math.h>
#include <assert.h>
namespace twobbearlib
{
	using namespace std;
	enum class ENodeType: int 
	{
		NomalMapNode = 0,
		BlockingNode,//blocking
		Black,//
		StartNode,//start
		DestNode, //dest
		CloseNode,//close
		OpenNode,//open
		MakePathNode,//�� �����Ҷ� ����� ��
	};
	enum class EFileResultType :int
	{
		False,
		True,
		Error,
		None
	};
	struct stNode
	{
		stNode()
		{
			prev = nullptr;
			x = 0;
			y = 0;
			g = 0;
			h = 0;
			f = g + h;
		}
		stNode(int _x, int _y)
		{
			prev = nullptr;
			x = _x;
			y = _y;
			g = 0;
			h = 0;
			f = g + h;
		}
		int x;
		int y;
		double g; // ����� ������ �� ����� �Ÿ�
		int h; // ������ ���� �� ����� �Ÿ�
		double f;// g�� h�� ��
		stNode* prev;
	};
	class MapBuilder
	{

	};
	/// @brief Astart �˰������� ��ã�� Ŭ����, 
	/// Ư¡���� ��ã�� ������ Ŭ�����͸��� ���� ���� ���� �˻��� �����ϵ��� �� ����. �׷��� ������ map  �̶�� ���� �̷���� ���� �ʿ���
	/// 3*3�� ������ �ִ� ������ �ϳ��� ���� ����ؼ� ó���� ����.
	class AStarPathFinder
	{
	public:

	public:
		AStarPathFinder()
		{

		}
		AStarPathFinder(int _mapXSize, int _mapYSize)
		{
			mapSizeX = _mapXSize;
			mapSizeY = _mapYSize;
			MakeSquareMap(_mapXSize, _mapYSize);
		}
		~AStarPathFinder()
		{
			ClearAllMapNode();
			closeList.clear();
			openList.clear();
			blockList.clear();
		}
	public:

	private:
		stNode* startNode=nullptr;
		stNode* curNode = nullptr;
		stNode* destNode = nullptr;
		list<stNode*> openList;//������ ���� ���� ����Ʈ
		list<stNode*> closeList;//�̹� �� ���� ���� ����Ʈ
		list<stNode*> blockList;//���ع��� ���� ����Ʈ
		list<stNode*> map;//��ü ��忡 ���� ����Ʈ
		int mapSizeX;
		int mapSizeY;
	public:
		void InitializeAStar()
		{
			closeList.clear();
			openList.clear();
			blockList.clear();
			list<stNode*>::iterator itr = map.begin();
			for (;itr!=map.end();++itr)
			{
				if ((*itr)!=nullptr)
				{
					(*itr)->f = 0;
					(*itr)->g = 0;
					(*itr)->h = 0;
					(*itr)->prev = nullptr;
				}
			}
			curNode = startNode;
		}
		/// @brief ����� ��� ����
		/// @param _x 
		/// @param _y 
		/// @return false�� �������� ���� ����
		bool SetStartNode(int _x, int _y)
		{
			//�ش� ��ġ�� node�� �ִ��� Ȯ��
			stNode* node = GetNodeInMapList(_x, _y);
			if (node == nullptr)
			{
				return false;
			}
			//������ ���� ������ Ȯ��
			else if (destNode != nullptr && destNode->x == node->x && destNode->y == node->y)
			{
				return false;
			}
			startNode = node;
			curNode = startNode;
			return true;
		}
		/// @brief  ������ ��� ����
		/// @param _x 
		/// @param _y 
		/// @return false�� �������� ���� ����
		bool SetDestinationNode(int _x, int _y)
		{
			//�ش� ��ġ�� node�� �ִ��� Ȯ��
			stNode* node = GetNodeInMapList(_x, _y);
			if (node == nullptr)
			{
				return false;
			}
			//���� ���� ������ Ȯ��
			else if (startNode != nullptr && startNode->x == node->x && startNode->y == node->y)
			{
				return false;
			}
			destNode = node;
			return true;
		}
		/// @brief ��ֹ� ��� ����
		/// @param _x 
		/// @param _y 
		/// @return false�� �������� ���� ����
		bool SetBlockingNode(int _x, int _y)
		{
			//�ش� ��ġ�� node�� �ִ��� Ȯ��
			stNode* node = GetNodeInMapList(_x, _y);
			if (node == nullptr)
			{
				return false;
			}
			//���۳��� ������ Ȯ��
			else if (startNode != nullptr && startNode->x == node->x && startNode->y == node->y)
			{
				return false;
			}
			//������ ���� ������ Ȯ��
			else if (destNode != nullptr && destNode->x == node->x && destNode->y == node->y)
			{
				return false;
			}
			//�̹� �߰��� ����� ��� �ѱ��
			list<stNode*>::iterator itr = blockList.begin();
			for (;itr != blockList.end();++itr)
			{
				if ((*itr)!=nullptr)
				{
					if ((*itr)== node)
					{
						//blockList.erase(itr);
						return false;
					}
				}
			}

			//��ֹ� ����Ʈ�� �߰�
			blockList.push_back(node);
			return true;

		}

		/// @brief ��ã�� ���� �Լ�
		/// @return false ���� ã�� ���ߴٴ� �ǹ�
		EFileResultType FindPathStart()
		{
			if (curNode ==nullptr) // ���� ��尡 ���� �����
			{
				return EFileResultType::Error;
			}
			//���� ��带 close �ϱ�
			closeList.push_back(curNode);
			//���¿��� ����
			openList.remove(curNode);
			//���� ��尡 ������ �������
			if (IsCurNodeisDestNode(curNode, destNode) == true)
			{
				return EFileResultType::True;
			}
			//���� ��尡 �θ� �߸� ���� �Ǿ� �ִ���
			IsFindMoreRowGPrev(curNode);
			//���� ��� �ֺ� open �� �� �ִ� ��� �߰��ϱ�
			SearchAndAddOpenNode(curNode);
			//openlist �߿��� ���� ���� f�� ã��
			stNode* newCurNode = FindRowFValueInOpenList();
			if (newCurNode == nullptr)
			{
				//���̻� ã�� openNode�� ��� nullptr�� ��ȯ�� ���� ��ã�� ����
				return EFileResultType::False;
			}
			curNode = newCurNode;
			return EFileResultType::None;
		}
		/// @brief ���� ���¸� ȭ�鿡 ����ϴ� �ڵ�
		/// @param _func ȭ�鿡 ����ϱ� ���� ���� �Լ�.
		void PrintAll(function<void(stNode* ,ENodeType)> _func)
		{
			if (_func==nullptr)
			{
				return;
			}
			//map node ���� ���
			list<stNode*>::iterator mapitr = map.begin();
			for (; mapitr!= map.end();++mapitr)
			{
				if((*mapitr)==nullptr)
				{
					continue;
				}
				_func((*mapitr),ENodeType::NomalMapNode);
			}
			//open node ���� ���
			list<stNode*>::iterator openitr = openList.begin();
			for (; openitr != openList.end(); ++openitr)
			{
				if ((*openitr) == nullptr)
				{
					continue;
				}
				_func((*openitr), ENodeType::OpenNode);
			}
			//close node ���� ���
			list<stNode*>::iterator closeitr = closeList.begin();
			for (; closeitr != closeList.end(); ++closeitr)
			{
				if ((*closeitr) == nullptr)
				{
					continue;
				}
				_func((*closeitr), ENodeType::CloseNode);
			}
			//blocking �׸���
			list<stNode*>::iterator blockitr = blockList.begin();
			for (; blockitr != blockList.end(); ++blockitr)
			{
				if ((*blockitr) == nullptr)
				{
					continue;
				}
				_func((*blockitr), ENodeType::BlockingNode);
			}
			//start, dest , 
			_func(startNode, ENodeType::StartNode);
			_func(destNode, ENodeType::DestNode);
		}
		void PrintPath(function<void(stNode*, ENodeType)> _func)
		{
			if (_func == nullptr)
			{
				return;
			}
			stNode* node = curNode;
			while (node->prev!=nullptr)
			{
				_func(node, ENodeType::MakePathNode);
				node = node->prev;
			}
		}
	private:
		void SearchAndAddOpenNode(stNode* _curNode)
		{
			stNode* node = nullptr;
			//��
			node = GetNodeInMapList(_curNode->x , _curNode->y - 1);
			if (node != nullptr)
			{
				if (IsBlockingNode(node) == false && IsClosedNode(node) == false)
				{
					if (IsOpenedNode(node) == false)
					{
						node->g = _curNode->g + 1;
						node->h = CalH(destNode,node);
						node->f = node->g + node->h;
						node->prev = _curNode;
						openList.push_back(node); // open list�� ��� �߰�
					}
				}
			}
			//���� ��
			node = GetNodeInMapList(_curNode->x+1, _curNode->y - 1);
			if (node != nullptr)
			{
				if (IsBlockingNode(node) == false && IsClosedNode(node) == false)
				{
					if (IsOpenedNode(node) == false)
					{
						node->g = _curNode->g + 1.5;
						node->h = CalH(destNode, node);
						node->f = node->g + node->h;
						node->prev = _curNode;
						openList.push_back(node); // open list�� ��� �߰�
					}
				}
			}
			//����
			node = GetNodeInMapList(_curNode->x+1, _curNode->y );
			if (node != nullptr)
			{
				if (IsBlockingNode(node) == false && IsClosedNode(node) == false)
				{
					if (IsOpenedNode(node) == false)
					{
						node->g = _curNode->g + 1;
						node->h = CalH(destNode, node);
						node->f = node->g + node->h;
						node->prev = _curNode;
						openList.push_back(node); // open list�� ��� �߰�
					}
				}
			}
			//���� �Ʒ�
			node = GetNodeInMapList(_curNode->x+1, _curNode->y + 1);
			if (node != nullptr)
			{
				if (IsBlockingNode(node) == false && IsClosedNode(node) == false)
				{
					if (IsOpenedNode(node) == false)
					{
						node->g = _curNode->g + 1.5;
						node->h = CalH(destNode, node);
						node->f = node->g + node->h;
						node->prev = _curNode;
						openList.push_back(node); // open list�� ��� �߰�
					}
				}
			}
			//�Ʒ�
			node = GetNodeInMapList(_curNode->x, _curNode->y + 1);
			if (node != nullptr)
			{
				if (IsBlockingNode(node) == false && IsClosedNode(node) == false)
				{
					if (IsOpenedNode(node) == false)
					{
						node->g = _curNode->g + 1;
						node->h = CalH(destNode, node);
						node->f = node->g + node->h;
						node->prev = _curNode;
						openList.push_back(node); // open list�� ��� �߰�
					}
				}
			}
			//���� �Ʒ�
			node = GetNodeInMapList(_curNode->x - 1, _curNode->y + 1);
			if (node != nullptr)
			{
				if (IsBlockingNode(node) == false && IsClosedNode(node) == false)
				{
					if (IsOpenedNode(node) == false)
					{
						node->g = _curNode->g + 1.5;
						node->h = CalH(destNode, node);
						node->f = node->g + node->h;
						node->prev = _curNode;
						openList.push_back(node); // open list�� ��� �߰�
					}
				}
			}
			//����
			node = GetNodeInMapList(_curNode->x - 1, _curNode->y );
			if (node != nullptr) 
			{
				if (IsBlockingNode(node) == false && IsClosedNode(node) == false)
				{
					if (IsOpenedNode(node) == false)
					{
						node->g = _curNode->g + 1;
						node->h = CalH(destNode, node);
						node->f = node->g + node->h;
						node->prev = _curNode;
						openList.push_back(node); // open list�� ��� �߰�
					}
				}
			}
			//���� ��
			node = GetNodeInMapList(_curNode->x - 1, _curNode->y - 1);
			if (node != nullptr)
			{
				if (IsBlockingNode(node) == false && IsClosedNode(node) == false)
				{
					if (IsOpenedNode(node) == false)
					{
						node->g = _curNode->g + 1.5;
						node->h = CalH(destNode, node);
						node->f = node->g + node->h;
						node->prev = _curNode;
						openList.push_back(node); // open list�� ��� �߰�
					}
				}
			}

			openList.sort([](const stNode* lhs, const stNode* rhs) {
				return lhs->f < rhs->f;
				});
		}
		/// @brief �簢�� ���� ����� �Լ�
		/// @param _x ���� ������
		/// @param _y ���� ������
		void MakeSquareMap(int _x, int _y)
		{
			//node �����
			for (int x = 0; x < _x; x++)
			{
				for (int y = 0; y < _y; y++)
				{
					map.push_back(new stNode(x, y));
				}
			}
		}
		/// @brief map ����Ʈ�� Ư�� ��ǥ�� node�� �����ϴ���
		/// @param _x 
		/// @param _y 
		/// @return ã�Ҵٸ� �ش� node�� ��ȯ��, ���ٸ� nullptr�� ��ȯ��.
		stNode* GetNodeInMapList(int _x, int _y)
		{
			list<stNode*>::iterator itr = map.begin();
			for (;itr != map.end();++itr)
			{
				if ((*itr)!=nullptr && (*itr)->x==_x && (*itr)->y == _y)
				{
					return (*itr);
				}
			}

			return nullptr;
		}
		/// @brief map ����Ʈ�� �ִ� ��� ���� ���� �Լ� �޸� ���� ���� ��ȯ��
		void ClearAllMapNode()
		{
			list<stNode*>::iterator itr = map.begin();
			for (; itr != map.end();)
			{
				if ((*itr) != nullptr)
				{
					delete (*itr);
					(*itr) = nullptr;
				}
				itr = map.erase(itr);
			}
		}
		/// @brief ��尡 blocking ������� �Ǵ��ϴ� �Լ�
		/// @param _node 
		/// @return 
		bool IsBlockingNode(stNode* _node)
		{
			list<stNode*>::iterator itr = blockList.begin();
			for (;itr!=blockList.end();++itr)
			{
				if ((*itr)!=nullptr && (*itr)->x == _node->x && (*itr)->y == _node->y)
				{
					return true;
				}
			}
			return false;
		}
		bool IsClosedNode(stNode* _node)
		{
			list<stNode*>::iterator itr = closeList.begin();
			for (;itr != closeList.end();++itr)
			{
				if ((*itr)!=nullptr && (*itr)->x==_node->x && (*itr)->y == _node->y)
				{
					return true;
				}
			}
			return false;
		}
		bool IsOpenedNode(stNode* _node)
		{
			list<stNode*>::iterator itr = openList.begin();
			for (; itr != openList.end(); ++itr)
			{
				if ((*itr) != nullptr && (*itr)->x == _node->x && (*itr)->y == _node->y)
				{
					return true;
				}
			}
			return false;
		}
		int CalH(stNode* _destNode,stNode* _node)
		{
			int x = abs(_node->x - _destNode->x);
			int y = abs(_node->y - _destNode->y);
			int h = (x + y);
			return h;
		}
		/// @brief ���� openlist�� ���� �ֻ�� open��带 �����ش�.
		/// @return 
		stNode* FindRowFValueInOpenList()
		{
			stNode* result = nullptr;
			list<stNode*>::iterator itr = openList.begin();
			for (; itr != openList.end(); ++itr)
			{
				if ((*itr) != nullptr)
				{
					result = (*itr);
					openList.erase(itr);
					return result;
				}
			}
			return nullptr;
		}
		bool IsCurNodeisDestNode(stNode* _curNode, stNode* _destNode)
		{
			if (_curNode->x ==_destNode->x && _curNode->y == _destNode->y)
			{
				return true;
			}
			return false;
		}

		void IsFindMoreRowGPrev(stNode* _targetNode)
		{
			stNode* newPrev = nullptr;
			//�ֺ� 8���⿡�� �� ���� h ���� �ִٸ� �װ��� �θ�� ��
			stNode* node = nullptr;
			double addValue = 0;
			//��
			node = GetNodeInMapList(_targetNode->x, _targetNode->y - 1);
			if (node != nullptr)
			{
				if (IsBlockingNode(node) == false )
				{
					if (IsOpenedNode(node) == true|| IsClosedNode(node)==true)
					{
						if (_targetNode->g > node->g)
						{
							if (newPrev==nullptr)
							{
								newPrev = node;
								addValue = 1;
							}
							else if(newPrev->g> node->g)
							{
								newPrev = node;
								addValue = 1;
							}
							
						}
					}
				}
			}
			//���� ��
			node = GetNodeInMapList(_targetNode->x + 1, _targetNode->y - 1);
			if (node != nullptr)
			{
				if (IsBlockingNode(node) == false )
				{
					if (IsOpenedNode(node) == true || IsClosedNode(node) == true)
					{
						if (_targetNode->g > node->g)
						{
							if (newPrev == nullptr)
							{
								newPrev = node;
								addValue = 1.5;
							}
							else if (newPrev->g > node->g)
							{
								newPrev = node;
								addValue = 1.5;
							}

						}
					}
				}
			}
			//����
			node = GetNodeInMapList(_targetNode->x + 1, _targetNode->y);
			if (node != nullptr)
			{
				if (IsBlockingNode(node) == false )
				{
					if (IsOpenedNode(node) == true || IsClosedNode(node) == true)
					{
						if (_targetNode->g > node->g)
						{
							if (newPrev == nullptr)
							{
								newPrev = node;
								addValue = 1;
							}
							else if (newPrev->g > node->g)
							{
								newPrev = node;
								addValue = 1;
							}

						}
					}
				}
			}
			//���� �Ʒ�
			node = GetNodeInMapList(_targetNode->x + 1, _targetNode->y + 1);
			if (node != nullptr)
			{
				if (IsBlockingNode(node) == false)
				{
					if (IsOpenedNode(node) == true || IsClosedNode(node) == true)
					{
						if (_targetNode->g > node->g)
						{
							if (newPrev == nullptr)
							{
								newPrev = node;
								addValue = 1.5;
							}
							else if (newPrev->g > node->g)
							{
								newPrev = node;
								addValue = 1.5;
							}

						}
					}
				}
			}
			//�Ʒ�
			node = GetNodeInMapList(_targetNode->x, _targetNode->y + 1);
			if (node != nullptr)
			{
				if (IsBlockingNode(node) == false )
				{
					if (IsOpenedNode(node) == true || IsClosedNode(node) == true)
					{
						if (_targetNode->g > node->g)
						{
							if (newPrev == nullptr)
							{
								newPrev = node;
								addValue = 1;
							}
							else if (newPrev->g > node->g)
							{
								newPrev = node;
								addValue = 1;
							}

						}
					}
				}
			}
			//���� �Ʒ�
			node = GetNodeInMapList(_targetNode->x - 1, _targetNode->y + 1);
			if (node != nullptr)
			{
				if (IsBlockingNode(node) == false )
				{
					if (IsOpenedNode(node) == true || IsClosedNode(node) == true)
					{
						if (_targetNode->g > node->g)
						{
							if (newPrev == nullptr)
							{
								newPrev = node;
								addValue = 1.5;
							}
							else if (newPrev->g > node->g)
							{
								newPrev = node;
								addValue = 1.5;
							}

						}
					}
				}
			}
			//����
			node = GetNodeInMapList(_targetNode->x - 1, _targetNode->y);
			if (node != nullptr)
			{
				if (IsBlockingNode(node) == false )
				{
					if (IsOpenedNode(node) == true || IsClosedNode(node) == true)
					{
						if (newPrev == nullptr)
						{
							newPrev = node;
							addValue = 1;
						}
						else if (newPrev->g > node->g)
						{
							newPrev = node;
							addValue = 1;
						}
					}
				}
			}
			//���� ��
			node = GetNodeInMapList(_targetNode->x - 1, _targetNode->y - 1);
			if (node != nullptr)
			{
				if (IsBlockingNode(node) == false )
				{
					if (IsOpenedNode(node) == true || IsClosedNode(node) == true)
					{
						if (newPrev == nullptr)
						{
							newPrev = node;
							addValue = 1.5;
						}
						else if (newPrev->g > node->g)
						{
							newPrev = node;
							addValue = 1.5;
						}
					}
				}
			}
			if (newPrev!=nullptr)
			{
				_targetNode->prev = newPrev;
				_targetNode->g = newPrev->g + addValue;
				_targetNode->f = _targetNode->g + _targetNode->h;
			}
		}
	};
}

