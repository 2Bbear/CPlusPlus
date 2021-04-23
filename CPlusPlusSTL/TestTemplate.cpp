
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
  
    cout << "ù��° ���� ����- ���� ����ġ 145.5f" << endl;
    kExpStack.push( 145.5f );

    cout << "�ι�° ���� ����- ���� ����ġ 183.25f" << endl;
    kExpStack.push( 183.25f );

    cout << "����° ���� ����- ���� ����ġ162.3f" << endl;
    kExpStack.push( 162.3f );


    int Count = kExpStack.Count();
    for( int i = 0; i < Count; ++i )
    {
        cout << "���� ����ġ->" << kExpStack.pop() << endl;
    }
}
void TestTemplate::TestMoney()
{
    Stack<__int64> kStackMoney;
    int Count=0;
    cout << "ù��° ���� ����- ���� �� 1000023" << endl;
    kStackMoney.push( 1000023 );

    cout << "�ι�° ���� ����- ���� �� 1000234" << endl;
    kStackMoney.push( 1000234 );

    cout << "����° ���� ����- ���� �� 1000145" << endl;
    kStackMoney.push( 1000145 );


    Count = kStackMoney.Count();
    for( int i = 0; i < Count; ++i )
    {
        cout << "���� ��->" << kStackMoney.pop() << endl;
    }
}