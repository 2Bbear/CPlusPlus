#ifndef  TestTemplate_h
#define TestTemplate_h

#include <stdint.h> 

#include "BaseSTLTester.h"

// 경험치�?? ????��?�� ?�� ?��?�� 최�?? 개수
const int MAX_EXP_COUNT = 100;

class ExpStack
{
    public:
  ExpStack()
  {
    Clear();
  }

  // 초기?�� ?��?��.
  void Clear()
  {
    m_Count = 0;
  }

  // ?��?��?�� ????��?�� 개수
  int Count()
  {
    return m_Count;
  }

        // ????��?�� ?��?��?���?? ?��?���???
  bool IsEmpty()
  {
    return 0 == m_Count ? true : false;
  }

  // 경험치�?? ????��?��?��.
  bool push( float Exp )
  {
    // ????��?�� ?�� ?��?�� 개수�?? ?��?���?? 조사?��?��.
    if( m_Count >= MAX_EXP_COUNT )
    {
      return false;
    }

    // 경험치�?? ????�� ?�� 개수�?? ?��?�� ?��린다.
    m_aData[ m_Count ] = Exp;
    ++m_Count;

    return true; 
  }

  // ?��?��?��?�� 경험치�?? 빼낸?��.
  float pop()
  {
    // ????��?�� 것이 ?��?���?? 0.0f�?? 반환?��?��.
    if( m_Count  < 1 )
    {
      return 0.0f;
    }

    // 개수�?? ?��?�� 감소 ?�� 반환?��?��.
    --m_Count;
    return m_aData[ m_Count ];
  }

private:
  float  m_aData[MAX_EXP_COUNT];
  int    m_Count;
};

// ?��?�� ????��?�� ?�� ?��?�� 최�?? 개수
const int MAX_MONEY_COUNT = 100;

// ?�� ????�� ?��?�� ?��?��?��
class MoneyStack
{
public:
  MoneyStack()
  {
    Clear();
  }

  // 초기?�� ?��?��.
  void Clear()
  {
    m_Count = 0;
  }

  // ?��?��?�� ????��?�� 개수
  int Count()
  {
    return m_Count;
  }

  // ????��?�� ?��?��?���???��?���???
  bool IsEmpty()
  {
    return 0 == m_Count ? true : false;
  }

  // ?��?�� ????��?��?��.
  bool push( __int64 Money )
  {
    // ????�� ?�� ?�� ?��?�� 개수�?? ?��?���?? 조사?��?��.
    if( m_Count >= MAX_MONEY_COUNT )
    {
      return false;
    }

    // ????��?�� 개수�?? ?��?�� ?��린다.
    m_aData[ m_Count ] = Money;
    ++m_Count;

    return true; 
  }

  // ?��?��?��?�� ?��?�� 빼낸?��.
  __int64 pop()
  {
    // ????��?�� 것이 ?��?���?? 0?�� 반환?��?��.
    if( m_Count  < 1 )
    {
      return 0;
    }

    // 개수�?? ?��?�� 감소 ?�� 반환?��?��.
    --m_Count;
    return m_aData[ m_Count ];
  }

private:
  __int64  m_aData[MAX_MONEY_COUNT];
  int  m_Count;
};

const int MAX_COUNT = 100;
 
template <typename T> class Stack
{
public:
  Stack()
  {
    Clear();
  }

  // 초기?�� ?��?��.
  void Clear()
  {
    m_Count = 0;
  }

  // ?��?��?�� ????��?�� 개수
  int Count()
  {
    return m_Count;
  }

  // ????��?�� ?��?��?���?? ?��?���???
  bool IsEmpty()
  {
    return 0 == m_Count ? true : false;
  }

  // ?��?��?���?? ????��?��?��.
  bool push( T data )
  {
    // ????�� ?��?�� ?��?�� 개수�?? ?��?���?? 조사?��?��.
    if( m_Count >= MAX_COUNT )
    {
      return false;
    }

    // ????��?�� 개수�?? ?��?�� ?��린다.
    m_aData[ m_Count ] = data;
    ++m_Count;

    return true; 
  }

  // ?��?��?��?�� 빼낸?��.
  T pop()
  {
    // ????��?�� 것이 ?��?���?? 0?�� 반환?��?��.
    if( m_Count  < 1 )
    {
      return 0;
    }

    // 개수�?? ?��?�� 감소 ?�� 반환?��?��.
    --m_Count;
    return m_aData[ m_Count ];
  }

private:
  T  m_aData[MAX_COUNT];
  int    m_Count;
};

class TestTemplate:public BaseSTLTester
{
    
public:
    void DoTest();  
private:
    void TestExp();
    void TestMoney();
};

#endif