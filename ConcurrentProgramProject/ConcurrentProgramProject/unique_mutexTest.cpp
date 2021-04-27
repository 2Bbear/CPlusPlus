#include "stdafx.h"
#include "unique_mutexTest.h"


using namespace std;
namespace Unique_mutexTest
{
	struct CriticalData
	{
		mutex mut;
	};

	void deadLock(CriticalData& a, CriticalData& b);

	void Doit()
	{
		CriticalData c1;
		CriticalData c2;

		thread t1([&] {deadLock(c1, c2); });
		thread t2([&] {deadLock(c2, c1); });

		t1.join();
		t2.join();

	}

	void deadLock(CriticalData& a, CriticalData& b)
	{
		unique_lock<mutex> guard1(a.mut,defer_lock);
		cout << "Thread: " << this_thread::get_id() << "first mutex" << endl;
		this_thread::sleep_for(chrono::milliseconds(1));
		unique_lock<mutex> guard2(b.mut, defer_lock);
		cout << "Thread: " << this_thread::get_id() << "second mutex" << endl;
		cout << "Thread: " << this_thread::get_id() << "second mutex" << endl;
		lock(guard1, guard2);
	}
}
