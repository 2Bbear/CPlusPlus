#pragma once
namespace call_onceTest
{
	void Doit();
	class MySingleton
	{
	private:
		static std::once_flag initInstanceFlag;
		static MySingleton* instance;
		MySingleton() = default;
		~MySingleton() = default;

	public:
		MySingleton(const MySingleton&) = delete; // 이 메서드는 사용할 수 없음.
		MySingleton& operator=(const MySingleton&) = delete;// 이 메서드는 사용할 수 없음

		static MySingleton* getInstance()
		{
			std::call_once(initInstanceFlag, MySingleton::initSingleton);
			return instance;
		}
		static void initSingleton()
		{
			instance = new MySingleton();
		}
	};
	MySingleton* MySingleton::instance = nullptr;
	std::once_flag MySingleton::initInstanceFlag;
	void Doit2()
	{
		std::cout << "MySingleton::getInstance(): " << MySingleton::getInstance() << std::endl;
		std::cout << "MySingleton::getInstance(): " << MySingleton::getInstance() << std::endl;
	}

}

