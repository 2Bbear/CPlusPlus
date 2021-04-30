#include "stdafx.h"
#include "Memory_order_relexedTest.h"


namespace Memory_order_relexedTest
{
	using namespace std;
	using std::memory_order_relaxed;

	void t1(atomic<int>* a, atomic<int>* b)
	{
		b->store(1, memory_order_relaxed);      // b = 1 (����)
		int x = a->load(memory_order_relaxed);  // x = a (�б�)

		printf("x : %d \n", x);
	}

	void t2(atomic<int>* a, atomic<int>* b) {
		a->store(1, memory_order_relaxed);      // a = 1 (����)
		int y = b->load(memory_order_relaxed);  // y = b (�б�)

		printf("y : %d \n", y);
	}
	/*
	���� ������ �ٲ� �� �ִ� �ڵ�
	����ȭ�� ���� �׽�Ʈ �ϸ� cpu ���� ������ ������ ������ �� �ֽ��ϴ�.
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