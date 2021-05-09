#include "stdafx.h"
#include "PetersonsAlgorithm.h"

namespace PetersonsAlgorithm
{
	bool flag[2];
	int turn;//��� ���������� �����带 ����Ŵ
#define BUFFERSIZE 1000000
	int gBuffer[BUFFERSIZE] = {0};
	int cri = 0;

	void lock_init()
	{
		flag[0] = flag[1] = 0;
		turn = 0;
	}

	void thread0Main()// ���ϴ� ��
	{
		flag[0] = true;
		turn = 1;
		while (flag[1] && turn == 1); //1���� �Ӱ迵���� �ִٸ� �ݺ�
		//�Ӱ豸��
		for (int i = 0; i < BUFFERSIZE; i++)
		{
			if (cri>= BUFFERSIZE)//�ִ� ũ�� ���� ũ�ٸ� �ѱ�
			{
				continue;
			}
			cri++;
		}
		//�Ӱ豸�� ��
		flag[0] = false;
	}
	void thread1Main()//���� ��
	{
		flag[1] = true;
		turn = 0;
		while (flag[0] && turn == 0); // 0���� �Ӱ迵���� �ִٸ� �ݺ�
		//�Ӱ豸��
		for (int i = 0; i < BUFFERSIZE; i++)
		{
			if (cri<=0) //0���� �۴ٸ� �ѱ�
			{
				continue;
			}
			cri--;
		}
		//�Ӱ豸�� ��
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

