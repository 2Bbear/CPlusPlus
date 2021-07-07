#pragma once
#include <iostream>
#include <assert.h>
using namespace std;

template<typename T>
class CircleQueue
{
	
public:
	CircleQueue(int _size)
	{
		size = _size;
		values = new T[size];
		frontIdx = 0;
		rearIdx = 0;
	}
	~CircleQueue()
	{
		delete[] values;
	}
public:

private:
	int frontIdx;
	int rearIdx;
	int size;
	T* values;
public:
		
	void EnQueue(T _value)
	{
		if (!IsFull())
		{
			rearIdx = (rearIdx + 1) % size;
			values[rearIdx] = _value;
		}
		else
		{
			std::cout << "push false" << std::endl;
			assert(0);
		}
				
	}
		
	T DeQueue()
	{
		if (Empty())
		{
			cout << "Pop error : Queue is Empty" << endl;
			assert(0);
			return nullptr;
				
		}
		else
		{
			frontIdx = (frontIdx + 1) % size;
			return values[frontIdx];
		}
	}
	bool Empty()
	{
		if (rearIdx == frontIdx)
			return true;
		else
			return false;
	}

	T Back() noexcept
	{
		if (Empty())
		{
			cout << "Back error : Queue is Empty" << endl;
			return nullptr;
		}
		return values[rearIdx];
	}
	T Front() noexcept
	{
		if (Empty())
		{
			cout << "Front error : Queue is Empty" << endl;
			return nullptr;
		}
		return values[frontIdx];
	}
	void Emplace()
	{

	}
	unsigned int Size()
	{
		return size;
	}
	void Swap()
	{

	}
		
private:
	bool IsFull()
	{
		if ((rearIdx + 1) % size == frontIdx)
			return true;
		else
			return false;
	}
};


