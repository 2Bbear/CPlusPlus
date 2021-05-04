#include "stdafx.h"
#include "MySpinlock.h"

namespace MySpinlock
{
	
	void MySpinlock::lock()
	{
		while (flag.test_and_set());
	}

	void MySpinlock::unlock()
	{
		flag.clear();
	}

	MySpinlock spin;

	void workOnResource()
	{
		spin.lock();
		//�Ӱ迵��
		spin.unlock();
	}

	void Doit()
	{
		std::thread t(workOnResource);
		std::thread t2(workOnResource);

		t.join();
		t2.join();
	}

}
