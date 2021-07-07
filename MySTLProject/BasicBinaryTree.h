#pragma once
#include <functional>
namespace twobbearlib
{
	using namespace  std;
	/// @brief 이진 트리인데, 특징인게 삽입시 정렬을 위해 중간 삽입이 일어남
	class BasicBinaryTree
	{
		
	public:
		struct st_Node
		{
			int data=0;
			st_Node* left = nullptr;
			st_Node* right = nullptr;
			st_Node* parents=nullptr;
		};
		enum class ResultType
		{
			fail = 0,
			success,
			overlapError
		};
	public:
		BasicBinaryTree()
		{
			rootNode = nullptr;
		}
		~BasicBinaryTree()
		{

		}
	private:
		st_Node* rootNode;
	public:
		int createdNodeCount = 0;
	public:
		void PrintAllNode(function<void(st_Node*)> _func)
		{
			if (rootNode == nullptr)
			{
				return;
			}
			//gui 버전
			if (_func!=nullptr)
			{
				InOrder(rootNode, nullptr, [_func](st_Node* _node, st_Node* _targetNode)->ResultType {
					_func(_node);
					return ResultType::fail;
					});
			}
			//console 버전
			else
			{
				InOrder(rootNode, nullptr, [](st_Node* _node, st_Node* _targetNode)->ResultType {
					static int check = -1;
					printf("%d %s %s\n", _node->data, (check == _node->data ? "중복에러" : " "), (check > _node->data ? "순서에러" : " "));
					check = _node->data;
					return ResultType::fail;
					});
			}
			

		}
		/// @brief 노드 삽입
		/// @return true일 경우 삽입 성공
		template<typename Func>
		ResultType InsertNode(int _num, Func _guiFunc)
		{
			st_Node* _node = new st_Node();
			_node->data = _num;
			if (rootNode == nullptr)
			{
				rootNode = _node;
				createdNodeCount++;
				return ResultType::success;
			}
			else
			{
				//재귀 insert 함수 호출
				if (RecursionInsert(rootNode, _node) == ResultType::success)
				{
					createdNodeCount++;
				}

			}
		}
		ResultType RecursionInsert(st_Node* _node, st_Node* _targetNode)
		{
			if (_node == nullptr)
			{
				return ResultType::fail;
			}

			if ( _node->data>_targetNode->data)
			{
				ResultType r = RecursionInsert(_node->left, _targetNode);
				if (r == ResultType::overlapError) { return ResultType::overlapError; }
				else if (r == ResultType::success) { return ResultType::success; }

				_targetNode->parents = _node;
				_targetNode->left = _node->left;
				if (_node->left != nullptr)
				{
					_node->left->parents = _targetNode;
				}
				_node->left = _targetNode;
				return ResultType::success;
			}
			else if(_node->data < _targetNode->data)
			{
				ResultType r = RecursionInsert(_node->right, _targetNode);
				if ( r == ResultType::overlapError) { return ResultType::overlapError; }
				else if (r == ResultType::success) { return ResultType::success; }

				if (_node->parents !=nullptr&&_node->parents->data < _targetNode->data)
				{
					return ResultType::fail;
				}
				if (_node->right != nullptr&& _node->right->data< _targetNode->data)
				{
					return ResultType::fail;
				}

				_targetNode->parents = _node;
				_targetNode->right = _node->right;
				if (_node->right != nullptr)
				{
					_node->right->parents = _targetNode;
				}
				_node->right = _targetNode;
				return ResultType::success;
			}
			else
			{
				return ResultType::overlapError;
			}
			return ResultType::fail;
		}
		/// @brief 노드 삭제
		/// @return null일 경우 실패, 성공시 삭제한 노드 반환
		st_Node* DeleteNode()
		{
			return nullptr;
		}
		/*
		전위 순회
		1. Access the data part of the current node
		2. Traverse the left subtree by recursively calling the pre-order function
		3. Travers the right subtree by recursicely calling the pre-order function.
		*/
		template<typename Func>
		void PreOrder(st_Node* _node, st_Node* _targetNode, Func _func)
		{
			if (_node == nullptr)
			{
				return;
			}
			//작업
			ResultType result1 = _func(_node, _targetNode);
			if (result1 == ResultType::success)
			{
				return ResultType::success;
			}
			else if (result1 == ResultType::overlapError)
			{
				//중복 에러 발생
				return ResultType::overlapError;
			}
			//
			ResultType result2 = PreOrder(_node->left);
			if (result2 == ResultType::success) { return ResultType::success; }
			else if (result2 == ResultType::overlapError)
			{
				return ResultType::overlapError;
			}
			ResultType result3 = PreOrder(_node->right);
			if (result3 == ResultType::success) { return ResultType::success; }
			else if (result3 == ResultType::overlapError)
			{
				return ResultType::overlapError;
			}
		}
		/*
		중위 순회
		1. Traverse the left subtree by recursively calling the in- order function
		2. Access the data part of the current node
		3. Travers the right subtree by recursicely calling the in-order function
		*/
		template<typename Func>
		ResultType InOrder(st_Node* _node, st_Node* _targetNode, Func _func)
		{
			if (_node == nullptr)
			{
				return ResultType::fail;
			}
			ResultType result1= InOrder(_node->left, _targetNode, _func);
			if (result1 == ResultType::success) { return ResultType::success; }
			else if (result1 == ResultType::overlapError)
			{
				return ResultType::overlapError;
			}
			//작업
			ResultType result2 = _func(_node, _targetNode);
			if (result2 == ResultType::success)
			{
				return ResultType::success;
			}
			else if (result2== ResultType::overlapError)
			{
				//중복 에러 발생
				return ResultType::overlapError;
			}
			//
			ResultType result3 = InOrder(_node->right, _targetNode, _func);
			if (result3 == ResultType::success) { return ResultType::success; }
			else if (result3 == ResultType::overlapError)
			{
				return ResultType::overlapError;
			}
			return ResultType::fail;
		}
		/*
		후위 순회
		1. Traverse the left subtree by recursively calling the post-order function.
		2. Traverse the right subtree by recursively calling the post-order function.
		3. Access the data part of the current node.
		*/
		void PostOrder(st_Node* _node)
		{
			if (_node == nullptr)
			{
				return;
			}
			PostOrder(_node->left);
			PostOrder(_node->right);
			//작업

			//
		}
		/*
		역 중위 순회
		Traverse the right subtree by recursively calling the reverse in-order function.
		Access the data part of the current node.
		Traverse the left subtree by recursively calling the reverse in-order function.
		*/
		void ReverseInOrder()
		{

		}
	};


}
