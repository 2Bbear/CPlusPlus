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
					(_node->corol==ENode_Color::StartNode?"��":"��"),
					(check == _node->iData ? "�ߺ�����" : " "),
					(check > _node->iData ? "��������" : " "));

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
				//����!
				stNode* pChLeft = _targetNode->pLeft;
				stNode* pChRight = _targetNode->pRight;
				//�����Ϸ��� ����� �ڽ� ��尡 ���� ���
				if (pChLeft ==&nil && pChRight == &nil)
				{
					//�θ��� �������� ��������
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
					else//���� ������ �ȵǾ� �ִ� ����
					{
						assert(1);
						printf_s("error!! \n");
					}
					ENode_Color deletedNodeColor = _targetNode->corol;
					delete _targetNode;
					
					//������ ������ ���� nil�� �ٿ��� �״� nil�� �������� �뷱���� �Ѵ�.
					BalancingAfterDeleteNode(&nil, deletedNodeColor);

					result = true;
					return;
				}
				//�����Ϸ��� ����� �ڽ� ��尡 �ϳ� �϶�
				else if (pChLeft == &nil && pChRight != &nil)
				{
					//�θ��� �������� ��������
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
					else//���� ������ �ȵǾ� �ִ� ����
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
					//�θ��� �������� ��������
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
					else//���� ������ �ȵǾ� �ִ� ����
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

				//�����Ϸ��� ����� �ڽ��� �Ѵ� ������ ���ʿ��� ���� ū ��带 �÷���������
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
					if (pChild->pLeft!=&nil)//�ٿ����� ���� ��尡 �ִ� ���
					{
						if (pChildsParents== _targetNode) // ���� �������ϴ°� ���� ����� �ٷ� �Ʒ��� �ִ� �����?
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
					else//�ٿ����� ���� ��尡 ���� ���
					{
						if (pChildsParents == _targetNode) // ���� �������ϴ°� ���� ����� �ٷ� �Ʒ��� �ִ� �����?
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
					//�Ҿƹ��� �������� ��ȸ��
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
				//������ ����Ŷ�� �뷱�� �� �ʿ䰡 ����.
				return;
			}
			stNode* pPrents = _targetNode->pParent;
			int direction = (pPrents->pLeft == _targetNode) ? -1 : 1;
			stNode* pSibling = (pPrents->pLeft == _targetNode) ? pPrents->pRight : pPrents->pLeft;
			stNode* pSiblingLChild = pSibling->pLeft;
			stNode* pSiblingRChilde = pSibling->pRight;
			
			/*
			
			2.1 ���� ����� �θ�� �ڽ��� ��� ������ ���
			(�Ǵ� ���� ����� �ڽ��� �����ΰ�?)

			�θ�� �ڽ��� ��� ������ �������(��)�� ��������
					������� ������ ���� �ϳ� �پ��� ���� - ������ ������ ��.

					���� ����� �ڽ�(����) �� ������ �ٲپ������� ���� �ذ�


			�̴� �ٸ��� ���� ��������� �ڽ� (��ġ�� ���� ���) �� �����ΰ�?
			�� �������� �������� �� �ִ�.

			���� �ڸ��� ��尡 ������ �̸� ������ �ٲٴ°ɷ� ������ ��.
			*/
			if (_targetNode->corol==ENode_Color::StartNode)
			{
				_targetNode->corol = ENode_Color::Black;
				return;
			}
			/*
			2.2 ���� ����� ������ ����


			������ ������ �ٲ�
			�θ� �������� ��ȸ��!!!
			���� �θ� ����� �ٲ�.

			�׸��� ������ (���� �� ���³��, ��������� �ڽ�) �������� ó������ ���۾�.

			�� ���� ��ȸ������ ������� ������ ��ĭ�� ��������
			�Ƿ� �� ���̰� ����� ���·� ��˻縦 �ϰ� ��.


			+ ������ ������ ������ ���� �����ϴ� �� �ڽ� ������ ������ �ְԵ�.
			+ �̶� ��ȸ���� �ϸ� ������ �θ� �Ǹ鼭 ������ ���� ���ڽ��� �������� �Ѿ���� ��
			+ �̷μ� ������ ���� �Ǵ� ��Ȳ�� �����
			? �θ� ���� ���忴���� ��ϴ°�?,  ������ ����Ƿ� �θ� ������ �� ����.

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
			 2.3 ���� ����� ������ ���̰� ������ ���� �ڽ��� ��

			�̶��� ������ �����

			�̰ɷ� �� ���� �뷱�� ó���� ��. (������ ��-1 �̾���, ������ �� -1 ���־���)
			������ �� ���� ��ü���� ���� �ϳ� �������Ƿ�, �Ҿƹ��� ���忡����
			�� �θ�������� �� -1 ���°� ��.

			�׷��� �� �θ� �������� ó������ �ٽ� ó��.
			(�� �θ� ��������� �ڽ����� ��)

			? ����, �θ� ���忴�ٸ�? (������ ����� �ٲٴµ��� �����߻�?)
			������ �θ� �������� 2.1 ���� �ٽ� üũ�ϰ� �ǹǷ�
			���� ����� �ڽ��� ������ �������� ����, ������ ������ ��Ȳ����.
			*/
			else if (pSibling->corol==ENode_Color::Black && pSibling->pLeft->corol==ENode_Color::Black&& pSibling->pRight->corol==ENode_Color::Black)
			{
				pSibling->corol = ENode_Color::StartNode;
				BalancingAfterDeleteNode(pPrents, ENode_Color::Black);
			}
			/*
			 2.4 ���� ����� ������ ���̰� ������ ���ڽ��� ����, �����ڽ��� ��

			������ ���ڽ��� ������

			������ �����

			���� �������� ��ȸ��


			�̰ɷ� 2.5 ��Ȳ���� ���߾���.


			������ ���� -> ����
			���� �ڽ� �ϳ��� -> ��

			������ ���� �ڽ��� �θ�� �ø���,
			������ �θ�� ����� �ٲ�.

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
			 2.5 ���� ����� ������ ���̰� ������ �����ڽ��� ����


			���� �÷��� �θ� �÷���.

			�θ� �÷��� ��

			������ �����ڽ��� ��

			�θ���� ��ȸ��  

			+ �� ��Ȳ�� ����(��)�� �ڽĿ� ���尡 ���� �����μ�
			+ �����ϰ� �����ʿ� ���� �߰��Ͽ� ���ʿ� ���� ������ų �� �ִ� ��Ȳ!

			+ �θ��� �÷��� �������� �����Ƿμ� ȸ�� �� �θ��� ������ ������ ���̸�
			+ ���� �θ� ������ �ٲپ �̸� ȸ���Ͽ� ������� ������ �ѱ�.
			+ �̷��� ������ ���� +1  ��Ű��, �� �̾��� ������ �θ����� �ϸ鼭
			+ ���� ������ ���� -1 �Ǿ���, �̸� ���� ������ �����ڽ��� ������ �ٲ㼭 �ذ�
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
				//�뷱�� üũ
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
				//�뷱�� üũ
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


