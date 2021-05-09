#include "stdafx.h"
#include "PetersonsAlgorithm.h"

namespace PetersonsAlgorithm
{
	bool flag[2];
	int turn;//몇번 스레드인지 스레드를 가르킴
#define BUFFERSIZE 1000000
	int gBuffer[BUFFERSIZE] = {0};
	int cri = 0;

	void lock_init()
	{
		flag[0] = flag[1] = 0;
		turn = 0;
	}

	void thread0Main()// 더하는 쪽
	{
		flag[0] = true;
		turn = 1;
		while (flag[1] && turn == 1); //1번이 임계영역에 있다면 반복
		//임계구역
		for (int i = 0; i < BUFFERSIZE; i++)
		{
			if (cri>= BUFFERSIZE)//최대 크기 보다 크다면 넘김
			{
				continue;
			}
			cri++;
		}
		//임계구역 끝
		flag[0] = false;
	}
	void thread1Main()//빼는 쪽
	{
		flag[1] = true;
		turn = 0;
		while (flag[0] && turn == 0); // 0번이 임계영역에 있다면 반복
		//임계구역
		for (int i = 0; i < BUFFERSIZE; i++)
		{
			if (cri<=0) //0보다 작다면 넘김
			{
				continue;
			}
			cri--;
		}
		//임계구역 끝
		flag[1] = false;
	}
	
	void Doit()
	{
		std::thread th1(thread0Main);
		std::thread th2(thread1Main);

		th1.join();
		th2.join();

		printf("Actual Count: %d | Expected Count: %d\n", cri, BUFFERSIZE );
	}
};

