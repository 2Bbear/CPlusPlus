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
	만약 x= 0, y=0이 뜬다면 cpu가 순서를 재정렬했다는 것을 추측할 수 있습니다.
	왜냐면 단일 스레드 관점에서 볼때 x나 y값이 들어가려면 먼저 1이 들어가야 하기 때문입니다.
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

	void worker(std::atomic<int>* counter) {
		for (int i = 0; i < 10000; i++) {
			// 다른 연산들 수행

			counter->fetch_add(1, memory_order_relaxed);
		}
	}
	/*
	순서 재배치가 일어나도 무방한 코드

	어차피 counter를 증가시킬 것이기 때문에, 전체 총 스레드 수만 맞다면
	수정 순서는 모두 일관적이기 때문에 예상한 결과 값을 얻을 수 있다.
	이경우 40000이 나올 것이다. 스레드가 4개니까.
	*/
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