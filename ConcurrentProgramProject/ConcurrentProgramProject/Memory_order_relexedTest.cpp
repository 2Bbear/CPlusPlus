#include "stdafx.h"
#include "Memory_order_relexedTest.h"


namespace Memory_order_relexedTest
{
	using namespace std;
	using std::memory_order_relaxed;

	void t1(atomic<int>* a, atomic<int>* b)
	{
		b->store(1, memory_order_relaxed);      // b = 1 (쓰기)
		int x = a->load(memory_order_relaxed);  // x = a (읽기)

		printf("x : %d \n", x);
	}

	void t2(atomic<int>* a, atomic<int>* b) {
		a->store(1, memory_order_relaxed);      // a = 1 (쓰기)
		int y = b->load(memory_order_relaxed);  // y = b (읽기)

		printf("y : %d \n", y);
	}
	
	void Doit()
	{
		cout << "======" << endl;
		vector<thread> threads;

		atomic<int> a(0);
		atomic<int> b(0);

		threads.push_back(thread(t1, &a, &b));
		threads.push_back(thread(t2, &a, &b));

		for (int i = 0; i < 2; i++) {
			threads[i].join();
		}
		
	}

	void worker(std::atomic<int>* counter) {
		for (int i = 0; i < 10000; i++) {
			// 다른 연산들 수행

			counter->fetch_add(1, memory_order_relaxed);
		}
	}
	
	void Doit2()
	{
		std::vector<std::thread> threads;

		std::atomic<int> counter(0);

		for (int i = 0; i < 4; i++) {
			threads.push_back(std::thread(worker, &counter));
		}

		for (int i = 0; i < 4; i++) {
			threads[i].join();
		}

		std::cout << "Counter : " << counter << std::endl;
	}
}