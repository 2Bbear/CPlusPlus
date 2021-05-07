#include "stdafx.h"
#include "PromiseAndFutureTest.h"
namespace PromiseAndFutureTest
{
	using std::string;

	void worker(std::promise<string>*p)
	{
		p->set_value("some data");//�� �Լ��� ��������� ���� wait�� ������
	}
	void Doit()
	{
		std::promise<string> p;

		// �̷��� string �����͸� ���� �ְڴٴ� ���.
		std::future<string> data = p.get_future();

		std::thread t(worker, &p);

		// �̷��� ��ӵ� �����͸� ���� �� ���� ��ٸ���.
		data.wait();

		// wait �� �����ߴٴ� ���� future �� �����Ͱ� �غ�Ǿ��ٴ� �ǹ�.
		// ����� wait ���� �׳� get �ص� wait �� �Ͱ� ����.
		std::cout << "���� ������ : " << data.get() << std::endl;

		t.join();
	}

	std::condition_variable cv;
	std::mutex m;
	bool done = false;
	std::string info;

	void worker2() {
		{
			std::lock_guard<std::mutex> lk(m);
			info = "some data";  // ���� p->set_value("some data") �� ����
			done = true;
		}
		cv.notify_all();
	}
	void Doit2()
	{
		std::thread t(worker2);
		std::unique_lock<std::mutex> lk(m);
		cv.wait(lk, [] { return done; });  // ���� data.wait() �̶� ���� �ȴ�.
		lk.unlock();
		std::cout << "���� ������ : " << info << std::endl;
		t.join();
	}

};
