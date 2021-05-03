#include "stdafx.h"
#include "Memory_order_acquireTest.h"

namespace Memory_order_acquireTest
{
	using namespace std;
	using std::memory_order_relaxed;
	
	void producer(std::atomic<bool>* is_ready, int* data) {
		*data = 10;
		is_ready->store(true, std::memory_order_release);
	}

	void consumer(std::atomic<bool>* is_ready, int* data) {
		// data �� �غ�� �� ���� ��ٸ���.
		while (!is_ready->load(std::memory_order_acquire)) {}

		std::cout << "Data : " << *data << std::endl;
	}

	void Doit()
	{
		std::vector<std::thread> threads;

		std::atomic<bool> is_ready(false);
		int data = 0;

		threads.push_back(std::thread(producer, &is_ready, &data));
		threads.push_back(std::thread(consumer, &is_ready, &data));

		for (int i = 0; i < 2; i++) {
			threads[i].join();
		}
	}
}