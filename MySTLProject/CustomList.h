#pragma once
template<typename T>
class CustomList
{
	//메모리 최적화 해볼것.
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
			//end를 의미
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

			//만약 자기 자신을 바꾸려고 한다면
			if (this->_node == _itr._node)
			{
				return;
			}
			//target1가 end를 의미
			if (this->_node->next == nullptr)
			{
				base._node = base._node->prev;
				pBase = &base;
			}
			else
			{
				pBase = this;
			}

			//target2이 end를 의미
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
	Node begineNode; // 실질적으로 유저가 접근 할 방법이 없다.
	Node endNode;
	unsigned int size = 0;

	//foreach
	//for문 할때마다 end 잡는 건 좀 성능이 후질것이다.
public:
	//비어있는 list를 생성합니다
	CustomList()
	{
		endNode.next = nullptr;
		endNode.prev = &begineNode;
		endNode.none = -1;
		begineNode.next = &endNode;
		begineNode.prev = nullptr;
		begineNode.none = -2;
	}
	//0값이나 nullptr로 초기화 된 원소 _elementCount 개를 가진 list를 생성합니다.
	CustomList(unsigned int _elementCount)
	{

	}
	// _obj로 초기화 된 _elementCount  개를 가진 list를 생성합니다
	CustomList(unsigned int _elementCount, T _obj)
	{
		Assign(_elementCount, _obj);
	}
	//_anotherList를 복사한 list를 생성합니다.
	CustomList(CustomList & _anotherList)
	{
		for (CustomList<T>::Iterator itr = _anotherList.GetBegine(); itr!=_anotherList.GetEnd();++itr)
		{
			this->PushBack(*itr);
		}
	}
	~CustomList()
	{
		//이어진 모든 노드 삭제.
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
	_addObj로 초기화 된 _addcount 개를 할당합니다.
	포인터를 저장하는 개체라면 사용하지 않기를 추천합니다.
	*/
	bool Assign(unsigned int _addCount , T _addObj )
	{
		T temp = _addObj; // 복사가 일어남. 포인터라면 포인터인대로도 문제이긴 함..
		for (int count=0;count<_addCount;++count)
		{
			PushBack(temp);
		}
	}
	//리스트의 첫번째 요소를 반환합니다.
	Iterator GetBegine()
	{
		//실제 앞 요소를 반환하는 게 좋음
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
	//리스트의 마지막의미를 나타냅니다.
	Iterator GetEnd()
	{
		Iterator itr;
		itr._node = &endNode;

		return itr;
	}
	//첫번째 요소를 반환합니다.
	T GetFrontElement()
	{
		return begineNode.next->Data;
	}
	//마지막 요소를 반환합니다.
	T GetBackElement()
	{
		return endNode.prev->Data;
	}
	//대상을 삭제합니다.
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
	//맨 앞 원소를 제거
	void PopFront()
	{
		//비어 있다면
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
	//맨 뒤 원소를 제거
	void PopBack()
	{
		//비어 있다면
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