#include<iostream>
#include<list>
using namespace std;

#include "TestLinkedList.h"


void TestLinkedList::DoTest()
{
    cout<<"TestLinkedList DoTest!!"<<endl;
    TestMiddleInsert();
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
void TestLinkedList::TestUsingTemplateOperator()
{
    int count=10;
    list<int> intList;
    list<int>::iterator itr=intList.begin();
    for (size_t i = 0; i < count; i++)
    {
        intList.push_back(i*10);
    }

    intList.remove_if( Is_Over20_Under30< int >() );


    //전체 출력
    for (auto t : intList)
    {
        
        cout<<t<<endl;
    }

}

void TestLinkedList::TestSort()
{
    list< int > list1;

    list1.push_back(20);
    list1.push_back(10);
    list1.push_back(35);
    list1.push_back(15);
    list1.push_back(12);

    cout << "sort 올림차순" << endl;
    // 올림 차순으로 정렬한다.
    list1.sort();

    list<int>::iterator itr = list1.begin();

    for(auto t : list1)
    {
        cout<<t<<endl;
    }


}

void TestLinkedList::TestCustomSort()
{
    list<Item> list1;
    Item item1;
    item1.ItemCd=2;
    Item item2;
    item2.ItemCd=8;
    Item item3;
    item3.ItemCd=242;
    Item item4;
    item4.ItemCd=1;
    list1.push_back(item1);
    list1.push_back(item2);
    list1.push_back(item4);
    list1.push_back(item3);

    list1.sort(COMPARE_ITEM< Item >());

    for(auto t: list1)
    {
        cout<<t.ItemCd<<endl;
    }
}
/**
 * @brief 이것으로 템플릿은 컴파일시에 확인하여 적용된 다는 것을 알 수 있다.
 * 
*/
void TestLinkedList::TestCustomSort2()
{
    list<clItem> list1;
    clItem item1;
    item1.data=2;
    clItem item2;
    item2.data=8;
    clItem item3;
    item3.data=242;
    clItem item4;
    item4.data=1;

    list1.push_back(item1);
    list1.push_back(item2);
    list1.push_back(item4);
    list1.push_back(item3);

    list1.sort(CUSTOM_SORT< clItem >());

    for(auto t: list1)
    {
        cout<<t.data<<endl;
    }
}

/**
 * @brief 이터레이터를 이용한 list 복사방법
 * 
 */
void TestLinkedList::TestAssign()
{
    list<int> list1;
    list<int> list2;
    for(int i=0;i<10;++i)
    {
        list2.push_back(i);
    }
    list1.assign(list2.begin(),list2.end());

    for(auto t: list1)
    {
        cout<<t<<endl;
    }
}
/**
 * @brief list에 있는 값들의 순서를 뒤집고 싶을때
 * 
 */
void TestLinkedList::TestRevers()
{
    list<int> list1;
    for(int i=0;i<10;++i)
    {
        list1.push_back(i);
    }

    list1.reverse();

    for(auto t: list1)
    {
        cout<<t<<endl;
    }
}

void TestLinkedList::TestAllListSwap()
{
    list<int> list1;
    list<int> list2;
    for(int i=0;i<10;++i)
    {
        list1.push_back(i);
        list2.push_back(i*10);
    }

    list1.swap(list2);

    for(auto t: list1)
    {
        cout<<t<<endl;
    }
}
void TestLinkedList::TestMiddleInsert()
{
    list<int> list1;
    for(int i=0;i<10;++i)
    {
        list1.push_back(i);
    }

    list<int>::iterator itr=list1.begin();
    itr++;
    itr++;
    //이렇게 ++을 계속 해줘야 하는것인가?...
    list1.insert(itr,59);

    for(auto t: list1)
    {
        cout<<t<<endl;
    }
}