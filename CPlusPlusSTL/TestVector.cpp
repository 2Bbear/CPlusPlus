#include <iostream>
#include <vector>
using namespace std;

#include "TestVector.h"


TestVector::TestVector(/* args */)
{
}

TestVector::~TestVector()
{
}

void TestVector::DoTest()
{
    cout<<"TestVector DoTest!!"<<endl;
    TestAssignVector();
}

/**
 * @brief Ư�����ҷ� ä���.
 * 
 */
void TestVector::TestAssignVector()
{
    vector<int> ivect;
    ivect.assign(2,10); //2���� 10���� ä���

    for(auto t: ivect)
    {
        cout<< t<< endl;
    }

}