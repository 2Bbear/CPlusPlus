#include "stdafx.h"
#include "TaskTest.h"
using namespace std;
namespace TaskTest
{
	void Doit()
	{
		int res;
		thread t([&] {res = 2000 + 11; });
		t.join();
		cout << "res: " << res << endl;

		auto fut = async([] {return 2000 + 11; });
		//auto를 쓴 이유는 이 람다가 무엇을 반환할지 정의 할때 다르기 때문이다.
		//반환 값을 지정하기 귀찮아서...
		cout << "fut.get(): " << fut.get() << endl;
	}

	void Doit2()
	{
		auto begin = chrono::system_clock::now();
		auto asyncLazy = async(launch::deferred,
			[] {return std::chrono::system_clock::now(); });

		auto asyncEager = async(launch::async,
			[] {return chrono::system_clock::now(); });

		this_thread::sleep_for(chrono::seconds(1));

		auto lazyStart = asyncLazy.get() - begin;
		auto eagerStart = asyncEager.get() - begin;

		auto lazyDuration = chrono::duration<double>(lazyStart).count();
		auto eagerDuration = chrono::duration<double>(eagerStart).count();

		cout << "asyncLazy evaluated after : " << lazyDuration << " seconds." << endl;
		cout << "asyncEager evaluated after : "<< eagerDuration <<" seconds." << endl;

	}
	void Doit3()
	{
		auto fut = async([] {return 2011; });
		cout << fut.get() << endl;
		async(launch::async, [] {
			this_thread::sleep_for(chrono::seconds(2));
			cout<<"first thread"<<endl;
			});
		async(launch::async, [] {
			this_thread::sleep_for(chrono::seconds(1));
			cout << "second thread" << endl;
			});

		cout << "main thread" << endl;
	}
	
	static const int NUM = 100000000;
	
	long long getDotProduct(vector<int>& v , vector<int>& w)
	{
		auto vSize = v.size();

		auto future1 = async([&] {
			return inner_product(&v[0], &v[vSize / 4], &w[0], 0LL);
			});
		auto future2 = async([&] {
			return inner_product(&v[vSize/4], &v[vSize / 2], &w[vSize/4], 0LL);
			});
		auto future3 = async([&] {
			return inner_product(&v[vSize / 2], &v[vSize* 3/4], &w[vSize / 2], 0LL);
			});
		auto future4 = async([&] {
			return inner_product(&v[vSize *3/4], &v[vSize], &w[vSize* 3/4], 0LL);
			});

		return future1.get() + future2.get() + future3.get() + future4.get();
	}

	void Doit4()
	{
		random_device seed;
		mt19937 engine(seed());

		uniform_int_distribution<int> dist(0,100);

		vector<int> v, w;
		v.reserve(NUM);
		w.reserve(NUM);

		for (int i=0;i<NUM;++i)
		{
			v.push_back(dist(engine));
			v.push_back(dist(engine));
		}
		cout << "getDotProduct(v,w ): " << getDotProduct(v, w) << endl;
	}

	

	class SumUp
	{
	public:
		int operator()(int beg, int end)
		{
			long long int sum{ 0 };
			for (int i = beg; i < end; ++i)
			{
				sum += i;
			}

			return sum;
		}
	};
	void Doit5()
	{
		SumUp sumUp1;
		SumUp sumUp2;
		SumUp sumUp3;
		SumUp sumUp4;

		//태스크 래핑
		packaged_task<int(int, int)> sumTask1(sumUp1);
		packaged_task<int(int, int)> sumTask2(sumUp2);
		packaged_task<int(int, int)> sumTask3(sumUp3);
		packaged_task<int(int, int)> sumTask4(sumUp4);

		//퓨처 생성
		/*
		왜 task을 통해서 퓨처를 만들어야 하는것일까?
		task가 교신 채널의 프로미스이기 때문이다.
		*/
		future<int> sumResult1 = sumTask1.get_future();
		future<int> sumResult2 = sumTask2.get_future();
		future<int> sumResult3 = sumTask3.get_future();
		future<int> sumResult4 = sumTask4.get_future();

		//컨테이너에 태스크 넣기
		deque<packaged_task<int(int, int)>> allTasks;
		allTasks.push_back(move(sumTask1));
		allTasks.push_back(move(sumTask2));
		allTasks.push_back(move(sumTask3));
		allTasks.push_back(move(sumTask4));

		int begin{ 1 };
		int increment{ 2500 };
		int end = begin + increment;

		//별도의 스레드로 각 계산 수행
		while (not allTasks.empty())
		{
			packaged_task<int(int, int)> myTask = move(allTasks.front());
			allTasks.pop_front();
			thread sumThread(move(myTask),begin, end);
			begin = end;
			end += increment;
			sumThread.detach();
		}

		//결과 받기
		auto sum = sumResult1.get() + sumResult2.get() +
			sumResult3.get() + sumResult4.get();

		cout << "sum of 0 .. 10000 = " << sum << endl;
	}

	template<typename T, typename U>
	void add(T t, U u, decltype(t+u)* result)
	{
		*result = t + u;
	}
	template <typename T, typename U>
	auto add(T t, U u) -> decltype(t + u) {
		return t + u;
	}

};