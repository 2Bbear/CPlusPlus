#include <iostream>
using namespace std;

#include "MyMonster.h"
#include "MyHuman.h"

/*
목표
1. 상속에 대한 개념을 이해한다.

2. 클래스 포인터에 대한 개념을 이해합니다.
    자식은 부모 클래스로 다뤄줄(묶을) 수 있다.

3. 상속과 클래스 포인터의 응용 방법을 몇개 살펴보기

4. 오버라이딩 overriding 기능의 응용
              OverLoading 매개변수가 다른 함수.

5. 클래스를 제대로 만드는 방법
*/

int main()
{
    MyMonster m1, m2, m3;

    MyHuman h1, h2;

    //사람이 몬스터를 공격한다!
    h1.Attack(&m1);

    //몬스터가 사람을 공격한다!
    m1.Attack(&h1);

    //사람과 몬스터의 hp를 출력
    cout << "사람 hp" << endl;
    h1.PrintHP();
    h2.PrintHP();


    cout << "몬스터 hp" << endl;
    m1.PrintHP();
    m2.PrintHP();
    m3.PrintHP();

    return 0;
}
