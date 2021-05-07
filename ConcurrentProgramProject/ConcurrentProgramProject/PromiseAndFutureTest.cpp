#include "stdafx.h"
#include "PromiseAndFutureTest.h"
namespace PromiseAndFutureTest
{
	using std::string;

	void worker(std::promise<string>*p)
	{
		p->set_value("some data");//이 함수가 실행됨으로 인해 wait이 해제됨
	}
	void Doit()
	{
		std::promise<string> p;

		// 미래에 string 데이터를 돌려 주겠다는 약속.
		std::future<string> data = p.get_future();

		std::thread t(worker, &p);

		// 미래에 약속된 데이터를 받을 때 까지 기다린다.
		data.wait();

		// wait 이 리턴했다는 뜻이 future 에 데이터가 준비되었다는 의미.
		// 참고로 wait 없이 그냥 get 해도 wait 한 것과 같다.
		std::cout << "받은 데이터 : " << data.get() << std::endl;

		t.join();
	}

	std::condition_variable cv;
	std::mutex m;
	bool done = false;
	std::string info;

	void worker2() {
		{
			std::lock_guard<std::mutex> lk(m);
			info = "some data";  // 위의 p->set_value("some data") 에 대응
			done = true;
		}
		cv.notify_all();
	}
	void Doit2()
	{
		std::thread t(worker2);
		std::unique_lock<std::mutex> lk(m);
		cv.wait(lk, [] { return done; });  // 위의 data.wait() 이라 보면 된다.
		lk.unlock();
		std::cout << "받은 데이터 : " << info << std::endl;
		t.join();
	}

};
