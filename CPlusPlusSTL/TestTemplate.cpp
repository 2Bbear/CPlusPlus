
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

}