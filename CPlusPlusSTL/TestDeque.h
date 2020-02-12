#ifndef TestDeque_h
#define TestDeque_h

#include "BaseSTLTester.h"

/**
 * @brief 삽입 삭제를 양쪽으로 할 수 있는 구조
 * 중간 삽입 삭제에 적합하지 않음
 * 
 */
class TestDeque: public BaseSTLTester
{
private:
    /* data */
public:
    TestDeque(/* args */);
    ~TestDeque();
public:
    virtual void DoTest()override;
private:
    

};




#endif