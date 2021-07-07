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
		MakePathNode,//�� �����Ҷ� ����� ��
		RelayNode,//�߰� ����
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
		LL, //����
		LD, //����
		DD, //��
		RD, // ����
		RR, // ��
		RU,//���
		UU, // ��
		LU // �»�
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

	/// @brief Astart �˰������� ��ã�� Ŭ����, 
	/// Ư¡���� ��ã�� ������ Ŭ�����͸��� ���� ���� ���� �˻��� �����ϵ��� �� ����. �׷��� ������ map  �̶�� ���� �̷���� ���� �ʿ���
	/// 3*3�� ������ �ִ� ������ �ϳ��� ���� ����ؼ� ó���� ����.
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
		list<stNode*> openList;//������ ���� ���� ����Ʈ
		list<stNode*> closeList;//�̹� �� ���� ���� ����Ʈ
		list<stNode*> blockList;//���ع��� ���� ����Ʈ
		stNode** map;//��ü ��忡 ���� ����Ʈ
		list<stNode*> checkedNodeList;
		int mapSizeX;
		int mapSizeY;
	public:
		/// @brief map�� �޸𸮸� ���� ä�� ��� ������ �ʱ�ȭ��
		void InitializeAStar()
		{
			closeList.clear();
			openList.clear();
			blockList.clear();
			checkedNodeList.clear();
			ClearMapData();
			curNode = startNode;
		}
		/// @brief ��ֹ� ������ map�� �޸𸮸� ���� ä�� ������ ������ �ʱ�ȭ��
		void ResetNodeNotBlock()
		{
			closeList.clear();
			openList.clear();
			checkedNodeList.clear();
			ClearMapData();
			curNode = startNode;
		}
		/// @brief ����� ��� ����
		/// @param _x 
		/// @param _y 
		/// @return false�� �������� ���� ����
		bool SetStartNode(int _x, int _y)
		{
			//�ش� ��ġ�� node�� �ִ��� Ȯ��
			stNode* node = GetNodeInMap(_x, _y);
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
			stNode* node = GetNodeInMap(_x, _y);
			if (node == nullptr)
			{
				return false;
			}
			//���� ���� ������ Ȯ��
			else if (startNode ==destNode)
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
			stNode* node = GetNodeInMap(_x, _y);
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

			//��ֹ� ����Ʈ�� �߰�
			blockList.push_back(node);
			return true;

		}

		/// @brief �ѹ� ���� ã�� �Լ�
		/// @return false ���� ã�� ���ߴٴ� �ǹ�
		EFileResultType FindPathOneStep()
		{
			if (curNode == nullptr) // ���� ��尡 ���� �����
			{
				return EFileResultType::Error;
			}
			
			//���� ��尡 ������ �������
			if (IsCurNodeisDestNode(curNode, destNode) == true)
			{
				return EFileResultType::True;
			}
			


			if (curNode->prev==nullptr) //ù ���� ���
			{
				//��
				{
					
					//�⺻ - �¿찡 ��ֹ��� �ִٰ� ���� ������
					stNode* upNode = UUSearch(curNode);
					if (upNode != nullptr)
					{
						openList.push_back(upNode);
					}
					//���� - curNode ���ʿ� ��ֹ��� �־��� ���
					if (IsBlockingNode(GetNodeInMap(curNode->x - 1, curNode->y)) == true)
					{
						//���� ����� ���³������, ������ Ȯ��
						stNode* target = GetNodeInMap(curNode->x - 1, curNode->y - 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsOpenedNode(target) == false)
						{
							target->prev = curNode;
							NodeStraightCalculate(target);
							openList.push_back(target);
						}
					}
					//	   - curNode �����ʿ� ��ֹ��� �־��� ���
					if (IsBlockingNode(GetNodeInMap(curNode->x + 1, curNode->y)) == true)
					{
						//����� ���³������ ������ Ȯ��
						stNode* target = GetNodeInMap(curNode->x + 1, curNode->y - 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsOpenedNode(target) == false)
						{
							target->prev = curNode;
							NodeStraightCalculate(target);
							openList.push_back(target);
						}
					}
				}
				//�Ʒ�
				{
					
					//�⺻ - �¿찡 ��ֹ��� �ִٰ� ���� ������
					stNode* downNode = DDSearch(curNode);
					if (downNode != nullptr)
					{
						openList.push_back(downNode);
					}
					//���� - curNode ���ʿ� ��ֹ��� �־��� ���
					if (IsBlockingNode(GetNodeInMap(curNode->x - 1, curNode->y)) == true)
					{
						//���� ����� ���³������, ������ Ȯ��
						stNode* target = GetNodeInMap(curNode->x - 1, curNode->y + 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsOpenedNode(target) == false)
						{
							target->prev = curNode;
							NodeStraightCalculate(target);
							openList.push_back(target);
						}
					}
					//	   - curNode �����ʿ� ��ֹ��� �־��� ���
					if (IsBlockingNode(GetNodeInMap(curNode->x + 1, curNode->y)) == true)
					{
						//����� ���³������ ������ Ȯ��
						stNode* target = GetNodeInMap(curNode->x + 1, curNode->y + 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsOpenedNode(target) == false)
						{
							target->prev = curNode;
							NodeStraightCalculate(target);
							openList.push_back(target);
						}
					}
				}
				//��
				{
					
					//�⺻ - ���Ʒ��� ��ֹ��� �ִٰ� ���� ������
					stNode* leftNode = LLSearch(curNode);
					if (leftNode != nullptr)
					{
						openList.push_back(leftNode);
					}
					//���� - curNode �ϴܿ� ��ֹ��� �־��� ���
					if (IsBlockingNode(GetNodeInMap(curNode->x, curNode->y+1)) == true)
					{
						//���� �ϴ��� ���³������, ������ Ȯ��
						stNode* target = GetNodeInMap(curNode->x-1, curNode->y+ 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsOpenedNode(target) == false)
						{
							target->prev = curNode;
							NodeStraightCalculate(target);
							openList.push_back(target);
						}
					}
					//     - curNode ��ܿ� ��ֹ��� �־��� ���
					if (IsBlockingNode(GetNodeInMap(curNode->x, curNode->y - 1)) == true)
					{
						//���� ����� ���³������, ������ Ȯ��
						stNode* target = GetNodeInMap(curNode->x-1, curNode->y - 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsOpenedNode(target) == false)
						{
							target->prev = curNode;
							NodeStraightCalculate(target);
							openList.push_back(target);
						}
					}
				}
				//��
				{

					//�⺻ - ���Ʒ��� ��ֹ��� �ִٰ� ���� ������
					stNode* leftNode = RRSearch(curNode);
					if (leftNode != nullptr)
					{
						openList.push_back(leftNode);
					}
					//���� - curNode �ϴܿ� ��ֹ��� �־��� ���
					if (IsBlockingNode(GetNodeInMap(curNode->x, curNode->y + 1)) == true)
					{
						//���� �ϴ��� ���³������, ������ Ȯ��
						stNode* target = GetNodeInMap(curNode->x + 1, curNode->y + 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsOpenedNode(target) == false)
						{
							target->prev = curNode;
							NodeStraightCalculate(target);
							openList.push_back(target);
						}
					}
					//     - curNode ��ܿ� ��ֹ��� �־��� ���
					if (IsBlockingNode(GetNodeInMap(curNode->x, curNode->y - 1)) == true)
					{
						//���� ����� ���³������, ������ Ȯ��
						stNode* target = GetNodeInMap(curNode->x + 1, curNode->y - 1);
						if (target != nullptr && IsBlockingNode(target) == false && IsOpenedNode(target) == false)
						{
							target->prev = curNode;
							NodeStraightCalculate(target);
							openList.push_back(target);
						}
					}
				}
				//�»�
				{
					//�⺻ - �밢�� �̵�, ���� ���� ������ �� ���� ������
					stNode* leftUpDiagonal = LUSearch(GetNodeInMap(curNode->x - 1, curNode->y - 1), curNode);
					if (leftUpDiagonal!=nullptr)
					{
						leftUpDiagonal->prev = curNode;
						NodeDiagonalCalculate(leftUpDiagonal);
						openList.push_back(leftUpDiagonal);
					}
				}
				//����
				{
					stNode* leftDownDiagonal = LDSearch(GetNodeInMap(curNode->x - 1, curNode->y + 1), curNode);
					if (leftDownDiagonal != nullptr)
					{
						leftDownDiagonal->prev = curNode;
						NodeDiagonalCalculate(leftDownDiagonal);
						openList.push_back(leftDownDiagonal);
					}
				}
				
				//���
				{
					stNode* rightUpDiagonal = RUSearch(GetNodeInMap(curNode->x + 1, curNode->y - 1), curNode);
					if (rightUpDiagonal != nullptr)
					{
						rightUpDiagonal->prev = curNode;
						NodeDiagonalCalculate(rightUpDiagonal);
						openList.push_back(rightUpDiagonal);
					}
				}
				//����
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
			else //ù ���� ��尡 �ƴ� ���
			{
				//�θ𿡼� ��� �Դ��� ������ �˾Ƴ�����.
 				stNode* pPrev = curNode->prev;
				//���� ��������
				if (pPrev->x < curNode->x && pPrev->y == curNode->y)
				{
					//�⺻ - ���Ʒ��� ��ֹ��� �ִٰ� ���� ������
					stNode* leftNode = RRSearch(curNode);
					if (leftNode != nullptr)
					{
						openList.push_back(leftNode);
					}
					//���� - curNode �ϴܿ� ��ֹ��� �־��� ���
					if (IsBlockingNode(GetNodeInMap(curNode->x, curNode->y + 1)) == true)
					{
						//���� �ϴ��� ���³������, ������ Ȯ��
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
					//     - curNode ��ܿ� ��ֹ��� �־��� ���
					if (IsBlockingNode(GetNodeInMap(curNode->x, curNode->y - 1)) == true)
					{
						//���� ����� ���³������, ������ Ȯ��
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
				//��������
				else if (pPrev->x > curNode->x && pPrev->y == curNode->y)
				{
					//�⺻ - ���Ʒ��� ��ֹ��� �ִٰ� ���� ������
					stNode* leftNode = LLSearch(curNode);
					if (leftNode != nullptr)
					{
						openList.push_back(leftNode);
					}
					//���� - curNode �ϴܿ� ��ֹ��� �־��� ���
					if (IsBlockingNode(GetNodeInMap(curNode->x, curNode->y + 1)) == true)
					{
						//���� �ϴ��� ���³������, ������ Ȯ��
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
					//     - curNode ��ܿ� ��ֹ��� �־��� ���
					if (IsBlockingNode(GetNodeInMap(curNode->x, curNode->y - 1)) == true)
					{
						//���� ����� ���³������, ������ Ȯ��
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
				//�� ����
				else if (pPrev->y > curNode->y && pPrev->x == curNode->x)
				{
					//�⺻ - �¿찡 ��ֹ��� �ִٰ� ���� ������
					stNode* upNode = UUSearch(curNode);
					if (upNode != nullptr)
					{
						openList.push_back(upNode);
					}
					//���� - curNode ���ʿ� ��ֹ��� �־��� ���
					if (IsBlockingNode(GetNodeInMap(curNode->x - 1, curNode->y)) == true)
					{
						//���� ����� ���³������, ������ Ȯ��
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
					//	   - curNode �����ʿ� ��ֹ��� �־��� ���
					if (IsBlockingNode(GetNodeInMap(curNode->x + 1, curNode->y)) == true)
					{
						//����� ���³������ ������ Ȯ��
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
				//�� ����
				else if (pPrev->y < curNode->y && pPrev->x == curNode->x)
				{
					//�⺻ - �¿찡 ��ֹ��� �ִٰ� ���� ������
					stNode* downNode = DDSearch(curNode);
					if (downNode != nullptr)
					{
						openList.push_back(downNode);
					}
					//���� - curNode ���ʿ� ��ֹ��� �־��� ���
					if (IsBlockingNode(GetNodeInMap(curNode->x - 1, curNode->y)) == true)
					{
						//���� ����� ���³������, ������ Ȯ��
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
					//	   - curNode �����ʿ� ��ֹ��� �־��� ���
					if (IsBlockingNode(GetNodeInMap(curNode->x + 1, curNode->y)) == true)
					{
						//����� ���³������ ������ Ȯ��
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
				//���
				else if (pPrev->x < curNode->x && pPrev->y > curNode->y)
				{
					//����
					stNode* right = RRSearch(curNode);
					if (right != nullptr)
					{
						openList.push_back(right);
					}
					//����
					stNode* up = UUSearch(curNode);
					if (up != nullptr)
					{
						openList.push_back(up);
					}
					//�밢��
					stNode* rightUpDiagonal = RUSearch(GetNodeInMap(curNode->x+1 , curNode->y-1 ), curNode);
					if (rightUpDiagonal != nullptr)
					{
						rightUpDiagonal->prev = curNode;
						NodeDiagonalCalculate(rightUpDiagonal);
						openList.push_back(rightUpDiagonal);
					}
					//���� ����
					//�� �Ʒ��� ���ε�, �� ������ ��� �ְ�,�� ������ close ��尡�ƴϰ� 
					stNode* rdForceNode = GetNodeInMap(curNode->x + 1, curNode->y + 1);
					if (rdForceNode!=nullptr
						&&NewIsBlockingNode(GetNodeInMap(curNode->x, curNode->y + 1)) == EFileResultType::True
						&&NewIsBlockingNode(rdForceNode) == EFileResultType::False
						&&IsClosedNode(rdForceNode)==false)
					{
						//�� open ����ε� �� �θ�� ���� �� �Ҷ� ���� �� ���������� ����ﶧ
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
					//���� �»�
					//�� ������ ���ε�, �� ���� ��� �ְ�, �װ� close ��尡 �ƴϰ� 
					stNode* luForceNode = GetNodeInMap(curNode->x - 1, curNode->y-1 );
					if (luForceNode != nullptr
						&& NewIsBlockingNode(GetNodeInMap(curNode->x-1, curNode->y)) == EFileResultType::True
						&& NewIsBlockingNode(luForceNode) == EFileResultType::False
						&& IsClosedNode(luForceNode) == false)
					{
						//�� open ����ε�, �� �θ�� ���� �� �Ҷ� ���� �� ���������� ����ﶧ
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
				//����
				else if (pPrev->x < curNode->x && pPrev->y < curNode->y)
				{
					//����
 					stNode* right = RRSearch(curNode);
					if (right!=nullptr)
					{
						openList.push_back(right);
					}
					//����
					stNode* down = DDSearch(curNode);
					if (down!=nullptr)
					{

						openList.push_back(down);
					}
					//�밢��
					stNode* rightDownDiagonal = RDSearch(GetNodeInMap(curNode->x+1 , curNode->y+1 ), curNode);
					if (rightDownDiagonal != nullptr)
					{
						rightDownDiagonal->prev = curNode;
						NodeDiagonalCalculate(rightDownDiagonal);
						openList.push_back(rightDownDiagonal);
					}
					//���� ���� 
					//�� ������ ���ε� �� �Ʒ��� ��� �ְ�, close ��尡 �ƴϰ�, 
					stNode * ldForceNode = GetNodeInMap(curNode->x - 1, curNode->y + 1);
					if (ldForceNode != nullptr
						&& NewIsBlockingNode(GetNodeInMap(curNode->x - 1, curNode->y)) == EFileResultType::True
						&& NewIsBlockingNode(ldForceNode) == EFileResultType::False
						&& IsClosedNode(ldForceNode) == false)
					{
						//�� open ����ε�, �� �θ�� ���� �� �Ҷ� ���� �� ���������� ����ﶧ
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
					//���� ���
					//�� ���� ���ε�, �� ������ ��� �ְ�, close ��尡 �ƴϰ�
					stNode * ruForceNode = GetNodeInMap(curNode->x + 1, curNode->y - 1);
					if (ruForceNode != nullptr
						&& NewIsBlockingNode(GetNodeInMap(curNode->x, curNode->y-1)) == EFileResultType::True
						&& NewIsBlockingNode(ruForceNode) == EFileResultType::False
						&& IsClosedNode(ruForceNode) == false)
					{
						//�� open ����ε�, �� �θ�� ���� �� �Ҷ� ���� �� ���������� ����ﶧ
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
				//�»�
				else if (pPrev->x > curNode->x && pPrev->y > curNode->y)
				{
					//����
					stNode* left = LLSearch(curNode);
					if (left!=nullptr)
					{
						openList.push_back(left);
					}
					//����
					stNode* up = UUSearch(curNode);
					if (up != nullptr)
					{
						openList.push_back(up);
					}
					//�⺻ - �밢�� �̵�, ���� ���� ������ �� ���� ������
					stNode* leftUpDiagonal = LUSearch(GetNodeInMap(curNode->x-1 , curNode->y-1 ),curNode);
					if (leftUpDiagonal != nullptr)
					{
						leftUpDiagonal->prev = curNode;
						NodeDiagonalCalculate(leftUpDiagonal);
						openList.push_back(leftUpDiagonal);
					}
					//���� ����
					//�� �Ʒ��� ���̰�, �� ������ ��� �ְ�, close ��尡 �ƴҶ�
					stNode * ldForceNode = GetNodeInMap(curNode->x - 1, curNode->y + 1);
					if (ldForceNode != nullptr
						&& NewIsBlockingNode(GetNodeInMap(curNode->x, curNode->y+1)) == EFileResultType::True
						&& NewIsBlockingNode(ldForceNode) == EFileResultType::False
						&& IsClosedNode(ldForceNode) == false)
					{
						//�� open ����ε�, �� �θ�� ���� �� �Ҷ� ���� �� ���������� ����ﶧ
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
					//���� ���
					//�� ������ ���̰�, �� ���� ��� �ְ� close ��尡 �ƴҶ�
					stNode * ruForceNode = GetNodeInMap(curNode->x + 1, curNode->y - 1);
					if (ruForceNode != nullptr
						&& NewIsBlockingNode(GetNodeInMap(curNode->x+1, curNode->y )) == EFileResultType::True
						&& NewIsBlockingNode(ruForceNode) == EFileResultType::False
						&& IsClosedNode(ruForceNode) == false)
					{
						//�� open ����ε�, �� �θ�� ���� �� �Ҷ� ���� �� ���������� ����ﶧ
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
				//����
				else if (pPrev->x > curNode->x && pPrev->y < curNode->y)
				{
					//����
					stNode* left = LLSearch(curNode);
					if (left != nullptr)
					{
						openList.push_back(left);
					}
					//����
					stNode* down = DDSearch(curNode);
					if (down != nullptr)
					{
						openList.push_back(down);
					}
					//�밢��
					stNode* leftDownDiagonal = LDSearch(GetNodeInMap(curNode->x-1, curNode->y+1 ), curNode);
					if (leftDownDiagonal != nullptr)
					{
						openList.push_back(leftDownDiagonal);
					}
					//���� �»�
					//�� ���� ���̰�, �� ������ ��� �ְ�, close��尡 �ƴҶ�
					stNode * lfForceNode = GetNodeInMap(curNode->x - 1, curNode->y - 1);
					if (lfForceNode != nullptr
						&& NewIsBlockingNode(GetNodeInMap(curNode->x , curNode->y-1)) == EFileResultType::True
						&& NewIsBlockingNode(lfForceNode) == EFileResultType::False
						&& IsClosedNode(lfForceNode) == false)
					{
						//�� open ����ε�, �� �θ�� ���� �� �Ҷ� ���� �� ���������� ����ﶧ
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
					//���� ����
					//�� ������ ���̰�, �� �Ʒ��� ��� �ְ� close ��尡 �ƴҶ�
					stNode * rdForceNode = GetNodeInMap(curNode->x + 1, curNode->y + 1);
					if (rdForceNode != nullptr
						&& NewIsBlockingNode(GetNodeInMap(curNode->x+1, curNode->y)) == EFileResultType::True
						&& NewIsBlockingNode(rdForceNode) == EFileResultType::False
						&& IsClosedNode(rdForceNode) == false)
					{
						//�� open ����ε�, �� �θ�� ���� �� �Ҷ� ���� �� ���������� ����ﶧ
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
			

			//���� ��带 close �ϱ�
			closeList.push_back(curNode);
			//���¿��� ����
			openList.remove(curNode);

			if (openList.size() == 0)
			{
				return EFileResultType::Error;
			}
			curNode = openList.front();


			return EFileResultType::None;
		}
		/// @brief ���� ���¸� ȭ�鿡 ����ϴ� �ڵ�
		/// @param _func ȭ�鿡 ����ϱ� ���� ���� �Լ�. 
		void PrintAll(function<void(stNode*, ENodeType)> _func)
		{
			if (_func == nullptr)
			{
				return;
			}
			//map node ���� ���
			for (int y = 0; y < mapSizeY; ++y)
			{
				for (int x = 0; x < mapSizeX; ++x)
				{
					_func(&(map[y][x]), ENodeType::NomalMapNode);
				}
			}
			//Ȯ���� ��� �׸���
			list<stNode*>::iterator relayNodeitr = checkedNodeList.begin();
			for (; relayNodeitr != checkedNodeList.end(); ++relayNodeitr)
			{
				if ((*relayNodeitr) == nullptr)
				{
					continue;
				}
				_func((*relayNodeitr), ENodeType::RelayNode);
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
				//������ �ϰ�� �ٷ� ��ȯ
				if (IsCurNodeisDestNode(pCurNode,destNode)==true)
				{
					pCurNode->prev = _pTargetNode;
					NodeStraightCalculate(pCurNode);
					return pCurNode;
				}
				//������ ��ֹ��� �ִ� ����������
				if (pTraverNode != _pTargetNode && NewIsBlockingNode(GetNodeInMap(pCurNode->x-1, pCurNode->y - 1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x, pCurNode->y - 1)) == EFileResultType::False)
				{
					pTraverNode->prev = _pTargetNode;
					NodeStraightCalculate(pTraverNode);
					return pTraverNode;
				}
				//�Ʒ����� ��ֹ��� �ִ� ����������
				else if (pTraverNode != _pTargetNode&&NewIsBlockingNode(GetNodeInMap(pCurNode->x-1, pCurNode->y + 1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x, pCurNode->y + 1)) == EFileResultType::False)
				{
					pTraverNode->prev = _pTargetNode;
					NodeStraightCalculate(pTraverNode);
					return pTraverNode;
				}
				//���� ���
				//���� ��尡 ���̰� �� ���� �Ʒ��� ��� �ִٸ�
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
				//������ �ϰ�� �ٷ� ��ȯ
				if (IsCurNodeisDestNode(pCurNode, destNode) == true)
				{
					pCurNode->prev = _pTargetNode;
					NodeStraightCalculate(pCurNode);
					return pCurNode;
				}
				//������ ��ֹ��� �ִ� ����������
				if (pTraverNode != _pTargetNode && NewIsBlockingNode(GetNodeInMap(pCurNode->x+1, pCurNode->y - 1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x, pCurNode->y - 1)) == EFileResultType::False)
				{
					pTraverNode->prev = _pTargetNode;
					NodeStraightCalculate(pTraverNode);
					return pTraverNode;
				}
				//�Ʒ����� ��ֹ��� �ִ� ����������
				else if (pTraverNode != _pTargetNode && NewIsBlockingNode(GetNodeInMap(pCurNode->x+1, pCurNode->y + 1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x, pCurNode->y + 1)) == EFileResultType::False)
				{
					pTraverNode->prev = _pTargetNode;
					NodeStraightCalculate(pTraverNode);
					return pTraverNode;
				}


				//���� ���
				//���� ��尡 ���̰� �� ���� �Ʒ��� ��� �ִٸ�
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
				checkedNodeList.push_back(pCurNode);//���������� Ȯ���ϴ� �뵵
				//������ �ϰ�� �ٷ� ��ȯ
				if (IsCurNodeisDestNode(pCurNode, destNode) == true)
				{
					pCurNode->prev = _pTargetNode;
					NodeStraightCalculate(pCurNode);
					return pCurNode;
				}
				//������ ��ֹ��� �ִ� ����������
				if (pTraverNode != _pTargetNode && NewIsBlockingNode(GetNodeInMap(pCurNode->x - 1, pCurNode->y+1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x - 1, pCurNode->y)) == EFileResultType::False)
				{
					pTraverNode->prev = _pTargetNode;
					NodeStraightCalculate(pTraverNode);
					return pTraverNode;
				}
				//�������� ��ֹ��� �ִ� ����������
				else if (pTraverNode != _pTargetNode && NewIsBlockingNode(GetNodeInMap(pCurNode->x + 1, pCurNode->y+1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x + 1, pCurNode->y)) == EFileResultType::False)
				{
					pTraverNode->prev = _pTargetNode;
					NodeStraightCalculate(pTraverNode);
					return pTraverNode;
				}

				//���� ���
				//���� ��尡 ���̰� �� ���� �Ʒ��� ��� �ִٸ�
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
				//������ �ϰ�� �ٷ� ��ȯ
				if (IsCurNodeisDestNode(pCurNode, destNode) == true)
				{
					pCurNode->prev = _pTargetNode;
					NodeStraightCalculate(pCurNode);
					return pCurNode;
				}
				//������ ��ֹ��� �ִ� ����������
				if (pTraverNode!= _pTargetNode&&NewIsBlockingNode(GetNodeInMap(pCurNode->x - 1, pCurNode->y-1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x - 1, pCurNode->y)) == EFileResultType::False)
				{
					pTraverNode->prev = _pTargetNode;
					NodeStraightCalculate(pTraverNode);
					return pTraverNode;
				}
				//�������� ��ֹ��� �ִ� ����������
				else if (pTraverNode != _pTargetNode && NewIsBlockingNode(GetNodeInMap(pCurNode->x + 1, pCurNode->y-1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x + 1, pCurNode->y)) == EFileResultType::False)
				{
					pTraverNode->prev = _pTargetNode;
					NodeStraightCalculate(pTraverNode);
					return pTraverNode;
				}
				//���� ���
				//���� ��尡 ���̰� �� ���� �Ʒ��� ��� �ִٸ�
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
				//������ �ϰ�� �ٷ� ��ȯ
				if (IsCurNodeisDestNode(pCurNode, destNode) == true)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}

				//����
				stNode* right = RRSearch(pCurNode);
				if(right!=nullptr)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}
				//����
				stNode* up = UUSearch(pCurNode);
				if (up != nullptr)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}
				//���� ��� �� ���� �ִٸ�?
				//�� ���� �Ʒ��� ���̰� �� �� ���� ��� �ִٸ�?
				if (NewIsBlockingNode(GetNodeInMap(pCurNode->x , pCurNode->y + 1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x+1, pCurNode->y + 1)) == EFileResultType::False)
				{
					return pCurNode;
				}
				// �� ���� ������ ���̰� �� ���� ����ִٸ�?
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
					//�θ� ��� f�� �� f ���� 
					return nullptr;
				}
				checkedNodeList.push_back(pCurNode);
				//������ �ϰ�� �ٷ� ��ȯ
				if (IsCurNodeisDestNode(pCurNode, destNode) == true)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);

					return pCurNode;
				}

				//����
				stNode* right = RRSearch(pCurNode);
				if (right != nullptr)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);

					return pCurNode;
				}
				//����
				stNode* down = DDSearch(pCurNode);
				if (down != nullptr)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);

					return pCurNode;
				}

				//���� ��� �� ���� �ִٸ�?
				//�� ���� ���� ���̰� �� ������ ��� �ִٸ�?
				if (NewIsBlockingNode(GetNodeInMap(pCurNode->x, pCurNode->y - 1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x + 1, pCurNode->y - 1)) == EFileResultType::False)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}
				// �� ���� ������ ���̰� �� �Ʒ��� ��� �ִٸ�?
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
				//������ �ϰ�� �ٷ� ��ȯ
				if (IsCurNodeisDestNode(pCurNode, destNode) == true)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}

				//����
				stNode* left = LLSearch(pCurNode);
				if (left != nullptr)
				{
					pCurNode->prev = parents;

					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}
				//����
				stNode* up = UUSearch(pCurNode);
				if (up != nullptr)
				{
					pCurNode->prev = parents;

					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}

				//���� ��� �� ���� �ִٸ�?
				//�� ���� �Ʒ��� ���̰� �� ������ ����ִٸ�?
				if (NewIsBlockingNode(GetNodeInMap(pCurNode->x, pCurNode->y + 1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x - 1, pCurNode->y + 1)) == EFileResultType::False)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}
				// �� ���� ������ ���̰� �� ���� ��� �ִٸ�?
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
				//������ �ϰ�� �ٷ� ��ȯ
				if (IsCurNodeisDestNode(pCurNode, destNode) == true)
				{
					pCurNode->prev = parents;

					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}

				//����
				stNode* left = LLSearch(pCurNode);
				if (left != nullptr)
				{
					pCurNode->prev = parents;

					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}
				//����
				stNode* down = DDSearch(pCurNode);
				if (down != nullptr)
				{
					pCurNode->prev = parents;

					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}

				//���� ��� �� ���� �ִٸ�?
				//�� ���� ���� ���̰� �� ������ ��� �ִٸ�?
				if (NewIsBlockingNode(GetNodeInMap(pCurNode->x, pCurNode->y - 1)) == EFileResultType::True
					&& NewIsBlockingNode(GetNodeInMap(pCurNode->x - 1, pCurNode->y - 1)) == EFileResultType::False)
				{
					pCurNode->prev = parents;
					NodeDiagonalCalculate(pCurNode);
					return pCurNode;
				}
				// �� ���� ������ ���̰� �� �Ʒ��� ��� �ִٸ�?
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
		} // ������

		//����� ����ġ�� ����ϴ� �Լ�
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
		/// @brief �簢�� ���� ����� �Լ�
		/// @param _x ���� ������
		/// @param _y ���� ������
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
		/// @brief map�� �ִ� �������� ���� �ʱ�ȭ ���ִ� �Լ�
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
		/// @brief map ����Ʈ�� �ִ� ��� ���� ���� �Լ� �޸� ���� ���� ��ȯ��
		void DeleteAllMapNode()
		{
			for (int y = 0; y < mapSizeY; ++y)
			{
				delete[] map[y];
			}
		}
		/// @brief map ����Ʈ�� Ư�� ��ǥ�� node�� �����ϴ���
		/// @param _x 
		/// @param _y 
		/// @return ã�Ҵٸ� �ش� node�� ��ȯ��, ���ٸ� nullptr�� ��ȯ��.
		stNode* GetNodeInMap(int _x, int _y)
		{
			if ((_x<0||_x>=mapSizeX) || (_y < 0 || _y >= mapSizeY))
			{
				return nullptr;
			}
			stNode* result = &(map[_y][_x]);
			return result;
		}

		/// @brief ��尡 blocking ������� �Ǵ��ϴ� �Լ�
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
			if (_curNode->x == _destNode->x && _curNode->y == _destNode->y)
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
			//���� ��
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
			//����
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
			//���� �Ʒ�
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
			//�Ʒ�
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
			//���� �Ʒ�
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
			//����
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
			//���� ��
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

