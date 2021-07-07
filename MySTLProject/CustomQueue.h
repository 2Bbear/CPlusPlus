#pragma once

#include <iostream>
using namespace std;

template<typename T>
class CustomQueue
{

public:
	//�޸� ����ȭ �غ���.
	struct Node
	{
		T Data;
		Node* next = nullptr;
		Node* prev = nullptr;

		~Node()
		{
			next = nullptr;
			prev = nullptr;
		}
	};
public:
	class Iterator
	{
	public:
		Node* _node;
	public:
		Iterator()
		{
			_node = nullptr;
		}
		Iterator& operator++()
		{
			_node = _node->next;
			return *this;
		}
		Iterator operator ++(int)
		{
			Iterator temp = *this;
			++(*this);
			return temp;
		}
		Iterator& operator--()
		{
			_node = _node->prev;
			return *this;
		}
		Iterator operator --(int)
		{
			Iterator temp = *this;
			--(*this);
			return temp;
		}
		T& operator *()
		{
			//end�� �ǹ�
			if (_node->next == nullptr)
			{
				return _node->prev->Data;
			}
			return _node->Data;
		}
		bool operator !=(Iterator  itr)
		{
			if (_node == itr._node)
			{
				return false;
			}
			return true;
		}
		bool operator ==(Iterator itr)
		{
			if (_node == itr._node)
			{
				return true;
			}
			return false;
		}
		Iterator& operator =(const Iterator& p)
		{
			this->_node = p._node;
			return *this;
		}
		void Swap(Iterator _itr)
		{
			Iterator* pBase;
			Iterator base;

			//���� �ڱ� �ڽ��� �ٲٷ��� �Ѵٸ�
			if (this->_node == _itr._node)
			{
				return;
			}
			//target1�� end�� �ǹ�
			if (this->_node->next == nullptr)
			{
				base._node = base._node->prev;
				pBase = &base;
			}
			else
			{
				pBase = this;
			}

			//target2�� end�� �ǹ�
			if (_itr._node->next == nullptr)
			{
				_itr._node = _itr._node->prev;
			}

			T temp = pBase->_node->Data;
			pBase->_node->Data = _itr._node->Data;
			_itr._node->Data = temp;

		}
	};
private:
	Node begineNode; // ���������� ������ ���� �� ����� ����.
	Node endNode;
public:
	int size = 0;
	

	CustomQueue()
	{
		endNode.next = nullptr;
		endNode.prev = &begineNode;
		begineNode.next = &endNode;
		begineNode.prev = nullptr;
	}
	virtual ~CustomQueue()
	{
		//delete[] values;
	}

	//����Ʈ�� ù��° ��Ҹ� ��ȯ�մϴ�.
	Iterator Begine()
	{
		//���� �� ��Ҹ� ��ȯ�ϴ� �� ����
		Iterator itr;
		if (Empty() == true)
		{
			itr._node = &endNode;
		}
		else
		{
			itr._node = begineNode.next;
		}
		return itr;
	}
	//����Ʈ�� �������ǹ̸� ��Ÿ���ϴ�.
	Iterator End()
	{
		Iterator itr;
		itr._node = &endNode;
		return itr;
	}

	bool Push(T _value) 
	{
		Node* newNode = new Node();
		newNode->Data = _value;

		//ó�� ���� ����� ���
		if (Empty()==true)
		{
			//�տ� �־���.
			begineNode.next = newNode;
			newNode->prev = &begineNode;
			newNode->next = &endNode;
			endNode.prev = newNode;

		}
		//�߰��� ���� ����� ���
		else
		{
			Node* temp = begineNode.next;

			begineNode.next = newNode;
			newNode->prev = &begineNode;
			newNode->next = temp;
			temp->prev = newNode;
		}
		++size;

		return true;
	}
	T Pop()
	{
		T result=NULL;
		//��� �ִٸ�
		if (Empty()==true)
		{
			return result;
		}

		Node* targetNode = endNode.prev;

		Node* prevNode = targetNode->prev;

		endNode.prev = prevNode;
		prevNode->next = &endNode;
		result = targetNode->Data;
		delete targetNode;
		--size;
		return result;
	}

	T Front()
	{
		if (Empty()==true)
		{
			return nullptr;
		}
		return begineNode.next->Data;
	}

	T Back()
	{
		if (Empty() == true)
		{
			return nullptr;
		}
		return endNode.prev->Data;
	}


	bool Empty()
	{
		if (begineNode.next == &endNode && endNode.prev == &begineNode)
		{
			return true;
		}
		return false;
	}
};
