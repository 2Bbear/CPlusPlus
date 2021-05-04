#include "stdafx.h"
#include "Memory_order_acquireTest.h"

namespace Memory_order_acquireTest
{
	using namespace std;
	using std::memory_order_relaxed;
	
	void producer(std::atomic<bool>* is_ready, int* data) {
		data[0] = 1;
		data[1] = 2;
		data[2] = 3;
		data[3] = 4;
		data[4] = 5;
		data[5] = 6;
		data[6] = 7;
		data[7] = 8;
		data[8] = 9;
		data[9] = 10;
		is_ready->store(true, memory_order_relaxed);
	}

	void consumer(std::atomic<bool>* is_ready, int* data) {
		// data 가 준비될 때 까지 기다린다.
		while (!is_ready->load(memory_order_relaxed)) {
		}
		if (data[0] != 1)
		{
			std::cout << "Data error : " << data[0] << std::endl;
		}
		if (data[1] != 2)
		{
			std::cout << "Data error : " << data[1] << std::endl;
		}
		if (data[2] != 3)
		{
			std::cout << "Data error : " << data[2] << std::endl;
		}
		if (data[3] != 4)
		{
			std::cout << "Data error : " << data[3] << std::endl;
		}
		if (data[4] != 5)
		{
			std::cout << "Data error : " << data[4] << std::endl;
		}
		if (data[5] != 6)
		{
			std::cout << "Data error : " << data[5] << std::endl;
		}
		if (data[6] != 7)
		{
			std::cout << "Data error : " << data[6] << std::endl;
		}
		if (data[7] != 8)
		{
			std::cout << "Data error : " << data[7] << std::endl;
		}
		if (data[8] != 9)
		{
			std::cout << "Data error : " << data[8] << std::endl;
		}
		if (data[9] != 10)
		{
			std::cout << "Data error : " << data[9] << std::endl;
		}
	}
	/*
	CPU에서 진짜 명령어 재배치가 일어나는지 확인하는 코드
	*/
	void Doit()
	{
		std::vector<std::thread> threads;

		std::atomic<bool> is_ready(false);

		int *data = new int[10];
		threads.push_back(std::thread(producer, &is_ready, data));
		threads.push_back(std::thread(consumer, &is_ready, data));

		for (int i = 0; i < 2; i++) {
			threads[i].join();
		}
		delete[] data;
	}

	void producer2(std::atomic<bool>* is_ready, int* data) {
		*data = 10;
		is_ready->store(true, std::memory_order_release);
	}

	void consumer2(std::atomic<bool>* is_ready, int* data) {
		// data 가 준비될 때 까지 기다린다.
		while (!is_ready->load(std::memory_order_acquire)) {
		}

		std::cout << "Data : " << *data << std::endl;
	}

	void Doit2() {
		std::vector<std::thread> threads;

		std::atomic<bool> is_ready(false);
		int data = 0;

		threads.push_back(std::thread(producer2, &is_ready, &data));
		threads.push_back(std::thread(consumer2, &is_ready, &data));

		for (int i = 0; i < 2; i++) {
			threads[i].join();
		}
	}

	std::atomic<bool> is_ready;
	std::atomic<int> data[3];

	void producer3() {
		data[0].store(1, memory_order_relaxed);
		data[1].store(2, memory_order_relaxed);
		data[2].store(3, memory_order_relaxed);
		is_ready.store(true, std::memory_order_release);
	}

	void consumer3() {
		// data 가 준비될 때 까지 기다린다.
		while (!is_ready.load(std::memory_order_acquire)) {
		}

		std::cout << "data[0] : " << data[0].load(memory_order_relaxed) << std::endl;
		std::cout << "data[1] : " << data[1].load(memory_order_relaxed) << std::endl;
		std::cout << "data[2] : " << data[2].load(memory_order_relaxed) << std::endl;
	}

	void Doit3() {
		std::vector<std::thread> threads;

		threads.push_back(std::thread(producer3));
		threads.push_back(std::thread(consumer3));

		for (int i = 0; i < 2; i++) {
			threads[i].join();
		}
	}


	std::atomic<bool> x(false);
	std::atomic<bool> y(false);
	std::atomic<int> z(0);

	void write_x() { x.store(true, std::memory_order_release); }

	void write_y() { y.store(true, std::memory_order_release); }

	void read_x_then_y() {
		while (!x.load(std::memory_order_acquire)) {
		}
		if (y.load(std::memory_order_acquire)) {
			++z;
		}
	}

	void read_y_then_x() {
		while (!y.load(std::memory_order_acquire)) {
		}
		if (x.load(std::memory_order_acquire)) {
			++z;
		}
	}
	/*
	relesae - acquire 이지만 원하는 동기화가 안 일어나는 것을 볼 수 있는 형태
	read_x_then_y 와 read_y_then_x 두 스레드가 같은 순서로 x.store와 y.store를 관찰한다는 보장이 없다.
	*/
	void Doit4() {
		thread a(write_x);
		thread b(write_y);
		thread c(read_x_then_y);
		thread d(read_y_then_x);
		a.join();
		b.join();
		c.join();
		d.join();
		std::cout << "z : " << z << std::endl;
	}

}