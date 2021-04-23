#ifndef TestHashMap_h
#define TestHashMap_h

#include "BaseSTLTester.h"


/**
 * @brief 
 * 수천 수만 정도의 대량의 자료를 보관하고 빠르게 검색하고 싶을때 사용하는 형태
 * 너무 빈번하고 자료를 삽입 삭제 하지 않을때 사용하기 적절하다
 */
class TestHashMap: public BaseSTLTester
{
private:
    /* data */
public:
    TestHashMap(/* args */);
    ~TestHashMap();
public:
    virtual void DoTest()override;
private:

};




#endif