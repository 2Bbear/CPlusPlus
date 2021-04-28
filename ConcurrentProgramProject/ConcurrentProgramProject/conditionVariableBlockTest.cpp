#include "stdafx.h"
#include "conditionVariableBlockTest.h"

using namespace std;
namespace conditionVariableBlockTest
{
	std::mutex m;
	std::condition_variable cv;
	bool dataReady{ false };
	
	void WrongWaitingForWork()
	{
		std::cout << "Worker: Waiting for work." << std::endl;

		unique_lock<mutex> lck(m);
		cv.wait(lck);
		

		cout << "Work done..." << endl;
	}
	void WrongSetDataReady()
	{
		cout << "Sender: Data is ready." << endl;
		cv.notify_one();
	}
	
	void Doit()
	{
		thread t1(WrongSetDataReady);
		thread t2(WrongWaitingForWork);

		t1.join();
		t2.join();
	}
	void DoTheWork()
	{
		cout << "Processing shared data." << endl;
	}
	void WaitingForWork()
	{
		std::cout << "Worker: Waiting for work." << std::endl;

		unique_lock<mutex> lck(m);
		cv.wait(lck, [] {return dataReady; });
		DoTheWork();

		cout << "Work done..." << endl;
	}
	void SetDataReady()
	{
		{
			lock_guard<mutex> lck(m);
			dataReady = true;
		}
		cout << "Sender: Data is ready." << endl;
		cv.notify_one();
	}

	void Doit2()
	{
		thread t1(SetDataReady);
		thread t2(WaitingForWork);

		t1.join();
		t2.join();
	}


}
