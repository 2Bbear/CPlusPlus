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
		//임계영역
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
