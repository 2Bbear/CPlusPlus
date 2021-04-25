#include "stdafx.h"

#include "FenceTest.h"
using namespace std;
namespace FenceTest
{
	std::atomic<std::string*>ptr;
	int data;
	std::atomic<int>atoData;

	void Producer()
	{
		string* p = new string("C++11");
		data = 2011;
		atoData.store(2014, memory_order_relaxed);
		ptr.store(p, memory_order_release);
	}
	void Consumer()
	{
		string* p2;
		while (!(p2 = ptr.load(memory_order_acquire)));
		cout << "*p2:" << *p2 << endl;
		cout << "data: " << data << endl;
		cout << "atoData: " << atoData.load(memory_order_relaxed) << endl;
	}
	void ProducerFence()
	{
		string* p = new string("C++11");
		data = 2011; // 순서 재지정 가능 대상
		atoData.store(2014, memory_order_relaxed);//순서 재지정 가능 대상
		atomic_thread_fence(memory_order_release);
		ptr.store(p, memory_order_relaxed);
	}

	void ConsumerFence()
	{
		string* p2;
		while (!(p2 = ptr.load(memory_order_relaxed)));
		atomic_thread_fence(memory_order_acquire);
		cout << "*p2:" << *p2 << endl;
		cout << "data: " << data << endl;
		cout << "atoData: " << atoData.load(memory_order_relaxed) << endl;
	}

	void Doit()
	{
		cout << endl;

		thread t1(Producer);
		thread t2(Consumer);
		t1.join();
		t2.join();
	}
}