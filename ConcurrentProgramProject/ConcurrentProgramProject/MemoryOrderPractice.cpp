#include "stdafx.h"
#include "MemoryOrderPractice.h"


namespace MemoryOrderPractice
{
	using namespace std;

	std::atomic<bool> condition{false};
	void MyUnLock()
	{
		condition.store(false,std::memory_order_seq_cst);
	}
	void MyLock()
	{
		bool ts = false;
		while (condition.compare_exchange_strong(ts,true,std::memory_order_seq_cst)==false) {}
	}

	void Insert(int _a, int *_ret )
	{
		MyLock();
		*_ret = _a;
		MyUnLock();
	}

	void Test1()
	{
		int result = 0;
		thread a(Insert,10,&result);
		thread b(Insert, 20, &result);

		a.join();
		b.join();

		cout << result << endl;

		
	}
};
