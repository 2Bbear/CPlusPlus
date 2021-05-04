#pragma once


namespace MySpinlock
{
	class MySpinlock
	{
		std::atomic_flag flag;
	public:
		void lock();
		void unlock();

	};
}

