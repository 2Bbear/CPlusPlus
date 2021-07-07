#pragma once
#include <list>
#include <functional>
#include <math.h>
#include <assert.h>
#include <Windows.h>
namespace twobbearlib
{
	using namespace std;
	enum class ENodeType : int
	{
		NomalMapNode = 0,
		BlockingNode,//blocking
		Black,//
		StartNode,//start
		DestNode, //dest
		CloseNode,//close
		OpenNode,//open
		MakePathNode,//길 연결할때 사용할 색
		RelayNode,//중간 노드용
	};
	enum class EFileResultType :int
	{
		False,
		True,
		Error,
		None
	};
	enum class EDirection : int
	{
		LL, //왼쪽
		LD, //좌하
		DD, //하
		RD, // 우하
		RR, // 우
		RU,//우상
		UU, // 상
		LU // 좌상
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

	/// @brief Astart 알고리즘으로 길찾는 클래스, 
	/// 특징으로 길찾는 구역을 클러스터링을 통해 더욱 빠른 검색이 가능하도록 할 것임. 그렇기 때문에 map  이라는 노드로 이루어진 맵이 필요함
	/// 3*3이 멀쩡히 있는 구역은 하나의 노드로 축약해서 처리할 예정.
	class JumpPointPathFinder
	{
	public:

	public:
		JumpPointPathFinder()
		{

		}
		JumpPointPathFinder(int _mapXSize, int _mapYSize)
		{
			mapSizeX = _mapXSize;
			mapSizeY = _mapYSize;
			MakeSquareMap(_mapXSize, _mapYSize);
		}
		~JumpPointPathFinder()
		{
			DeleteAllMapNode();
			closeList.clear();
			openList.clear();
			blockList.clear();
			checkedNodeList.clear();
		}
	public:

	private:
		stNode* startNode = nullptr;
		stNode* curNode = nullptr;
		stNode* destNode = nullptr;
		list<stNode*> openList;//가야할 곳을 넣은 리스트
		list<stNode*> closeList;//이미 간 곳을 넣은 리스트
		list<stNode*> blockList;//방해물을 넣은 리스트
		stNode** map;//전체 노드에 대한 리스트
		list<stNode*> checkedNodeList;
		int mapSizeX;
		int mapSizeY;
	public:
		/// @brief map의 메모리만 남긴 채로 모든 정보를 초기화함
		void InitializeAStar()
		{
			closeList.clear();
			openList.clear();
			blockList.clear();
			checkedNodeList.clear();
			ClearMapData();
			curNode = startNode;
		}
		/// @brief 장애물 정보와 map의 메모리만 남긴 채로 나머지 정보를 초기화함
		void ResetNodeNotBlock()
		{
			closeList.clear();
			openList.clear();
			checkedNodeList.clear();
			ClearMapData();
			curNode = startNode;
		}
		/// @brief 출발지 노드 설정
		/// @param _x 
		/// @param _y 
		/// @return false는 설정하지 못한 것임
		bool SetStartNode(int _x, int _y)
		{
			//해당 위치에 node가 있는지 확인
			stNode* node = GetNodeInMap(_x, _y);
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
			stNode* node = GetNodeInMap(_x, _y);
			if (node == nullptr)
			{
				return false;
			}
			//시작 노드와 같은지 확인
			else if (startNode ==destNode)
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
			stNode* node = GetNodeInMap(_x, _y);
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
			for (; itr != blockList.end(); ++itr)
			{
				if ((*itr) != nullptr)
				{
					if ((*itr) == node)
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

		/// @brief 한번 길을 찾는 함수
		/// @return false 길을 찾지 못했다는 의미
		EFileResultType FindPathOneStep()
		{
			if (curNode == nullptr) // 현재 노드가 없는 경우임
			{
				return EFileResultType::Error;
			}
			
			//현재 노드가 목적지 노드인지
			if (IsCurNodeisDestNode(curNode, destNode) == true)
			{
				return EFileResultType::True;
			}
			


			if (curNode->prev==nullptr) //첫 시작 노드
			{
				//위
				{
					
					//기본 - 좌우가 장애물이 있다가 없어 졌는지
					stNode* upNode = UUSearch(curNode);
					if (upNode != nullptr)
					{
						openList.push_back(upNode);
					}
					//강제 - curNode 왼쪽에 장애물이 있었을 경우
					if (IsBlockingNode(GetNodeInMap(curNode->x - 1, curNode->y)) == true)
					{
						//왼쪽 상단이 오픈노드인지, 블럭인지 확인
						stNode* target = GetNodeInMap(curNode->x - 1, curNode->y - 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsOpenedNode(target) == false)
						{
							target->prev = curNode;
							NodeStraightCalculate(target);
							openList.push_back(target);
						}
					}
					//	   - curNode 오른쪽에 장애물이 있었을 경우
					if (IsBlockingNode(GetNodeInMap(curNode->x + 1, curNode->y)) == true)
					{
						//우상이 오픈노드인지 블럭인지 확인
						stNode* target = GetNodeInMap(curNode->x + 1, curNode->y - 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsOpenedNode(target) == false)
						{
							target->prev = curNode;
							NodeStraightCalculate(target);
							openList.push_back(target);
						}
					}
				}
				//아래
				{
					
					//기본 - 좌우가 장애물이 있다가 없어 졌는지
					stNode* downNode = DDSearch(curNode);
					if (downNode != nullptr)
					{
						openList.push_back(downNode);
					}
					//강제 - curNode 왼쪽에 장애물이 있었을 경우
					if (IsBlockingNode(GetNodeInMap(curNode->x - 1, curNode->y)) == true)
					{
						//왼쪽 상단이 오픈노드인지, 블럭인지 확인
						stNode* target = GetNodeInMap(curNode->x - 1, curNode->y + 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsOpenedNode(target) == false)
						{
							target->prev = curNode;
							NodeStraightCalculate(target);
							openList.push_back(target);
						}
					}
					//	   - curNode 오른쪽에 장애물이 있었을 경우
					if (IsBlockingNode(GetNodeInMap(curNode->x + 1, curNode->y)) == true)
					{
						//우상이 오픈노드인지 블럭인지 확인
						stNode* target = GetNodeInMap(curNode->x + 1, curNode->y + 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsOpenedNode(target) == false)
						{
							target->prev = curNode;
							NodeStraightCalculate(target);
							openList.push_back(target);
						}
					}
				}
				//좌
				{
					
					//기본 - 위아래로 장애물이 있다가 없어 졌는지
					stNode* leftNode = LLSearch(curNode);
					if (leftNode != nullptr)
					{
						openList.push_back(leftNode);
					}
					//강제 - curNode 하단에 장애물이 있었을 경우
					if (IsBlockingNode(GetNodeInMap(curNode->x, curNode->y+1)) == true)
					{
						//왼쪽 하단이 오픈노드인지, 블럭인지 확인
						stNode* target = GetNodeInMap(curNode->x-1, curNode->y+ 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsOpenedNode(target) == false)
						{
							target->prev = curNode;
							NodeStraightCalculate(target);
							openList.push_back(target);
						}
					}
					//     - curNode 상단에 장애물이 있었을 경우
					if (IsBlockingNode(GetNodeInMap(curNode->x, curNode->y - 1)) == true)
					{
						//왼쪽 상단이 오픈노드인지, 블럭인지 확인
						stNode* target = GetNodeInMap(curNode->x-1, curNode->y - 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsOpenedNode(target) == false)
						{
							target->prev = curNode;
							NodeStraightCalculate(target);
							openList.push_back(target);
						}
					}
				}
				//우
				{

					//기본 - 위아래로 장애물이 있다가 없어 졌는지
					stNode* leftNode = RRSearch(curNode);
					if (leftNode != nullptr)
					{
						openList.push_back(leftNode);
					}
					//강제 - curNode 하단에 장애물이 있었을 경우
					if (IsBlockingNode(GetNodeInMap(curNode->x, curNode->y + 1)) == true)
					{
						//우측 하단이 오픈노드인지, 블럭인지 확인
						stNode* target = GetNodeInMap(curNode->x + 1, curNode->y + 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsOpenedNode(target) == false)
						{
							target->prev = curNode;
							NodeStraightCalculate(target);
							openList.push_back(target);
						}
					}
					//     - curNode 상단에 장애물이 있었을 경우
					if (IsBlockingNode(GetNodeInMap(curNode->x, curNode->y - 1)) == true)
					{
						//우측 상단이 오픈노드인지, 블럭인지 확인
						stNode* target = GetNodeInMap(curNode->x + 1, curNode->y - 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsOpenedNode(target) == false)
						{
							target->prev = curNode;
							NodeStraightCalculate(target);
							openList.push_back(target);
						}
					}
				}
				//좌상
				{
					//기본 - 대각선 이동, 직접 다음 노드부터 할 것을 지정함
					stNode* leftUpDiagonal = LUSearch(GetNodeInMap(curNode->x - 1, curNode->y - 1), curNode);
					if (leftUpDiagonal!=nullptr)
					{
						leftUpDiagonal->prev = curNode;
						NodeDiagonalCalculate(leftUpDiagonal);
						openList.push_back(leftUpDiagonal);
					}
				}
				//좌하
				{
					stNode* leftDownDiagonal = LDSearch(GetNodeInMap(curNode->x - 1, curNode->y + 1), curNode);
					if (leftDownDiagonal != nullptr)
					{
						leftDownDiagonal->prev = curNode;
						NodeDiagonalCalculate(leftDownDiagonal);
						openList.push_back(leftDownDiagonal);
					}
				}
				
				//우상
				{
					stNode* rightUpDiagonal = RUSearch(GetNodeInMap(curNode->x + 1, curNode->y - 1), curNode);
					if (rightUpDiagonal != nullptr)
					{
						rightUpDiagonal->prev = curNode;
						NodeDiagonalCalculate(rightUpDiagonal);
						openList.push_back(rightUpDiagonal);
					}
				}
				//우하
				{
					stNode* rightDownDiagonal = RDSearch(GetNodeInMap(curNode->x + 1, curNode->y + 1), curNode);
					if (rightDownDiagonal != nullptr)
					{
						rightDownDiagonal->prev = curNode;
						NodeDiagonalCalculate(rightDownDiagonal);
						openList.push_back(rightDownDiagonal);
					}
				}
			}
			else //첫 시작 노드가 아닐 경우
			{
				//부모에서 어떻게 왔는지 방향을 알아내야함.
 				stNode* pPrev = curNode->prev;
				//우측 방향인지
				if (pPrev->x < curNode->x && pPrev->y == curNode->y)
				{
					//기본 - 위아래로 장애물이 있다가 없어 졌는지
					stNode* leftNode = RRSearch(curNode);
					if (leftNode != nullptr)
					{
						openList.push_back(leftNode);
					}
					//강제 - curNode 하단에 장애물이 있었을 경우
					if (IsBlockingNode(GetNodeInMap(curNode->x, curNode->y + 1)) == true)
					{
						//우측 하단이 오픈노드인지, 블럭인지 확인
						stNode* target = GetNodeInMap(curNode->x + 1, curNode->y + 1);
						if (target != nullptr 
							&& IsBlockingNode(target) == false 
							&& IsClosedNode(target)==false)
						{
							if (IsOpenedNode(target)==true)
							{
								if (target->prev->g > curNode->g)
								{
									target->prev = curNode;
								}
							}
							else
							{
								target->prev = curNode;
								NodeStraightCalculate(target);
								openList.push_back(target);
							}
							
						}
					}
					//     - curNode 상단에 장애물이 있었을 경우
					if (IsBlockingNode(GetNodeInMap(curNode->x, curNode->y - 1)) == true)
					{
						//우측 상단이 오픈노드인지, 블럭인지 확인
						stNode* target = GetNodeInMap(curNode->x + 1, curNode->y - 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsClosedNode(target) == false)
						{
							if (IsOpenedNode(target) == true)
							{
								if (target->prev->g > curNode->g)
								{
									target->prev = curNode;
								}
							}
							else
							{
								target->prev = curNode;
								NodeStraightCalculate(target);
								openList.push_back(target);
							}
						}
					}
				}
				//좌측인지
				else if (pPrev->x > curNode->x && pPrev->y == curNode->y)
				{
					//기본 - 위아래로 장애물이 있다가 없어 졌는지
					stNode* leftNode = LLSearch(curNode);
					if (leftNode != nullptr)
					{
						openList.push_back(leftNode);
					}
					//강제 - curNode 하단에 장애물이 있었을 경우
					if (IsBlockingNode(GetNodeInMap(curNode->x, curNode->y + 1)) == true)
					{
						//왼쪽 하단이 오픈노드인지, 블럭인지 확인
						stNode* target = GetNodeInMap(curNode->x - 1, curNode->y + 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsClosedNode(target) == false)
						{
							if (IsOpenedNode(target) == true)
							{
								if (target->prev->g > curNode->g)
								{
									target->prev = curNode;
								}
							}
							else
							{

								target->prev = curNode;
								NodeStraightCalculate(target);
								openList.push_back(target);
							}
						}
					}
					//     - curNode 상단에 장애물이 있었을 경우
					if (IsBlockingNode(GetNodeInMap(curNode->x, curNode->y - 1)) == true)
					{
						//왼쪽 상단이 오픈노드인지, 블럭인지 확인
						stNode* target = GetNodeInMap(curNode->x - 1, curNode->y - 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsClosedNode(target) == false)
						{
							if (IsOpenedNode(target) == true)
							{
								if (target->prev->g > curNode->g)
								{
									target->prev = curNode;
								}
							}
							else
							{
								target->prev = curNode;
								NodeStraightCalculate(target);
								openList.push_back(target);

							}
						}
					}
				}
				//상 인지
				else if (pPrev->y > curNode->y && pPrev->x == curNode->x)
				{
					//기본 - 좌우가 장애물이 있다가 없어 졌는지
					stNode* upNode = UUSearch(curNode);
					if (upNode != nullptr)
					{
						openList.push_back(upNode);
					}
					//강제 - curNode 왼쪽에 장애물이 있었을 경우
					if (IsBlockingNode(GetNodeInMap(curNode->x - 1, curNode->y)) == true)
					{
						//왼쪽 상단이 오픈노드인지, 블럭인지 확인
						stNode* target = GetNodeInMap(curNode->x - 1, curNode->y - 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsClosedNode(target) == false)
						{
							if (IsOpenedNode(target) == true)
							{
								if (target->prev->g > curNode->g)
								{
									target->prev = curNode;
								}
							}
							else
							{

								target->prev = curNode;
								NodeStraightCalculate(target);
								openList.push_back(target);
							}
						}
					}
					//	   - curNode 오른쪽에 장애물이 있었을 경우
					if (IsBlockingNode(GetNodeInMap(curNode->x + 1, curNode->y)) == true)
					{
						//우상이 오픈노드인지 블럭인지 확인
						stNode* target = GetNodeInMap(curNode->x + 1, curNode->y - 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsClosedNode(target) == false)
						{
							if (IsOpenedNode(target) == true)
							{
								if (target->prev->g > curNode->g)
								{
									target->prev = curNode;
								}
							}
							else
							{
								target->prev = curNode;
								NodeStraightCalculate(target);
								openList.push_back(target);
							}
							
						}
					}
				}
				//하 인지
				else if (pPrev->y < curNode->y && pPrev->x == curNode->x)
				{
					//기본 - 좌우가 장애물이 있다가 없어 졌는지
					stNode* downNode = DDSearch(curNode);
					if (downNode != nullptr)
					{
						openList.push_back(downNode);
					}
					//강제 - curNode 왼쪽에 장애물이 있었을 경우
					if (IsBlockingNode(GetNodeInMap(curNode->x - 1, curNode->y)) == true)
					{
						//왼쪽 상단이 오픈노드인지, 블럭인지 확인
						stNode* target = GetNodeInMap(curNode->x - 1, curNode->y + 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsClosedNode(target) == false)
						{
							if (IsOpenedNode(target) == true)
							{
								if (target->prev->g > curNode->g)
								{
									target->prev = curNode;
								}
							}
							else
							{

								target->prev = curNode;
								NodeStraightCalculate(target);
								openList.push_back(target);
							}
						}
					}
					//	   - curNode 오른쪽에 장애물이 있었을 경우
					if (IsBlockingNode(GetNodeInMap(curNode->x + 1, curNode->y)) == true)
					{
						//우상이 오픈노드인지 블럭인지 확인
						stNode* target = GetNodeInMap(curNode->x + 1, curNode->y + 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsClosedNode(target) == false)
						{
							if (IsOpenedNode(target) == true)
							{
								if (target->prev->g > curNode->g)
								{
									target->prev = curNode;
								}
							}
							else
							{
								target->prev = curNode;
								NodeStraightCalculate(target);
								openList.push_back(target);
							}
						}
					}
				}
				//우상
				else if (pPrev->x < curNode->x && pPrev->y > curNode->y)
				{
					//우측
					stNode* right = RRSearch(curNode);
					if (right != nullptr)
					{
						openList.push_back(right);
					}
					//상측
					stNode* up = UUSearch(curNode);
					if (up != nullptr)
					{
						openList.push_back(up);
					}
					//대각선
					stNode* rightUpDiagonal = RUSearch(GetNodeInMap(curNode->x+1 , curNode->y-1 ), curNode);
					if (rightUpDiagonal != nullptr)
					{
						rightUpDiagonal->prev = curNode;
						NodeDiagonalCalculate(rightUpDiagonal);
						openList.push_back(rightUpDiagonal);
					}
					//강제 우하
					//내 아래가 블럭인데, 그 우측이 비어 있고,그 우측이 close 노드가아니고 
					stNode* rdForceNode = GetNodeInMap(curNode->x + 1, curNode->y + 1);
					if (rdForceNode!=nullptr
						&&NewIsBlockingNode(GetNodeInMap(curNode->x, curNode->y + 1)) == EFileResultType::True
						&&NewIsBlockingNode(rdForceNode) == EFileResultType::False
						&&IsClosedNode(rdForceNode)==false)
					{
						//또 open 노드인데 그 부모랑 나랑 비교 할때 내가 더 시작점에서 가까울때
						if (IsOpenedNode(rdForceNode)==true)
						{
							if (rdForceNode->prev->g>curNode->g)
							{
								rdForceNode->prev = curNode;
							}
						}
						else
						{
							rdForceNode->prev = curNode;
							NodeStraightCalculate(rdForceNode);
							openList.push_back(rdForceNode);
						}
					}
					//강제 좌상
					//내 좌측이 블럭인데, 그 위가 비어 있고, 그게 close 노드가 아니고 
					stNode* luForceNode = GetNodeInMap(curNode->x - 1, curNode->y-1 );
					if (luForceNode != nullptr
						&& NewIsBlockingNode(GetNodeInMap(curNode->x-1, curNode->y)) == EFileResultType::True
						&& NewIsBlockingNode(luForceNode) == EFileResultType::False
						&& IsClosedNode(luForceNode) == false)
					{
						//또 open 노드인데, 그 부모랑 나랑 비교 할때 내가 더 시작점에서 가까울때
						if (IsOpenedNode(luForceNode) == true)
						{
							if (luForceNode->prev->g > curNode->g)
							{
								luForceNode->prev = curNode;
							}
						}
						else
						{
							luForceNode->prev = curNode;
							NodeStraightCalculate(luForceNode);
							openList.push_back(luForceNode);
						}
					}
				}
				//우하
				else if (pPrev->x < curNode->x && pPrev->y < curNode->y)
				{
					//우측
 					stNode* right = RRSearch(curNode);
					if (right!=nullptr)
					{
						openList.push_back(right);
					}
					//하측
					stNode* down = DDSearch(curNode);
					if (down!=nullptr)
					{

						openList.push_back(down);
					}
					//대각선
					stNode* rightDownDiagonal = RDSearch(GetNodeInMap(curNode->x+1 , curNode->y+1 ), curNode);
					if (rightDownDiagonal != nullptr)
					{
						rightDownDiagonal->prev = curNode;
						NodeDiagonalCalculate(rightDownDiagonal);
						openList.push_back(rightDownDiagonal);
					}
					//강제 좌하 
					//내 좌측이 블럭인데 그 아래가 비어 있고, close 노드가 아니고, 
					stNode * ldForceNode = GetNodeInMap(curNode->x - 1, curNode->y + 1);
					if (ldForceNode != nullptr
						&& NewIsBlockingNode(GetNodeInMap(curNode->x - 1, curNode->y)) == EFileResultType::True
						&& NewIsBlockingNode(ldForceNode) == EFileResultType::False
						&& IsClosedNode(ldForceNode) == false)
					{
						//또 open 노드인데, 그 부모랑 나랑 비교 할때 내가 더 시작점에서 가까울때
						if (IsOpenedNode(ldForceNode) == true)
						{
							if (ldForceNode->prev->g > curNode->g)
							{
								ldForceNode->prev = curNode;
							}
						}
						else
						{
							ldForceNode->prev = curNode;
							NodeStraightCalculate(ldForceNode);
							openList.push_back(ldForceNode);
						}
					}
					//강제 우상
					//내 위가 블럭인데, 그 우측이 비어 있고, close 노드가 아니고
					stNode * ruForceNode = GetNodeInMap(curNode->x + 1, curNode->y - 1);
					if (ruForceNode != nullptr
						&& NewIsBlockingNode(GetNodeInMap(curNode->x, curNode->y-1)) == EFileResultType::True
						&& NewIsBlockingNode(ruForceNode) == EFileResultType::False
						&& IsClosedNode(ruForceNode) == false)
					{
						//또 open 노드인데, 그 부모랑 나랑 비교 할때 내가 더 시작점에서 가까울때
						if (IsOpenedNode(ruForceNode) == true)
						{
							if (ruForceNode->prev->g > curNode->g)
							{
								ruForceNode->prev = curNode;
							}
						}
						else
						{
							ruForceNode->prev = curNode;
							NodeStraightCalculate(ruForceNode);
							openList.push_back(ruForceNode);
						}
					}
				}
				//좌상
				else if (pPrev->x > curNode->x && pPrev->y > curNode->y)
				{
					//좌측
					stNode* left = LLSearch(curNode);
					if (left!=nullptr)
					{
						openList.push_back(left);
					}
					//상측
					stNode* up = UUSearch(curNode);
					if (up != nullptr)
					{
						openList.push_back(up);
					}
					//기본 - 대각선 이동, 직접 다음 노드부터 할 것을 지정함
					stNode* leftUpDiagonal = LUSearch(GetNodeInMap(curNode->x-1 , curNode->y-1 ),curNode);
					if (leftUpDiagonal != nullptr)
					{
						leftUpDiagonal->prev = curNode;
						NodeDiagonalCalculate(leftUpDiagonal);
						openList.push_back(leftUpDiagonal);
					}
					//강제 좌하
					//내 아래가 블럭이고, 그 왼편이 비어 있고, close 노드가 아닐때
					stNode * ldForceNode = GetNodeInMap(curNode->x - 1, curNode->y + 1);
					if (ldForceNode != nullptr
						&& NewIsBlockingNode(GetNodeInMap(curNode->x, curNode->y+1)) == EFileResultType::True
						&& NewIsBlockingNode(ldForceNode) == EFileResultType::False
						&& IsClosedNode(ldForceNode) == false)
					{
						//또 open 노드인데, 그 부모랑 나랑 비교 할때 내가 더 시작점에서 가까울때
						if (IsOpenedNode(ldForceNode) == true)
						{
							if (ldForceNode->prev->g > curNode->g)
							{
								ldForceNode->prev = curNode;
							}
						}
						else
						{
							ldForceNode->prev = curNode;
							NodeStraightCalculate(ldForceNode);
							openList.push_back(ldForceNode);
						}
					}
					//강제 우상
					//내 우측이 블럭이고, 그 위가 비어 있고 close 노드가 아닐때
					stNode * ruForceNode = GetNodeInMap(curNode->x + 1, curNode->y - 1);
					if (ruForceNode != nullptr
						&& NewIsBlockingNode(GetNodeInMap(curNode->x+1, curNode->y )) == EFileResultType::True
						&& NewIsBlockingNode(ruForceNode) == EFileResultType::False
						&& IsClosedNode(ruForceNode) == false)
					{
						//또 open 노드인데, 그 부모랑 나랑 비교 할때 내가 더 시작점에서 가까울때
						if (IsOpenedNode(ruForceNode) == true)
						{
							if (ruForceNode->prev->g > curNode->g)
							{
								ruForceNode->prev = curNode;
							}
						}
						else
						{
							ruForceNode->prev = curNode;
							NodeStraightCalculate(ruForceNode);
							openList.push_back(ruForceNode);
						}
					}
				}
				//좌하
				else if (pPrev->x > curNode->x && pPrev->y < curNode->y)
				{
					//좌측
					stNode* left = LLSearch(curNode);
					if (left != nullptr)
					{
						openList.push_back(left);
					}
					//하측
					stNode* down = DDSearch(curNode);
					if (down != nullptr)
					{
						openList.push_back(down);
					}
					//대각선
					stNode* leftDownDiagonal = LDSearch(GetNodeInMap(curNode->x-1, curNode->y+1 ), curNode);
					if (leftDownDiagonal != nullptr)
					{
						openList.push_back(leftDownDiagonal);
					}
					//강제 좌상
					//내 위가 블럭이고, 그 우측이 비어 있고, close노드가 아닐때
					stNode * lfForceNode = GetNodeInMap(curNode->x - 1, curNode->y - 1);
					if (lfForceNode != nullptr
						&& NewIsBlockingNode(GetNodeInMap(curNode->x , curNode->y-1)) == EFileResultType::True
						&& NewIsBlockingNode(lfForceNode) == EFileResultType::False
						&& IsClosedNode(lfForceNode) == false)
					{
						//또 open 노드인데, 그 부모랑 나랑 비교 할때 내가 더 시작점에서 가까울때
						if (IsOpenedNode(lfForceNode) == true)
						{
							if (lfForceNode->prev->g > curNode->g)
							{
								lfForceNode->prev = curNode;
							}
						}
						else
						{
							lfForceNode->prev = curNode;
							NodeStraightCalculate(lfForceNode);
							openList.push_back(lfForceNode);
						}
					}
					//강제 우하
					//내 우측이 블럭이고, 그 아래가 비어 있고 close 노드가 아닐때
					stNode * rdForceNode = GetNodeInMap(curNode->x + 1, curNode->y + 1);
					if (rdForceNode != nullptr
						&& NewIsBlockingNode(GetNodeInMap(curNode->x+1, curNode->y)) == EFileResultType::True
						&& NewIsBlockingNode(rdForceNode) == EFileResultType::False
						&& IsClosedNode(rdForceNode) == false)
					{
						//또 open 노드인데, 그 부모랑 나랑 비교 할때 내가 더 시작점에서 가까울때
						if (IsOpenedNode(rdForceNode) == true)
						{
							if (rdForceNode->prev->g > curNode->g)
							{
								rdForceNode->prev = curNode;
							}
						}
						else
						{
							rdForceNode->prev = curNode;
							NodeStraightCalculate(rdForceNode);
							openList.push_back(rdForceNode);
						}
					}
				}
				

			}
			openList.sort([](const stNode* lhs, const stNode* rhs) {
				return lhs->f < rhs->f;
				});
			

			//현재 노드를 close 하기
			closeList.push_back(curNode);
			//오픈에서 제거
			openList.remove(curNode);

			if (openList.size() == 0)
			{
				return EFileResultType::Error;
			}
			curNode = openList.front();


			return EFileResultType::None;
		}
		/// @brief 현재 상태를 화면에 출력하는 코드
		/// @param _func 화면에 출력하기 위한 람다 함수. 
		void PrintAll(function<void(stNode*, ENodeType)> _func)
		{
			if (_func == nullptr)
			{
				return;
			}
			//map node 전부 출력
			for (int y = 0; y < mapSizeY; ++y)
			{
				for (int x = 0; x < mapSizeX; ++x)
				{
					_func(&(map[y][x]), ENodeType::NomalMapNode);
				}
			}
			//확인한 노드 그리기
			list<stNode*>::iterator relayNodeitr = checkedNodeList.begin();
			for (; relayNodeitr != checkedNodeList.end(); ++relayNodeitr)
			{
				if ((*relayNodeitr) == nullptr)
				{
					continue;
				}
				_func((*relayNodeitr), ENodeType::RelayNode);
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
			while (node!= nullptr)
			{
				_func(node, ENodeType::MakePathNode);
				node = node->prev;
			}
		}
	private:

		stNode* RRSearch(stNode* _pTargetNode)
		{
			if (_pTargetNode==nullptr)
			{
				return nullptr;
			}

			stNode* pTraverNode = _pTargetNode;
			while (true)
			{
				stNode* pCurNode = GetNodeInMap(pTraverNode->x + 1, pTraverNode->y);
				if (pCurNode ==nullptr 
					|| IsBlockingNode(pCurNode)==true 
					|| IsClosedNode(pCurNode)==true)
				{
					return nullptr;
				}
				else if (IsOpenedNode(pCurNode)==true)
				{
					return nullptr;
				}
				checkedNodeList.push_back(pCurNode);
				//목적지 일경우 바로 반환
				if (IsCurNodeisDestNode(pCurNode,destNode)==true)
				{
					pCurNode->prev = _pTargetNode;
					NodeStraightCalculate(pCurNode);
					return pCurNode;
				}
				//위쪽이 장애물이 있다 없어졌는지
				if (pTraverNode != _pTargetNode && NewIsBlockingNode(GetNodeInMap(pCurNode->x-1, pCurNode->y - 1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x, pCurNode->y - 1)) == EFileResultType::False)
				{
					pTraverNode->prev = _pTargetNode;
					NodeStraightCalculate(pTraverNode);
					return pTraverNode;
				}
				//아래쪽이 장애물이 있다 없어졌는지
				else if (pTraverNode != _pTargetNode&&NewIsBlockingNode(GetNodeInMap(pCurNode->x-1, pCurNode->y + 1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x, pCurNode->y + 1)) == EFileResultType::False)
				{
					pTraverNode->prev = _pTargetNode;
					NodeStraightCalculate(pTraverNode);
					return pTraverNode;
				}
				//강제 노드
				//다음 노드가 블럭이고 그 위나 아래가 비어 있다면
				if ((NewIsBlockingNode(GetNodeInMap(pCurNode->x +1, pCurNode->y )) == EFileResultType::True)
					&&(NewIsBlockingNode(GetNodeInMap(pCurNode->x+1, pCurNode->y-1)) == EFileResultType::False|| NewIsBlockingNode(GetNodeInMap(pCurNode->x+1, pCurNode->y+1)) == EFileResultType::False)
					)
				{
					pCurNode->prev = _pTargetNode;
					NodeStraightCalculate(pCurNode);
					return pCurNode;
				}
				
				
				pTraverNode = pCurNode;
			}
			return nullptr;
		}
		stNode* LLSearch(stNode* _pTargetNode)
		{
			if (_pTargetNode == nullptr)
			{
				return nullptr;
			}
			stNode* pTraverNode = _pTargetNode;
			while (true)
			{
				stNode* pCurNode = GetNodeInMap(pTraverNode->x - 1, pTraverNode->y);
				if (pCurNode == nullptr || IsBlockingNode(pCurNode) == true || IsClosedNode(pCurNode) == true)
				{
					return nullptr;
				}
				else if (IsOpenedNode(pCurNode) == true)
				{
					return nullptr;
				}
				checkedNodeList.push_back(pCurNode);
				//목적지 일경우 바로 반환
				if (IsCurNodeisDestNode(pCurNode, destNode) == true)
				{
					pCurNode->prev = _pTargetNode;
					NodeStraightCalculate(pCurNode);
					return pCurNode;
				}
				//위쪽이 장애물이 있다 없어졌는지
				if (pTraverNode != _pTargetNode && NewIsBlockingNode(GetNodeInMap(pCurNode->x+1, pCurNode->y - 1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x, pCurNode->y - 1)) == EFileResultType::False)
				{
					pTraverNode->prev = _pTargetNode;
					NodeStraightCalculate(pTraverNode);
					return pTraverNode;
				}
				//아래쪽이 장애물이 있다 없어졌는지
				else if (pTraverNode != _pTargetNode && NewIsBlockingNode(GetNodeInMap(pCurNode->x+1, pCurNode->y + 1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x, pCurNode->y + 1)) == EFileResultType::False)
				{
					pTraverNode->prev = _pTargetNode;
					NodeStraightCalculate(pTraverNode);
					return pTraverNode;
				}


				//강제 노드
				//다음 노드가 블럭이고 그 위나 아래가 비어 있다면
				if ((NewIsBlockingNode(GetNodeInMap(pCurNode->x - 1, pCurNode->y)) == EFileResultType::True)
					&& (NewIsBlockingNode(GetNodeInMap(pCurNode->x - 1, pCurNode->y - 1)) == EFileResultType::False || NewIsBlockingNode(GetNodeInMap(pCurNode->x - 1, pCurNode->y + 1)) == EFileResultType::False)
					)
				{
					pCurNode->prev = _pTargetNode;
					NodeStraightCalculate(pCurNode);
					return pCurNode;
				}
				pTraverNode = pCurNode;
			}
			return nullptr;
		}
		stNode* UUSearch(stNode* _pTargetNode)
		{
			if (_pTargetNode == nullptr)
			{
				return nullptr;
			}
			stNode* pTraverNode = _pTargetNode;
			while (true)
			{
				stNode* pCurNode = GetNodeInMap(pTraverNode->x, pTraverNode->y - 1);
				if (pCurNode == nullptr || IsBlockingNode(pCurNode) == true || IsClosedNode(pCurNode) == true)
				{
					return nullptr;
				}
				else if (IsOpenedNode(pCurNode) == true)
				{
					return nullptr;
				}
				checkedNodeList.push_back(pCurNode);//지나갔는지 확인하는 용도
				//목적지 일경우 바로 반환
				if (IsCurNodeisDestNode(pCurNode, destNode) == true)
				{
					pCurNode->prev = _pTargetNode;
					NodeStraightCalculate(pCurNode);
					return pCurNode;
				}
				//왼쪽이 장애물이 있다 없어졌는지
				if (pTraverNode != _pTargetNode && NewIsBlockingNode(GetNodeInMap(pCurNode->x - 1, pCurNode->y+1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x - 1, pCurNode->y)) == EFileResultType::False)
				{
					pTraverNode->prev = _pTargetNode;
					NodeStraightCalculate(pTraverNode);
					return pTraverNode;
				}
				//오른쪽이 장애물이 있다 없어졌는지
				else if (pTraverNode != _pTargetNode && NewIsBlockingNode(GetNodeInMap(pCurNode->x + 1, pCurNode->y+1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x + 1, pCurNode->y)) == EFileResultType::False)
				{
					pTraverNode->prev = _pTargetNode;
					NodeStraightCalculate(pTraverNode);
					return pTraverNode;
				}

				//강제 노드
				//다음 노드가 블럭이고 그 위나 아래가 비어 있다면
				if ((NewIsBlockingNode(GetNodeInMap(pCurNode->x, pCurNode->y-1)) == EFileResultType::True)
					&& (NewIsBlockingNode(GetNodeInMap(pCurNode->x - 1, pCurNode->y - 1)) == EFileResultType::False || NewIsBlockingNode(GetNodeInMap(pCurNode->x + 1, pCurNode->y - 1)) == EFileResultType::False)
					)
				{
					pCurNode->prev = _pTargetNode;
					NodeStraightCalculate(pCurNode);
					return pCurNode;
				}
				pTraverNode = pCurNode;
			}
			return nullptr;
		}
		stNode* DDSearch(stNode* _pTargetNode)
		{
			if (_pTargetNode == nullptr)
			{
				return nullptr;
			}

			stNode* pTraverNode = _pTargetNode;
			while (true)
			{
				stNode* pCurNode = GetNodeInMap(pTraverNode->x, pTraverNode->y + 1);
				if (pCurNode == nullptr || IsBlockingNode(pCurNode) == true || IsClosedNode(pCurNode) == true)
				{
					return nullptr;
				}
				else if (IsOpenedNode(pCurNode) == true)
				{
					return nullptr;
				}
				checkedNodeList.push_back(pCurNode);
				//목적지 일경우 바로 반환
				if (IsCurNodeisDestNode(pCurNode, destNode) == true)
				{
					pCurNode->prev = _pTargetNode;
					NodeStraightCalculate(pCurNode);
					return pCurNode;
				}
				//왼쪽이 장애물이 있다 없어졌는지
				if (pTraverNode!= _pTargetNode&&NewIsBlockingNode(GetNodeInMap(pCurNode->x - 1, pCurNode->y-1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x - 1, pCurNode->y)) == EFileResultType::False)
				{
					pTraverNode->prev = _pTargetNode;
					NodeStraightCalculate(pTraverNode);
					return pTraverNode;
				}
				//오른쪽이 장애물이 있다 없어졌는지
				else if (pTraverNode != _pTargetNode && NewIsBlockingNode(GetNodeInMap(pCurNode->x + 1, pCurNode->y-1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x + 1, pCurNode->y)) == EFileResultType::False)
				{
					pTraverNode->prev = _pTargetNode;
					NodeStraightCalculate(pTraverNode);
					return pTraverNode;
				}
				//강제 노드
				//다음 노드가 블럭이고 그 위나 아래가 비어 있다면
				if ((NewIsBlockingNode(GetNodeInMap(pCurNode->x , pCurNode->y+1)) == EFileResultType::True)
					&& (NewIsBlockingNode(GetNodeInMap(pCurNode->x - 1, pCurNode->y + 1)) == EFileResultType::False || NewIsBlockingNode(GetNodeInMap(pCurNode->x + 1, pCurNode->y + 1)) == EFileResultType::False)
					)
				{
					pCurNode->prev = _pTargetNode;
					NodeStraightCalculate(pCurNode);
					return pCurNode;
				}

				pTraverNode = pCurNode;
			}

			return nullptr;
		}



		stNode* RUSearch(stNode* _pTargetNode, stNode* parents)
		{
			if (_pTargetNode == nullptr)
			{
				return nullptr;
			}

			stNode* pTraverNode = _pTargetNode;
			stNode* pCurNode = pTraverNode;
			while (true)
			{
				//stNode* pCurNode = GetNodeInMap(pTraverNode->x + 1, pTraverNode->y - 1);
				if (pCurNode == nullptr || IsBlockingNode(pCurNode) == true || IsClosedNode(pCurNode) == true)
				{
					return nullptr;
				}
				else if (IsOpenedNode(pCurNode) == true)
				{
					return nullptr;
				}
				checkedNodeList.push_back(pCurNode);
				//목적지 일경우 바로 반환
				if (IsCurNodeisDestNode(pCurNode, destNode) == true)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}

				//우측
				stNode* right = RRSearch(pCurNode);
				if(right!=nullptr)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}
				//상측
				stNode* up = UUSearch(pCurNode);
				if (up != nullptr)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}
				//강제 노드 할 곳이 있다면?
				//내 기준 아래가 블럭이고 그 블럭 옆이 비어 있다면?
				if (NewIsBlockingNode(GetNodeInMap(pCurNode->x , pCurNode->y + 1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x+1, pCurNode->y + 1)) == EFileResultType::False)
				{
					return pCurNode;
				}
				// 내 기준 좌측이 블럭이고 그 위가 비어있다면?
				if (NewIsBlockingNode(GetNodeInMap(pCurNode->x-1, pCurNode->y)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x -1 , pCurNode->y -1 )) == EFileResultType::False)
				{
					return pCurNode;
				}

				pCurNode = GetNodeInMap(pTraverNode->x + 1, pTraverNode->y - 1);
				pTraverNode = pCurNode;
			}


			return nullptr;
		}
		stNode* RDSearch(stNode* _pTargetNode, stNode* parents)
		{
			if (_pTargetNode == nullptr)
			{
				return nullptr;
			}

			stNode* pTraverNode = _pTargetNode;
			stNode* pCurNode = pTraverNode;

			while (true)
			{
				//stNode* pCurNode = GetNodeInMap(pTraverNode->x + 1, pTraverNode->y + 1);
				if (pCurNode == nullptr || IsBlockingNode(pCurNode) == true || IsClosedNode(pCurNode) == true)
				{
					return nullptr;
				}
				else if (pTraverNode!= _pTargetNode&&IsOpenedNode(pCurNode) == true)
				{
					//부모 경로 f가 내 f 보다 
					return nullptr;
				}
				checkedNodeList.push_back(pCurNode);
				//목적지 일경우 바로 반환
				if (IsCurNodeisDestNode(pCurNode, destNode) == true)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);

					return pCurNode;
				}

				//우측
				stNode* right = RRSearch(pCurNode);
				if (right != nullptr)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);

					return pCurNode;
				}
				//하측
				stNode* down = DDSearch(pCurNode);
				if (down != nullptr)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);

					return pCurNode;
				}

				//강제 노드 할 곳이 있다면?
				//내 기준 위가 블럭이고 그 우측이 비어 있다면?
				if (NewIsBlockingNode(GetNodeInMap(pCurNode->x, pCurNode->y - 1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x + 1, pCurNode->y - 1)) == EFileResultType::False)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}
				// 내 기준 좌측이 블럭이고 그 아래가 비어 있다면?
				if (NewIsBlockingNode(GetNodeInMap(pCurNode->x - 1, pCurNode->y)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x - 1, pCurNode->y + 1)) == EFileResultType::False)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}

				pCurNode = GetNodeInMap(pTraverNode->x + 1, pTraverNode->y + 1);
				pTraverNode = pCurNode;
			}

			return nullptr;
		}
		stNode* LUSearch(stNode* _pTargetNode, stNode* parents)
		{
			if (_pTargetNode == nullptr)
			{
				return nullptr;
			}

			stNode* pTraverNode = _pTargetNode;
			stNode* pCurNode = pTraverNode;//GetNodeInMap(pTraverNode->x - 1, pTraverNode->y - 1);
			while (true)
			{
				if (pCurNode == nullptr || IsBlockingNode(pCurNode) == true || IsClosedNode(pCurNode) == true)
				{
					return nullptr;
				}
				else if (IsOpenedNode(pCurNode) == true)
				{
					return nullptr;
				}
				checkedNodeList.push_back(pCurNode);
				//목적지 일경우 바로 반환
				if (IsCurNodeisDestNode(pCurNode, destNode) == true)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}

				//좌측
				stNode* left = LLSearch(pCurNode);
				if (left != nullptr)
				{
					pCurNode->prev = parents;

					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}
				//상측
				stNode* up = UUSearch(pCurNode);
				if (up != nullptr)
				{
					pCurNode->prev = parents;

					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}

				//강제 노드 할 곳이 있다면?
				//내 기준 아래가 블럭이고 그 왼편이 비어있다면?
				if (NewIsBlockingNode(GetNodeInMap(pCurNode->x, pCurNode->y + 1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x - 1, pCurNode->y + 1)) == EFileResultType::False)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}
				// 내 기준 우측이 블럭이고 그 위가 비어 있다면?
				if (NewIsBlockingNode(GetNodeInMap(pCurNode->x + 1, pCurNode->y)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x + 1, pCurNode->y - 1)) == EFileResultType::False)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}

				pCurNode = GetNodeInMap(pTraverNode->x - 1, pTraverNode->y - 1);
				pTraverNode = pCurNode;
			}
			return nullptr;
		}
		stNode* LDSearch(stNode* _pTargetNode, stNode* parents)
		{
			if (_pTargetNode == nullptr)
			{
				return nullptr;
			}

			stNode* pTraverNode = _pTargetNode;
			stNode* pCurNode = pTraverNode;
			while (true)
			{
				//stNode* pCurNode = GetNodeInMap(pTraverNode->x -1, pTraverNode->y + 1);
				if (pCurNode == nullptr || IsBlockingNode(pCurNode) == true || IsClosedNode(pCurNode) == true)
				{
					return nullptr;
				}
				else if (IsOpenedNode(pCurNode) == true)
				{
					return nullptr;
				}
				checkedNodeList.push_back(pCurNode);
				//목적지 일경우 바로 반환
				if (IsCurNodeisDestNode(pCurNode, destNode) == true)
				{
					pCurNode->prev = parents;

					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}

				//좌측
				stNode* left = LLSearch(pCurNode);
				if (left != nullptr)
				{
					pCurNode->prev = parents;

					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}
				//하측
				stNode* down = DDSearch(pCurNode);
				if (down != nullptr)
				{
					pCurNode->prev = parents;

					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}

				//강제 노드 할 곳이 있다면?
				//내 기준 위가 블럭이고 그 왼편이 비어 있다면?
				if (NewIsBlockingNode(GetNodeInMap(pCurNode->x, pCurNode->y - 1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x - 1, pCurNode->y - 1)) == EFileResultType::False)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}
				// 내 기준 우측이 블럭이고 그 아래가 비어 있다면?
				if (NewIsBlockingNode(GetNodeInMap(pCurNode->x + 1, pCurNode->y)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x + 1, pCurNode->y + 1)) == EFileResultType::False)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}
				pCurNode = GetNodeInMap(pTraverNode->x - 1, pTraverNode->y + 1);
				pTraverNode = pCurNode;
			}

			return nullptr;
		} // 수정중

		//노드의 가중치를 계산하는 함수
		void NodeStraightCalculate(stNode* _pTargetNode)
		{
			_pTargetNode->g = (_pTargetNode->prev == nullptr) ? 1 : _pTargetNode->prev->g + 1;
			_pTargetNode->h = CalH(destNode, _pTargetNode);
			_pTargetNode->f = _pTargetNode->g + _pTargetNode->h;
		}
		void NodeDiagonalCalculate(stNode* _pTargetNode)
		{
			_pTargetNode->g = (_pTargetNode->prev == nullptr) ? 1.5 : _pTargetNode->prev->g + 1.5;
			_pTargetNode->h = CalH(destNode, _pTargetNode);
			_pTargetNode->f = _pTargetNode->g + _pTargetNode->h;
		}
		/// @brief 사각형 맵을 만드는 함수
		/// @param _x 가로 사이즈
		/// @param _y 세로 사이즈
		void MakeSquareMap(int _x, int _y)
		{
			map = new stNode* [_y];
			for (int y = 0; y < _y; y++)
			{
				map[y] = new stNode[_x];
				for (int x=0;x<_x;++x)
				{
					map[y][x].x = x;
					map[y][x].y = y;
				}
			}
		}
		/// @brief map에 있는 정보들을 전부 초기화 해주는 함수
		void ClearMapData()
		{
			for (int y = 0; y < mapSizeY; ++y)
			{
				for (int x = 0; x < mapSizeY; ++x)
				{
					map[y][x]. f = 0;
					map[y][x].g = 0;
					map[y][x].h = 0;
					map[y][x].prev = nullptr;
				}
			}
		}
		/// @brief map 리스트에 있는 모든 정보 삭제 함수 메모리 까지 전부 반환함
		void DeleteAllMapNode()
		{
			for (int y = 0; y < mapSizeY; ++y)
			{
				delete[] map[y];
			}
		}
		/// @brief map 리스트의 특정 좌표에 node가 존재하는지
		/// @param _x 
		/// @param _y 
		/// @return 찾았다면 해당 node를 반환함, 없다면 nullptr을 반환함.
		stNode* GetNodeInMap(int _x, int _y)
		{
			if ((_x<0||_x>=mapSizeX) || (_y < 0 || _y >= mapSizeY))
			{
				return nullptr;
			}
			stNode* result = &(map[_y][_x]);
			return result;
		}

		/// @brief 노드가 blocking 노드인지 판단하는 함수
		/// @param _node 
		/// @return 
		bool IsBlockingNode(stNode* _node)
		{
			if (_node==nullptr)
			{
				return false;
			}
			list<stNode*>::iterator itr = blockList.begin();
			for (; itr != blockList.end(); ++itr)
			{
				if ((*itr) != nullptr && (*itr)->x == _node->x && (*itr)->y == _node->y)
				{
					return true;
				}
			}
			return false;
		}
		EFileResultType NewIsBlockingNode(stNode* _node)
		{
			if (_node == nullptr)
			{
				return EFileResultType::Error;
			}
			list<stNode*>::iterator itr = blockList.begin();
			for (; itr != blockList.end(); ++itr)
			{
				if ((*itr) != nullptr && (*itr)->x == _node->x && (*itr)->y == _node->y)
				{
					return EFileResultType::True;
				}
			}
			return EFileResultType::False;
		}
		bool IsClosedNode(stNode* _node)
		{
			if (_node==nullptr)
			{
				return false;
			}
			list<stNode*>::iterator itr = closeList.begin();
			for (; itr != closeList.end(); ++itr)
			{
				if ((*itr) != nullptr && (*itr)->x == _node->x && (*itr)->y == _node->y)
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
		int CalH(stNode* _destNode, stNode* _node)
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
			if (_curNode->x == _destNode->x && _curNode->y == _destNode->y)
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
			node = GetNodeInMap(_targetNode->x, _targetNode->y - 1);
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
			//우측 위
			node = GetNodeInMap(_targetNode->x + 1, _targetNode->y - 1);
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
			//우측
			node = GetNodeInMap(_targetNode->x + 1, _targetNode->y);
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
			node = GetNodeInMap(_targetNode->x + 1, _targetNode->y + 1);
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
			node = GetNodeInMap(_targetNode->x, _targetNode->y + 1);
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
			node = GetNodeInMap(_targetNode->x - 1, _targetNode->y + 1);
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
			//좌측
			node = GetNodeInMap(_targetNode->x - 1, _targetNode->y);
			if (node != nullptr)
			{
				if (IsBlockingNode(node) == false)
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
			node = GetNodeInMap(_targetNode->x - 1, _targetNode->y - 1);
			if (node != nullptr)
			{
				if (IsBlockingNode(node) == false)
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
			if (newPrev != nullptr)
			{
				_targetNode->prev = newPrev;
				_targetNode->g = newPrev->g + addValue;
				_targetNode->f = _targetNode->g + _targetNode->h;
			}
		}
	};
}

