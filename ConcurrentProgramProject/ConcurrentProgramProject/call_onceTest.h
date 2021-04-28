#pragma once
namespace call_onceTest
{
	void Doit();
	
	/*
	스레드 안전한 싱글톤
	*/
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

	/*
	컴파일러별로 정적 블록에 대한 시맨틱이 안전하게 구성되는지 확인해야 한다. gcc를 쓸경우...허미
	*/
	class MeyersSingleton
	{
	public:
		static MeyersSingleton& getInstance()
		{
			static MeyersSingleton instance;
			return instance;
		}
	private:
		MeyersSingleton();
		~MeyersSingleton();
		MeyersSingleton(const MeyersSingleton&) = delete;
		MeyersSingleton& operator=(const MeyersSingleton&) = delete; //복사생성자 못하도록 막음
	};
	MeyersSingleton::MeyersSingleton() = default; // 이건 무슨 문법이지?
	MeyersSingleton::~MeyersSingleton() = default;

	void Doit3()
	{
		MeyersSingleton::getInstance();
	}
}

