#pragma once
template<typename T>
class CustomList
{
	//�޸� ����ȭ �غ���.
	struct Node
	{
		T Data;
		Node* next= nullptr;
		Node* prev=nullptr;
		int none = 0;
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
			if (_node->next==nullptr)
			{
				return _node->prev->Data;
			}
			return _node->Data;
		}
		bool operator !=(Iterator  itr)
		{
			if (_node==itr._node)
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
			Iterator *  pBase;
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
public:
	
private :
	Node begineNode; // ���������� ������ ���� �� ����� ����.
	Node endNode;
	unsigned int size = 0;

	//foreach
	//for�� �Ҷ����� end ��� �� �� ������ �������̴�.
public:
	//����ִ� list�� �����մϴ�
	CustomList()
	{
		endNode.next = nullptr;
		endNode.prev = &begineNode;
		endNode.none = -1;
		begineNode.next = &endNode;
		begineNode.prev = nullptr;
		begineNode.none = -2;
	}
	//0���̳� nullptr�� �ʱ�ȭ �� ���� _elementCount ���� ���� list�� �����մϴ�.
	CustomList(unsigned int _elementCount)
	{

	}
	// _obj�� �ʱ�ȭ �� _elementCount  ���� ���� list�� �����մϴ�
	CustomList(unsigned int _elementCount, T _obj)
	{
		Assign(_elementCount, _obj);
	}
	//_anotherList�� ������ list�� �����մϴ�.
	CustomList(CustomList & _anotherList)
	{
		for (CustomList<T>::Iterator itr = _anotherList.GetBegine(); itr!=_anotherList.GetEnd();++itr)
		{
			this->PushBack(*itr);
		}
	}
	~CustomList()
	{
		//�̾��� ��� ��� ����.
		Iterator itr = GetBegine();
		for (; itr!= GetEnd();)
		{
			itr = Erase(itr);
		}
	}
	bool IsEmpty() {
		if (begineNode.next == &endNode && endNode.prev == &begineNode)
		{
			return true;
		}
		return false;
	}
	void PushFront(T _data)
	{
		Node* newNode = new Node();
		newNode->Data = _data;
		if (IsEmpty()==true)
		{
			begineNode.next = newNode;
			newNode->prev = &begineNode;
			newNode->next = &endNode;
			endNode.prev = newNode;
		}
		else
		{
			Node* temp = begineNode.next;
			
			begineNode.next = newNode;
			newNode->prev = &begineNode;
			newNode->next = temp;
			temp->prev = newNode;
		}


		++size;
	}
	void PushBack(T _data)
	{
		Node* newNode = new Node();
		newNode->Data = _data;
		if (IsEmpty()==true)
		{
			endNode.prev = newNode;
			newNode->next = &endNode;
			newNode->prev = &begineNode;
			begineNode.next = newNode;
		}
		else
		{
			Node* temp = endNode.prev;

			endNode.prev = newNode;
			newNode->next = &endNode;
			newNode->prev = temp;
			temp->next = newNode;
		}
		++size;
	}
	/*
	_addObj�� �ʱ�ȭ �� _addcount ���� �Ҵ��մϴ�.
	�����͸� �����ϴ� ��ü��� ������� �ʱ⸦ ��õ�մϴ�.
	*/
	bool Assign(unsigned int _addCount , T _addObj )
	{
		T temp = _addObj; // ���簡 �Ͼ. �����Ͷ�� �������δ�ε� �����̱� ��..
		for (int count=0;count<_addCount;++count)
		{
			PushBack(temp);
		}
	}
	//����Ʈ�� ù��° ��Ҹ� ��ȯ�մϴ�.
	Iterator GetBegine()
	{
		//���� �� ��Ҹ� ��ȯ�ϴ� �� ����
		Iterator itr;
		if (IsEmpty()==true)
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
	Iterator GetEnd()
	{
		Iterator itr;
		itr._node = &endNode;

		return itr;
	}
	//ù��° ��Ҹ� ��ȯ�մϴ�.
	T GetFrontElement()
	{
		return begineNode.next->Data;
	}
	//������ ��Ҹ� ��ȯ�մϴ�.
	T GetBackElement()
	{
		return endNode.prev->Data;
	}
	//����� �����մϴ�.
	Iterator Erase(Iterator _itr)
	{
		Iterator result;

		if (_itr._node == &endNode)
		{
			result._node = &endNode;
			return result;
		}
		else if (_itr._node == &begineNode)
		{
			result._node = &endNode;
			return result;
		}
		Node* beforeNode = _itr._node->prev;
		Node* nextNode = _itr._node->next;

		beforeNode->next = nextNode;
		nextNode->prev = beforeNode;

		result._node = nextNode;

		delete _itr._node;
		--size;
		return result;
	}
	//�� �� ���Ҹ� ����
	void PopFront()
	{
		//��� �ִٸ�
		if (IsEmpty())
		{
			return;
		}
		Node* targetNode = begineNode.next;
		
		Node* nextNode = targetNode->next;

		begineNode.next = nextNode;
		nextNode->prev = &begineNode;

		delete targetNode;
		--size;
	}
	//�� �� ���Ҹ� ����
	void PopBack()
	{
		//��� �ִٸ�
		if (IsEmpty())
		{
			return;
		}

		Node* targetNode = endNode.prev;

		Node* prevNode = targetNode->prev;
		
		endNode.prev = prevNode;
		prevNode->next = &endNode;

		delete targetNode;
		--size;

	}
	int Size() 
	{ 
		return size;
	}
	Iterator InsertBack(Iterator _itr, T _ele)
	{
		Node* newNode = new Node();
		newNode->Data = _ele;
		Node* targetNode ;
		if (_itr._node == &endNode)
		{
			targetNode = endNode.prev;
		}
		else
		{
			targetNode = _itr._node;
		}
		
		Node* nextNode = targetNode->next;

		targetNode->next = newNode;
		newNode->prev = targetNode;
		newNode->next = nextNode;
		nextNode->prev = newNode;

		size++;

		Iterator result;
		result._node = newNode;

		return result;
	}
	Iterator InsertFront(Iterator _itr, T _ele)
	{
		Node* newNode = new Node();
		newNode->Data = _ele;
		
		Node* targetNode;
		if (_itr._node == &endNode)
		{
			targetNode = endNode.prev;
		}
		else
		{
			targetNode = _itr._node;
		}

		Node* beforeNode = targetNode->prev;
		targetNode->prev = newNode;
		newNode->next = targetNode;
		newNode->prev = beforeNode;
		beforeNode->next = newNode;

		size++;

		Iterator result;
		result._node = newNode;

		return result;
	}
};