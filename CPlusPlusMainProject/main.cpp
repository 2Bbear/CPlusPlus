#include <iostream>
using namespace std;

#include "MyMonster.h"
#include "MyHuman.h"

/*
��ǥ
1. ��ӿ� ���� ������ �����Ѵ�.

2. Ŭ���� �����Ϳ� ���� ������ �����մϴ�.
    �ڽ��� �θ� Ŭ������ �ٷ���(����) �� �ִ�.

3. ��Ӱ� Ŭ���� �������� ���� ����� � ���캸��

4. �������̵� overriding ����� ����
              OverLoading �Ű������� �ٸ� �Լ�.

5. Ŭ������ ����� ����� ���
*/

int main()
{
    MyMonster m1, m2, m3;

    MyHuman h1, h2;

    //����� ���͸� �����Ѵ�!
    h1.Attack(&m1);

    //���Ͱ� ����� �����Ѵ�!
    m1.Attack(&h1);

    //����� ������ hp�� ���
    cout << "��� hp" << endl;
    h1.PrintHP();
    h2.PrintHP();


    cout << "���� hp" << endl;
    m1.PrintHP();
    m2.PrintHP();
    m3.PrintHP();

    return 0;
}
