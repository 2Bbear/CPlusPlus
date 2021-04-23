#ifndef  TestLinkedList_h
#define TestLinkedList_h

#include "BaseSTLTester.h"


template<typename T> class Is_Over20_Under30 
{
public:
   bool operator( ) ( T& val ) 
   {
      return ( val >= 20 && val < 30 );
   }
};

template<typename T>  struct COMPARE_ITEM
{
  bool operator()( const T l, const T r ) const
  {
    // ���� �ÿ��� �ø� �������εȴ�. ���� �������� �ϰ� ������ < ���� > ��
    // �����ϸ� �ȴ�.
    return l.ItemCd < r.ItemCd;
  }
};

template<typename T> class CUSTOM_SORT
{
public:
    bool operator()(const T l, const T r) const
    {
        return l.data < r.data;
    }
};

struct Item
{
  int ItemCd;
  int num;  
};

#include<iostream>
using namespace std;

class clItem
{
public:
    int data;
    friend ostream& operator<<(ostream& os, const clItem& dt)
    {
        os << "d";
        return os;
    }
};

class TestLinkedList: public BaseSTLTester
{
public:
    void DoTest();
private:
    void TestList1();
    void TestUsingTemplateOperator();
    void TestSort();
    void TestCustomSort();
    void TestCustomSort2();
    void TestAssign();
    void TestRevers();
    void TestAllListSwap();
    void TestMiddleInsert();



};


#endif