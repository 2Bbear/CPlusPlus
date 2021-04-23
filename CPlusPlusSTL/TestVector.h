#ifndef TestVector_h
#define TestVector_h

#include "BaseSTLTester.h"

/**
 * @brief 캐릭터 정보를 담고 있는 구조체
 * 
 */
struct CharacterInfo
{
    int lv;
    char * name;

};



/**
 * @brief 
 * 1.중간에 삽입 삭제가 없다
 * 2. 검색 속도가 빠른것도 아니나
 * 3. 랜덤 접근이 가능하다, 특정 인덱스를 기준으로 바로 접근이 가능하다.
 * 
 */
class TestVector: public BaseSTLTester
{
private:
    /* data */
public:
    TestVector(/* args */);
    ~TestVector();
public:
    virtual void DoTest()override;
private:
    void BasicVectorTest();
    void TestAssignVector();
    void TestEndItrVector();
    void TestSwqpVector();
    void TestCharacter();

};




#endif