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
		MakePathNode,//길 연결할때 사용할 색
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
		double g; // 출발점 부터의 이 노드의 거리
		int h; // 목적지 부터 이 노드의 거리
		double f;// g와 h의 합
		stNode* prev;
	};
	class MapBuilder
	{

	};
	/// @brief Astart 알고리즘으로 길찾는 클래스, 
	/// 특징으로 길찾는 구역을 클러스터링을 통해 더욱 빠른 검색이 가능하도록 할 것임. 그렇기 때문에 map  이라는 노드로 이루어진 맵이 필요함
	/// 3*3이 멀쩡히 있는 구역은 하나의 노드로 축약해서 처리할 예정.
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
		list<stNode*> openList;//가야할 곳을 넣은 리스트
		list<stNode*> closeList;//이미 간 곳을 넣은 리스트
		list<stNode*> blockList;//방해물을 넣은 리스트
		list<stNode*> map;//전체 노드에 대한 리스트
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
		/// @brief 출발지 노드 설정
		/// @param _x 
		/// @param _y 
		/// @return false는 설정하지 못한 것임
		bool SetStartNode(int _x, int _y)
		{
			//해당 위치에 node가 있는지 확인
			stNode* node = GetNodeInMapList(_x, _y);
			if (node == nullptr)
			{
				return false;
			}
			//목적지 노드와 같은지 확인
			else if (destNode != nullptr && destNode->x == node->x && destNode->y == node->y)
			{
				return false;
			}
			startNode = node;
			curNode = startNode;
			return true;
		}
		/// @brief  목적지 노드 설정
		/// @param _x 
		/// @param _y 
		/// @return false는 설정하지 못한 것임
		bool SetDestinationNode(int _x, int _y)
		{
			//해당 위치에 node가 있는지 확인
			stNode* node = GetNodeInMapList(_x, _y);
			if (node == nullptr)
			{
				return false;
			}
			//시작 노드와 같은지 확인
			else if (startNode != nullptr && startNode->x == node->x && startNode->y == node->y)
			{
				return false;
			}
			destNode = node;
			return true;
		}
		/// @brief 장애물 노드 설정
		/// @param _x 
		/// @param _y 
		/// @return false는 설정하지 못한 것임
		bool SetBlockingNode(int _x, int _y)
		{
			//해당 위치에 node가 있는지 확인
			stNode* node = GetNodeInMapList(_x, _y);
			if (node == nullptr)
			{
				return false;
			}
			//시작노드와 같은지 확인
			else if (startNode != nullptr && startNode->x == node->x && startNode->y == node->y)
			{
				return false;
			}
			//목적지 노드와 같은지 확인
			else if (destNode != nullptr && destNode->x == node->x && destNode->y == node->y)
			{
				return false;
			}
			//이미 추가된 블록인 경우 넘기기
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

			//장애물 리스트에 추가
			blockList.push_back(node);
			return true;

		}

		/// @brief 길찾기 시작 함수
		/// @return false 길을 찾지 못했다는 의미
		EFileResultType FindPathStart()
		{
			if (curNode ==nullptr) // 현재 노드가 없는 경우임
			{
				return EFileResultType::Error;
			}
			//현재 노드를 close 하기
			closeList.push_back(curNode);
			//오픈에서 제거
			openList.remove(curNode);
			//현재 노드가 목적지 노드인지
			if (IsCurNodeisDestNode(curNode, destNode) == true)
			{
				return EFileResultType::True;
			}
			//현재 노드가 부모가 잘못 설정 되어 있는지
			IsFindMoreRowGPrev(curNode);
			//현재 노드 주변 open 할 수 있는 노드 추가하기
			SearchAndAddOpenNode(curNode);
			//openlist 중에서 가장 낮은 f값 찾기
			stNode* newCurNode = FindRowFValueInOpenList();
			if (newCurNode == nullptr)
			{
				//더이상 찾을 openNode가 없어서 nullptr을 반환함 길을 못찾은 것임
				return EFileResultType::False;
			}
			curNode = newCurNode;
			return EFileResultType::None;
		}
		/// @brief 현재 상태를 화면에 출력하는 코드
		/// @param _func 화면에 출력하기 위한 람다 함수.
		void PrintAll(function<void(stNode* ,ENodeType)> _func)
		{
			if (_func==nullptr)
			{
				return;
			}
			//map node 전부 출력
			list<stNode*>::iterator mapitr = map.begin();
			for (; mapitr!= map.end();++mapitr)
			{
				if((*mapitr)==nullptr)
				{
					continue;
				}
				_func((*mapitr),ENodeType::NomalMapNode);
			}
			//open node 전부 출력
			list<stNode*>::iterator openitr = openList.begin();
			for (; openitr != openList.end(); ++openitr)
			{
				if ((*openitr) == nullptr)
				{
					continue;
				}
				_func((*openitr), ENodeType::OpenNode);
			}
			//close node 전부 출력
			list<stNode*>::iterator closeitr = closeList.begin();
			for (; closeitr != closeList.end(); ++closeitr)
			{
				if ((*closeitr) == nullptr)
				{
					continue;
				}
				_func((*closeitr), ENodeType::CloseNode);
			}
			//blocking 그리기
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
			//위
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
						openList.push_back(node); // open list에 노드 추가
					}
				}
			}
			//우측 위
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
						openList.push_back(node); // open list에 노드 추가
					}
				}
			}
			//우측
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
						openList.push_back(node); // open list에 노드 추가
					}
				}
			}
			//우측 아래
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
						openList.push_back(node); // open list에 노드 추가
					}
				}
			}
			//아래
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
						openList.push_back(node); // open list에 노드 추가
					}
				}
			}
			//좌측 아래
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
						openList.push_back(node); // open list에 노드 추가
					}
				}
			}
			//좌측
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
						openList.push_back(node); // open list에 노드 추가
					}
				}
			}
			//좌측 위
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
						openList.push_back(node); // open list에 노드 추가
					}
				}
			}

			openList.sort([](const stNode* lhs, const stNode* rhs) {
				return lhs->f < rhs->f;
				});
		}
		/// @brief 사각형 맵을 만드는 함수
		/// @param _x 가로 사이즈
		/// @param _y 세로 사이즈
		void MakeSquareMap(int _x, int _y)
		{
			//node 만들기
			for (int x = 0; x < _x; x++)
			{
				for (int y = 0; y < _y; y++)
				{
					map.push_back(new stNode(x, y));
				}
			}
		}
		/// @brief map 리스트의 특정 좌표에 node가 존재하는지
		/// @param _x 
		/// @param _y 
		/// @return 찾았다면 해당 node를 반환함, 없다면 nullptr을 반환함.
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
		/// @brief map 리스트에 있는 모든 정보 삭제 함수 메모리 까지 전부 반환함
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
		/// @brief 노드가 blocking 노드인지 판단하는 함수
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
		/// @brief 그저 openlist의 가장 최상단 open노드를 돌려준다.
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
			//주변 8방향에서 더 작은 h 값이 있다면 그것을 부모로 둠
			stNode* node = nullptr;
			double addValue = 0;
			//위
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
			//우측 위
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
			//우측
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
			//우측 아래
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
			//아래
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
			//좌측 아래
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
			//좌측
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
			//좌측 위
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

