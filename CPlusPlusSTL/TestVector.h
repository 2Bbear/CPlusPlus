#ifndef TestVector_h
#define TestVector_h

#include "BaseSTLTester.h"


/**
 * @brief 
 * 1.�߰��� ���� ������ ����
 * 2. �˻� �ӵ��� �����͵� �ƴϳ�
 * 3. ���� ������ �����ϴ�, Ư�� �ε����� �������� �ٷ� ������ �����ϴ�.
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
    void TestAssignVector();

};




#endif