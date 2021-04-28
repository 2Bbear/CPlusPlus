#pragma once
namespace call_onceTest
{
	void Doit();
	
	/*
	������ ������ �̱���
	*/
	class MySingleton
	{
	private:
		static std::once_flag initInstanceFlag;
		static MySingleton* instance;
		MySingleton() = default;
		~MySingleton() = default;

	public:
		MySingleton(const MySingleton&) = delete; // �� �޼���� ����� �� ����.
		MySingleton& operator=(const MySingleton&) = delete;// �� �޼���� ����� �� ����

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
	�����Ϸ����� ���� ���Ͽ� ���� �ø�ƽ�� �����ϰ� �����Ǵ��� Ȯ���ؾ� �Ѵ�. gcc�� �����...���
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
		MeyersSingleton& operator=(const MeyersSingleton&) = delete; //��������� ���ϵ��� ����
	};
	MeyersSingleton::MeyersSingleton() = default; // �̰� ���� ��������?
	MeyersSingleton::~MeyersSingleton() = default;

	void Doit3()
	{
		MeyersSingleton::getInstance();
	}
}
