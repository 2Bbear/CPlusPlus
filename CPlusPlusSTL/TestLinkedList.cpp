#include<iostream>
#include<list>
using namespace std;

#include "TestLinkedList.h"


void TestLinkedList::DoTest()
{
    cout<<"TestLinkedList DoTest!!"<<endl;
    TestList1();
}

void TestLinkedList::TestList1()
{
    int count=10;
    list<int> intList;
    list<int>::iterator itr=intList.begin();
    for (size_t i = 0; i < count; i++)
    {
        intList.push_back(i);
    }
    //전체 출력
    for (auto t : intList)
    {
        
        cout<<t<<endl;
    }
    //특정 위치에 삽입
    intList.insert(itr,100);
    //특정 위치 제거
    itr++;
    intList.erase(itr);
     //전체 출력
    for (auto t : intList)
    {
        
        cout<<t<<endl;
    }



    
    
    
}