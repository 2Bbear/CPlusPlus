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
 * @brief 특정원소로 채운다.
 * 
 */
void TestVector::TestAssignVector()
{
    vector<int> ivect;
    ivect.assign(2,10); //2개를 10으로 채운다

    for(auto t: ivect)
    {
        cout<< t<< endl;
    }

}