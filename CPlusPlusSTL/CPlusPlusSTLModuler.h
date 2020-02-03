#ifndef  CPlusPlusSTLModuler_h
#define CPlusPlusSTLModuler_h

class BaseSTLTester;

class CPlusPlusSTLModuler
{
public:
    enum eSTLType
    {
        templateTest,
        linkedList,
        vector,
    };
private:
    BaseSTLTester * pSTLTester;
public:
    CPlusPlusSTLModuler(eSTLType _eStlType);
    ~ CPlusPlusSTLModuler();
    void DoModule();
private:
    void SetpSTLTester(eSTLType _eStlType);
};
#endif