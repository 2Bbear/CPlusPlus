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
	/*
	수정 순서가 바뀔 수 있는 코드
	최적화를 끄고 테스트 하면 cpu 순서 재정렬 현상을 관람할 수 있습니다.
	*/
	void Doit()
	{
		vector<thread> threads;

		atomic<int> a(0);
		atomic<int> b(0);

		threads.push_back(thread(t1, &a, &b));
		threads.push_back(thread(t2, &a, &b));

		for (int i = 0; i < 2; i++) {
			threads[i].join();
		}
	}
}