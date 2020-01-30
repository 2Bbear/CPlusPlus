#pragma once

#include <stdint.h> 

#include "BaseSTLTester.h"

// 경험치를 저장할 수 있는 최대 개수
const int MAX_EXP_COUNT = 100;

class ExpStack
{
    public:
  ExpStack()
  {
    Clear();
  }

  // 초기화 한다.
  void Clear()
  {
    m_Count = 0;
  }

  // 스택에 저장된 개수
  int Count()
  {
    return m_Count;
  }

        // 저장된 데이터가 없는가?
  bool IsEmpty()
  {
    return 0 == m_Count ? true : false;
  }

  // 경험치를 저장한다.
  bool push( float Exp )
  {
    // 저장할 수 있는 개수를 넘는지 조사한다.
    if( m_Count >= MAX_EXP_COUNT )
    {
      return false;
    }

    // 경험치를 저장 후 개수를 하나 늘린다.
    m_aData[ m_Count ] = Exp;
    ++m_Count;

    return true; 
  }

  // 스택에서 경험치를 빼낸다.
  float pop()
  {
    // 저장된 것이 없다면 0.0f를 반환한다.
    if( m_Count  < 1 )
    {
      return 0.0f;
    }

    // 개수를 하나 감소 후 반환한다.
    --m_Count;
    return m_aData[ m_Count ];
  }

private:
  float  m_aData[MAX_EXP_COUNT];
  int    m_Count;
};

// 돈을 저장할 수 있는 최대 개수
const int MAX_MONEY_COUNT = 100;

// 돈 저장 스택 클래스
class MoneyStack
{
public:
  MoneyStack()
  {
    Clear();
  }

  // 초기화 한다.
  void Clear()
  {
    m_Count = 0;
  }

  // 스택에 저장된 개수
  int Count()
  {
    return m_Count;
  }

  // 저장된 데이터가없는가?
  bool IsEmpty()
  {
    return 0 == m_Count ? true : false;
  }

  // 돈을 저장한다.
  bool push( __int64 Money )
  {
    // 저장 할 수 있는 개수를 넘는지 조사한다.
    if( m_Count >= MAX_MONEY_COUNT )
    {
      return false;
    }

    // 저장후 개수를 하나 늘린다.
    m_aData[ m_Count ] = Money;
    ++m_Count;

    return true; 
  }

  // 스택에서 돈을 빼낸다.
  __int64 pop()
  {
    // 저장된 것이 없다면 0을 반환한다.
    if( m_Count  < 1 )
    {
      return 0;
    }

    // 개수를 하나 감소 후 반환한다.
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

  // 초기화 한다.
  void Clear()
  {
    m_Count = 0;
  }

  // 스택에 저장된 개수
  int Count()
  {
    return m_Count;
  }

  // 저장된 데이터가 없는가?
  bool IsEmpty()
  {
    return 0 == m_Count ? true : false;
  }

  // 데이터를 저장한다.
  bool push( T data )
  {
    // 저장 할수 있는 개수를 넘는지 조사한다.
    if( m_Count >= MAX_COUNT )
    {
      return false;
    }

    // 저장후 개수를 하나 늘린다.
    m_aData[ m_Count ] = data;
    ++m_Count;

    return true; 
  }

  // 스택에서 빼낸다.
  T pop()
  {
    // 저장된 것이 없다면 0을 반환한다.
    if( m_Count  < 1 )
    {
      return 0;
    }

    // 개수를 하나 감소 후 반환한다.
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