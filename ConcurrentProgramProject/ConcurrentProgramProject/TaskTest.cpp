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
};