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
		//auto�� �� ������ �� ���ٰ� ������ ��ȯ���� ���� �Ҷ� �ٸ��� �����̴�.
		//��ȯ ���� �����ϱ� �����Ƽ�...
		cout << "fut.get(): " << fut.get() << endl;
	}
};