
#include <iostream>
using namespace std;

#include "TestTemplate.h"

void TestTemplate::DoTest()
{
    std::cout<<"TestTemplate DoTest"<<std::endl;
    TestExp();
    TestMoney();

}

void TestTemplate::TestExp()
{
    ExpStack kExpStack;
  
    cout << "첫번째 게임 종료- 현재 경험치 145.5f" << endl;
    kExpStack.push( 145.5f );

    cout << "두번째 게임 종료- 현재 경험치 183.25f" << endl;
    kExpStack.push( 183.25f );

    cout << "세번째 게임 종료- 현재 경험치162.3f" << endl;
    kExpStack.push( 162.3f );


    int Count = kExpStack.Count();
    for( int i = 0; i < Count; ++i )
    {
        cout << "현재 경험치->" << kExpStack.pop() << endl;
    }
}
void TestTemplate::TestMoney()
{
    Stack<__int64> kStackMoney;
    int Count=0;
    cout << "첫번째 게임 종료- 현재 돈 1000023" << endl;
    kStackMoney.push( 1000023 );

    cout << "두번째 게임 종료- 현재 돈 1000234" << endl;
    kStackMoney.push( 1000234 );

    cout << "세번째 게임 종료- 현재 돈 1000145" << endl;
    kStackMoney.push( 1000145 );


    Count = kStackMoney.Count();
    for( int i = 0; i < Count; ++i )
    {
        cout << "현재 돈->" << kStackMoney.pop() << endl;
    }
}