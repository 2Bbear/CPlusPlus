#pragma once

class BaseSTLTester;

class CPlusPlusSTLModuler
{
public:
    enum eSTLType
    {
        templateTest,
        linkedList,
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