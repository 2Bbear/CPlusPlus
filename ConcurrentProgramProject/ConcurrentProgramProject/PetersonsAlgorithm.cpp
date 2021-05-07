#include "stdafx.h"
#include "PetersonsAlgorithm.h"

namespace PetersonsAlgorithm
{
	bool flag[2];
	int turn;//몇번 스레드인지 스레드를 가르킴
#define BUFFERSIZE 100000
	int gBuffer[BUFFERSIZE] = {0};
	int cri = 0;

	void lock_init()
	{
		flag[0] = flag[1] = 0;
		turn = 0;
	}

	void thread1Main()
	{
		flag[0] = true;
		turn = 1;
		while (flag[1] && turn == 1);
		//임계구역
		for (int i = 0; i < BUFFERSIZE; i++)
		{
			++cri;
		}
		//임계구역 끝
		flag[0] = false;
	}
	void thread2Main()
	{
		flag[1] = true;
		turn = 0;
		while (flag[0] && turn == 0);
		//임계구역
		for (int i = 0; i < BUFFERSIZE; i++)
		{
			++cri;
		}
		//임계구역 끝
		flag[1] = false;
	}
	
	void Doit()
	{
		std::thread th1(thread1Main);
		std::thread th2(thread2Main);

		th1.join();
		th2.join();

		printf("Actual Count: %d | Expected Count: %d\n", cri, BUFFERSIZE * 2);
	}
};

