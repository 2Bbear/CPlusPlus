#include "stdafx.h"
#include "shared_lockTest.h"


namespace shared_lockTest
{
	/*
	이 변수는 공유 변수 이며, 그 때문에 보호되어야 한다.
	*/
	std::map<std::string, int> teleBook{ 
		{"Dijkstra",1972},
		{"Scott",1976},
		{"Ritchie",1983}
	};

	std::shared_timed_mutex teleBookMutex; 

	void addToTeleBook(const std::string & na, int tele)
	{
		std::lock_guard<std::shared_timed_mutex> writerLock(teleBookMutex);
		std::cout << "STARTING UPDATE" << na;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		teleBook[na] = tele;
		std::cout << "... ENDING UPDATE " << na << std::endl;
	}

	void printNumber(const std::string& na)
	{
		std::shared_lock<std::shared_timed_mutex> readerLock(teleBookMutex);
		auto searchEntry = teleBook.find(na);
		if (searchEntry != teleBook.end())
		{
			std::cout << searchEntry->first << ": " << searchEntry->second << std::endl;
		}
		else
		{
			std::cout << na << " not found! " << std::endl;
		}
	}
	/*
	8개의 스레드가 전화번호부를 읽으려하고, 두 개의 스레드는 수정하려고 한다.
	읽을때는 같은 읽기 스레드들은 동시에 진입이 가능하지만 쓰기 스레드는 진입할 수 없다
	쓰기 스레드는 단독으로만 진입 가능하다. 같은 쓰기가 들어오려 할때 못오게 막는다
	*/
	void Doit()
	{
		std::cout << std::endl;

		std::thread reader1([] {printNumber("Scott"); });
		std::thread reader2([] {printNumber("Ritchie"); });
		std::thread w1([] {addToTeleBook("Scott",1968); });
		std::thread reader3([] {printNumber("Dijkstra"); });
		std::thread reader4([] {printNumber("Scott"); });
		std::thread w2([] {addToTeleBook("Bjarne", 1965); });
		std::thread reader5([] {printNumber("Scott"); });
		std::thread reader6([] {printNumber("Ritchie"); });
		std::thread reader7([] {printNumber("Scott"); });
		std::thread reader8([] {printNumber("Bjarne"); });


		reader1.join();
		reader2.join();
		reader3.join();
		reader4.join();
		reader5.join();
		reader6.join();
		reader7.join();
		reader8.join();
		w1.join();
		w2.join();

		std::cout << std::endl;

		std::cout << "\n The new telephone book" << std::endl;

		for (auto teleIt : teleBook)
		{
			std::cout << teleIt.first << " : " << teleIt.second << std::endl;
		}

		std::cout << std::endl;

	}
}