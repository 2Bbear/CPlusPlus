#pragma once
#include<functional>
#include <assert.h>

namespace twobbearlib
{
	using namespace std;
	class RedBlackTree
	{
	public:
		enum class ENode_Color :int
		{
			Black = 0,
			StartNode
		};
		struct stNode
		{
			stNode()
			{
				pParent = nullptr;
				pLeft = nullptr;
				pRight = nullptr;
				corol = ENode_Color::StartNode;
				iData = 0;
			}
			stNode(
				stNode* _pParent,
				stNode* _pLeft,
				stNode* _pRight,
				ENode_Color _corol,
				int _data)
			{
				pParent = _pParent;
				pLeft = _pLeft;
				pRight = _pRight;
				corol = _corol;
				iData = _data;
			}
			struct stNode* pParent;
			struct stNode* pLeft;
			struct stNode* pRight;

			ENode_Color corol;
			int iData;

		};
		
	public:
		RedBlackTree()
		{
			rootNode = nullptr;

			nil.corol = ENode_Color::Black;
			nil.pLeft = nullptr;
			nil.pParent = nullptr;
			nil.pRight = nullptr;
		}
		~RedBlackTree()
		{

		}
	public:
		/// @brief 
		/// @param _data 
		/// @return when return is null is fail 
		bool Insert(int _data)
		{
			bool result = false;
			if (rootNode ==nullptr)
			{
				stNode* node = new stNode();
				node->corol = ENode_Color::Black;
				node->iData = _data;
				node->pLeft = &nil;
				node->pRight = &nil;
				rootNode = node;
				result = true;
				return result;
			}
			stNode* node = new stNode(nullptr,&nil,&nil,ENode_Color::StartNode,_data);
			RecursiveInsert(rootNode, node, result);
			return result;
		}
		bool Delete(int _data)
		{
			if (rootNode == nullptr)
			{
				return false;
			}
			bool result = false;
			RecursiveDelete(rootNode, _data,result);
			return result;
		}
		int check = -1;
		ENode_Color colorCheck = ENode_Color::Black;
		void PrintfAll()
		{
			system("cls");
			RecursiveInOrder(rootNode, [this](stNode* _node) {
				printf("%d %s %s %s\n",
					_node->iData,
					(_node->corol==ENode_Color::StartNode?"빨":"블"),
					(check == _node->iData ? "중복에러" : " "),
					(check > _node->iData ? "순서에러" : " "));

				check = _node->iData;
				colorCheck = _node->corol;
				});
			check = -1;
			colorCheck = ENode_Color::Black;
		}
		void PrintGUIAll(function<void(stNode*)> _firstfunc, function<void(stNode*)> _secondfunc)
		{
			if (rootNode==nullptr|| rootNode ==&nil)
			{
				return;
			}
			RecursivePreOrder(rootNode, _firstfunc, _secondfunc);
		}
		
	private:
		void RecursiveDelete(stNode* _targetNode, int _data, bool& result)
		{
			if (_targetNode == &nil)
			{
				return;
			}
			//left
			if (_targetNode->iData > _data)
			{
				RecursiveDelete(_targetNode->pLeft, _data, result);
				return;
			}
			//right
			else if (_targetNode->iData < _data)
			{
				RecursiveDelete(_targetNode->pRight, _data, result);
				return;
			}
			else
			{
				//삭제!
				stNode* pChLeft = _targetNode->pLeft;
				stNode* pChRight = _targetNode->pRight;
				//삭제하려는 노드의 자식 노드가 없는 경우
				if (pChLeft ==&nil && pChRight == &nil)
				{
					//부모의 좌측인지 우측인지
					stNode*  pParent=_targetNode->pParent;
					if (pParent->pLeft == _targetNode)
					{
						pParent->pLeft = &nil;
						pParent->pLeft->pParent = pParent;
						
					}
					else if (pParent->pRight == _targetNode)
					{
						pParent->pRight = &nil;
						pParent->pRight->pParent = pParent;
					}
					else//뭔가 연결이 안되어 있는 거임
					{
						assert(1);
						printf_s("error!! \n");
					}
					ENode_Color deletedNodeColor = _targetNode->corol;
					delete _targetNode;
					
					//어차피 마지막 노드로 nil을 붙였을 테니 nil을 기준으로 밸런싱을 한다.
					BalancingAfterDeleteNode(&nil, deletedNodeColor);

					result = true;
					return;
				}
				//삭제하려는 노드의 자식 노드가 하나 일때
				else if (pChLeft == &nil && pChRight != &nil)
				{
					//부모의 좌측인지 우측인지
					stNode* pParent = _targetNode->pParent;
					stNode* pChild = pChRight;
									
					if (pParent->pLeft == _targetNode)
					{
						pParent->pLeft = pChild;
					}
					else if (pParent->pRight == _targetNode)
					{
						pParent->pRight = pChild;
					}
					else//뭔가 연결이 안되어 있는 거임
					{
						assert(1);
						printf_s("error!! \n");
					}
					pChild->pParent = pParent;
					ENode_Color deletedNodeColor = _targetNode->corol;
					delete _targetNode;
					BalancingAfterDeleteNode(pChild, deletedNodeColor);
					result = true;
					return;
				}
				else if (pChLeft != &nil && pChRight == &nil)
				{
					//부모의 좌측인지 우측인지
					stNode* pParent = _targetNode->pParent;
					stNode* pChild = pChLeft;

					if (pParent->pLeft == _targetNode)
					{
						pParent->pLeft = pChild;
					}
					else if (pParent->pRight == _targetNode)
					{
						pParent->pRight = pChild;
					}
					else//뭔가 연결이 안되어 있는 거임
					{
						assert(1);
						printf_s("error!! \n");
					}
					pChild->pParent = pParent;
					ENode_Color deletedNodeColor = _targetNode->corol;
					delete _targetNode;
					BalancingAfterDeleteNode(pChild, deletedNodeColor);
					result = true;
					return;
				}

				//삭제하려는 노드의 자식이 둘다 있을때 왼쪽에서 가장 큰 노드를 올려버릴거임
				else if (pChLeft != &nil && pChRight != &nil)
				{
					stNode* pChild = _targetNode->pLeft;
					for (;;)
					{
						if (pChild->pRight!=&nil)
						{
							pChild = pChild->pRight;
							continue;
						}

						break;
					}

					
					stNode* pChildsParents = pChild->pParent;
					if (pChild->pLeft!=&nil)//붙여야할 왼쪽 노드가 있는 경우
					{
						if (pChildsParents== _targetNode) // 만약 지워야하는게 삭제 대상의 바로 아래에 있는 노드라면?
						{
							pChildsParents->pLeft = pChild->pLeft;
							pChild->pLeft->pParent = pChildsParents;
							_targetNode->iData = pChild->iData;
							ENode_Color deletedNodeColor = pChild->corol;
							BalancingAfterDeleteNode(pChildsParents->pLeft, deletedNodeColor);
						}
						else
						{
							pChildsParents->pRight = pChild->pLeft;
							pChild->pLeft->pParent = pChildsParents;
							_targetNode->iData = pChild->iData;
							ENode_Color deletedNodeColor = pChild->corol;
							BalancingAfterDeleteNode(pChildsParents->pRight, deletedNodeColor);
						}
					}
					else//붙여야할 왼쪽 노드가 없는 경우
					{
						if (pChildsParents == _targetNode) // 만약 지워야하는게 삭제 대상의 바로 아래에 있는 노드라면?
						{
							pChildsParents->pLeft = &nil;
							nil.pParent = pChildsParents;
							_targetNode->iData = pChild->iData;
						}
						else
						{
							pChildsParents->pRight = &nil;
							nil.pParent = pChildsParents;
							_targetNode->iData = pChild->iData;
						}
						ENode_Color deletedNodeColor = pChild->corol;
						BalancingAfterDeleteNode(&nil, deletedNodeColor);
					}
					delete pChild;
					
					result = true;
					return;
				}
			}
		}
		void TurnLeftNode(stNode* _targetNode)
		{
			stNode* targetParents = _targetNode->pParent;
			int targetParentsDirection = 0;
			if (targetParents != nullptr)
			{
				targetParentsDirection = (targetParents->pLeft == _targetNode) ? -1 : 1;
			}

			stNode* upNode = _targetNode->pRight;
			stNode* spandingNode = upNode->pLeft;

			upNode->pParent = targetParents;
			if (targetParentsDirection == -1)
			{
				targetParents->pLeft = upNode;
			}
			else if (targetParentsDirection == 1)
			{
				targetParents->pRight = upNode;
			}
			_targetNode->pParent = upNode;
			_targetNode->pRight = spandingNode;
			if (spandingNode!=nullptr)
			{
				spandingNode->pParent = _targetNode;
			}
			
			upNode->pLeft = _targetNode;

			if (_targetNode == rootNode)
			{
				rootNode = upNode;
				rootNode->corol = ENode_Color::Black;
			}

		}
		void TurnRightNode(stNode* _targetNode)
		{
			stNode* targetParents = _targetNode->pParent;
			int targetParentsDirection = 0;
			if (targetParents != nullptr)
			{
				targetParentsDirection = (targetParents->pLeft == _targetNode) ? -1 : 1;
			}

			stNode* upNode = _targetNode->pLeft;
			stNode* spandingNode = upNode->pRight;

			upNode->pParent = targetParents;
			if (targetParentsDirection == -1)
			{
				targetParents->pLeft = upNode;
			}
			else if (targetParentsDirection == 1)
			{
				targetParents->pRight = upNode;
			}
			_targetNode->pParent = upNode;
			_targetNode->pLeft = spandingNode;
			if (spandingNode!=nullptr)
			{
				spandingNode->pParent = _targetNode;

			}
			upNode->pRight = _targetNode;

			if (_targetNode == rootNode)
			{
				rootNode = upNode;
				rootNode->corol = ENode_Color::Black;
			}
		}
		void BalancingAfterInsertNode(stNode* _prentsNode, stNode* _insertNode)
		{
			if (_prentsNode==nullptr)
			{
				_insertNode->corol = ENode_Color::Black;
				return;
			}
			if (_prentsNode->corol==ENode_Color::Black|| _insertNode->corol==ENode_Color::Black&& _prentsNode->corol==ENode_Color::StartNode)
			{
				return;
			}
			stNode* uncleNode = (_prentsNode->pParent->pLeft == _prentsNode) ? _prentsNode->pParent->pRight : _prentsNode->pParent->pLeft;
			int direction = (_prentsNode->pLeft == _insertNode) ? -1:1; //-1 left , 1 right , 0 none
			
			if (_prentsNode->corol==ENode_Color::StartNode && uncleNode->corol==ENode_Color::StartNode)
			{
				_prentsNode->corol = ENode_Color::Black;
				uncleNode->corol = ENode_Color::Black;
				if (_prentsNode->pParent != nullptr) 
				{ 
					_prentsNode->pParent->corol = ENode_Color::StartNode; 
					if (_prentsNode->pParent == rootNode) 
					{ 
						_prentsNode->pParent->corol = ENode_Color::Black; 
					} 
					BalancingAfterInsertNode(_prentsNode->pParent->pParent, _prentsNode->pParent);
					return;
				}
				BalancingAfterInsertNode(_prentsNode->pParent, _prentsNode);
			}
			else if (_prentsNode->corol == ENode_Color::StartNode && uncleNode->corol == ENode_Color::Black && direction==1)
			{
				stNode* grandFatherNode = _prentsNode->pParent;
				int uncleDirection = (grandFatherNode->pLeft == uncleNode) ? -1 : 1;
				if (uncleDirection == 1)
				{
					TurnLeftNode(_prentsNode);
					BalancingAfterInsertNode(_prentsNode->pParent, _prentsNode);
				}
				else
				{
					//할아버지 기준으로 좌회전
					TurnLeftNode(grandFatherNode);
					grandFatherNode->corol= ENode_Color::StartNode;
					grandFatherNode->pParent->corol = ENode_Color::Black;
				}
				
			}
			else if (_prentsNode->corol == ENode_Color::StartNode && uncleNode->corol == ENode_Color::Black && direction == -1)
			{
				stNode* grandFatherNode = _prentsNode->pParent;
				int uncleDirection = (grandFatherNode->pLeft == uncleNode) ? -1 : 1;
				if (uncleDirection==1)
				{
					_prentsNode->corol = ENode_Color::Black;
					grandFatherNode->corol = ENode_Color::StartNode;
					TurnRightNode(grandFatherNode);
					BalancingAfterInsertNode(_prentsNode->pParent, _prentsNode);
				}
				else
				{
					TurnRightNode(_prentsNode);
					BalancingAfterInsertNode(_prentsNode->pParent, _prentsNode);
				}
			}
		}
		void BalancingAfterDeleteNode(stNode* _targetNode, ENode_Color _deletedColor)
		{
			if (_deletedColor == ENode_Color::StartNode)
			{
				//빨강을 지운거라면 밸런싱 할 필요가 없음.
				return;
			}
			stNode* pPrents = _targetNode->pParent;
			int direction = (pPrents->pLeft == _targetNode) ? -1 : 1;
			stNode* pSibling = (pPrents->pLeft == _targetNode) ? pPrents->pRight : pPrents->pLeft;
			stNode* pSiblingLChild = pSibling->pLeft;
			stNode* pSiblingRChilde = pSibling->pRight;
			
			/*
			
			2.1 삭제 노드의 부모와 자식이 모두 레드인 경우
			(또는 삭제 노드의 자식이 레드인가?)

			부모와 자식이 모두 레드라면 삭제노드(블랙)이 빠짐으로
					삭제노드 라인의 블랙이 하나 줄어들고 레드 - 레드의 문제가 됨.

					삭제 노드의 자식(레드) 을 블랙으로 바꾸어줌으로 문제 해결


			이는 다르게 보면 삭제노드의 자식 (밀치고 들어온 노드) 가 레드인가?
			의 조건으로 한정지을 수 있다.

			새로 자리한 노드가 레드라면 이를 블랙으로 바꾸는걸로 마무리 됨.
			*/
			if (_targetNode->corol==ENode_Color::StartNode)
			{
				_targetNode->corol = ENode_Color::Black;
				return;
			}
			/*
			2.2 삭제 노드의 형제가 레드


			형제를 블랙으로 바꿈
			부모를 기준으로 좌회전!!!
			기존 부모를 레드로 바꿈.

			그리고 현재노드 (삭제 후 들어온노드, 삭제노드의 자식) 기준으로 처음부터 재작업.

			이 경우는 좌회전으로 삭제노드 방향이 한칸씩 내려가며
			되려 더 깊이가 깊어진 상태로 재검사를 하게 됨.


			+ 형제가 레드라면 형제는 블랙이 존재하는 두 자식 라인을 가지고 있게됨.
			+ 이때 좌회전을 하면 형제가 부모가 되면서 형제의 기존 왼자식이 내쪽으로 넘어오게 됨
			+ 이로서 형제가 블랙이 되는 상황을 만든다
			? 부모가 원래 레드였으면 어떡하는가?,  형제가 레드므로 부모가 레드일 수 없음.

			*/
			else if (pSibling->corol==ENode_Color::StartNode)
			{
				if (direction==-1)
				{
					pSibling->corol = ENode_Color::Black;
					TurnLeftNode(pPrents);
					pPrents->corol = ENode_Color::StartNode;
				}
				else
				{
					pSibling->corol = ENode_Color::Black;
					TurnRightNode(pPrents);
					pPrents->corol = ENode_Color::StartNode;
				}
				
			}
			/*
			 2.3 삭제 노드의 형제가 블랙이고 형제의 양쪽 자식이 블랙

			이때는 형제를 레드로

			이걸로 내 가족 밸런스 처리는 끝. (내쪽이 블랙-1 이었음, 형제도 블랙 -1 해주었음)
			하지만 내 가족 전체에서 블랙이 하나 빠졌으므로, 할아버지 입장에서는
			내 부모라인으로 블랙 -1 상태가 됨.

			그래서 내 부모를 기준으로 처음부터 다시 처리.
			(내 부모를 삭제노드의 자식으로 봄)

			? 만약, 부모가 레드였다면? (형제를 레드로 바꾸는데서 문제발생?)
			어차피 부모를 기준으로 2.1 부터 다시 체크하게 되므로
			삭제 노드의 자식이 레드인 조건으로 들어가서, 블랙으로 변경후 상황종료.
			*/
			else if (pSibling->corol==ENode_Color::Black && pSibling->pLeft->corol==ENode_Color::Black&& pSibling->pRight->corol==ENode_Color::Black)
			{
				pSibling->corol = ENode_Color::StartNode;
				BalancingAfterDeleteNode(pPrents, ENode_Color::Black);
			}
			/*
			 2.4 삭제 노드의 형제가 블랙이고 형제의 왼자식이 레드, 오른자식이 블랙

			형제의 왼자식을 블랙으로

			형제를 레드로

			형제 기준으로 우회전


			이걸로 2.5 상황으로 맞추어짐.


			형제의 블랙을 -> 레드
			형제 자식 하나를 -> 블랙

			블랙으로 만든 자식을 부모로 올리고,
			기존의 부모는 레드로 바꿈.

			*/
			else if (direction==-1&&pSibling->corol==ENode_Color::Black && pSibling->pLeft->corol==ENode_Color::StartNode&& pSibling->pRight->corol==ENode_Color::Black)
			{
				pSibling->pLeft->corol = ENode_Color::Black;
				pSibling->corol = ENode_Color::StartNode;
				TurnRightNode(pSibling);
				stNode* grandFather = pSibling->pParent->pParent;
				TurnLeftNode(grandFather);

			}
			else if(direction ==1 && pSibling->corol == ENode_Color::Black && pSibling->pLeft->corol == ENode_Color::Black && pSibling->pRight->corol == ENode_Color::StartNode)
			{ 
				pSibling->pRight->corol = ENode_Color::Black;
				pSibling->corol = ENode_Color::StartNode;
				TurnLeftNode(pSibling);
				stNode* grandFather = pSibling->pParent->pParent;
				TurnRightNode(grandFather);
			}
			/*
			 2.5 삭제 노드의 형제가 블랙이고 형제의 오른자식이 레드


			형제 컬러를 부모 컬러로.

			부모 컬러는 블랙

			형제의 오른자식은 블랙

			부모기준 좌회원  

			+ 이 상황은 형제(블랙)의 자식에 레드가 존재 함으로서
			+ 간단하게 형제쪽에 블랙을 추가하여 내쪽에 블랙을 증가시킬 수 있는 상황!

			+ 부모의 컬러를 형제에게 넣으므로서 회전 후 부모의 색상은 동일할 것이며
			+ 기존 부모를 블랙으로 바꾸어서 이를 회전하여 삭제노드 쪽으로 넘김.
			+ 이렇게 삭제된 블랙을 +1  시키고, 블랙 이었던 형제가 부모역할을 하면서
			+ 형제 라인의 블랙이 -1 되었음, 이를 본래 형제의 레드자식을 블랙으로 바꿔서 해결
			*/
			else if (pSibling->corol==ENode_Color::Black&& pSibling->pRight->corol==ENode_Color::StartNode)
			{
				if (direction == -1)
				{
					pSibling->corol = pPrents->corol;
					pPrents->corol = ENode_Color::Black;
					pSibling->pRight->corol = ENode_Color::Black;
					TurnLeftNode(pPrents);
				}
				else
				{
					pSibling->corol = pPrents->corol;
					pPrents->corol = ENode_Color::Black;
					pSibling->pLeft->corol = ENode_Color::Black;
					TurnRightNode(pPrents);
				}
				
			}
			
		}
		void RecursiveInOrder(stNode* _node, function<void(stNode*)> _func)
		{
			if (_node==&nil)
			{
				return;
			}
			RecursiveInOrder(_node->pLeft, _func);
			_func(_node);
			RecursiveInOrder(_node->pRight, _func);
		}
		void RecursivePreOrder(stNode* _node, function<void(stNode*)> _firstfunc, function<void(stNode*)> _secondfunc)
		{
			if (_node == &nil)
			{
				return;
			}
			_firstfunc(_node);
			RecursivePreOrder(_node->pLeft, _firstfunc, _secondfunc);
			RecursivePreOrder(_node->pRight, _firstfunc, _secondfunc);
			_secondfunc(_node);

		}
		void RecursiveInsert(stNode* _targetNode, stNode* _insertNode, bool& result)
		{
			if (_targetNode==&nil)
			{
				return ;
			}
			//left
			if (_targetNode->iData > _insertNode->iData)
			{
				if(_targetNode->pLeft!=&nil)
				{
					RecursiveInsert(_targetNode->pLeft, _insertNode, result);
					return ;
				}
				_insertNode->pParent = _targetNode;
				_insertNode->pLeft = _targetNode->pLeft;
				if (_targetNode->pLeft!=&nil)
				{
					_targetNode->pLeft->pParent = _insertNode;
				}
				_targetNode->pLeft = _insertNode;
				result = true;
				//밸런스 체크
				BalancingAfterInsertNode(_targetNode, _insertNode);
				return;
			}
			//right
			else if (_targetNode->iData < _insertNode->iData)
			{
				if (_targetNode->pRight != &nil)
				{
					RecursiveInsert(_targetNode->pRight, _insertNode, result);
					return;
				}
				_insertNode->pParent = _targetNode;
				_insertNode->pRight = _targetNode->pRight;
				if (_targetNode->pRight!=&nil)
				{
					_targetNode->pRight->pParent = _insertNode;
				}
				_targetNode->pRight = _insertNode;
				result = true;
				//밸런스 체크
				BalancingAfterInsertNode(_targetNode, _insertNode);
				return ;
			}
			//==
			else
			{
				delete _insertNode;
				result = false;
				return ;
			}
		}
	public:
	private:
		stNode nil;
		stNode* rootNode;
	};
}


