#include "stdafx.h"
#include "call_onceTest.h"

namespace call_onceTest
{
	std::once_flag onceFlag;
	void do_once()
	{
		std::call_once(onceFlag, []() {
			std::cout << "Only once." << std::endl;
			});
	}

	void Doit()
	{
		std::thread t1(do_once);
		std::thread t2(do_once);
		std::thread t3(do_once);
		std::thread t4(do_once);

		t1.join();
		t2.join();
		t3.join();
		t4.join();
	}
}

