#pragma once

class BaseSTLTester;

class CPlusCPlusDesignPatternModuler
{
public:
    enum eSTLType
    {
        templaterTest,
    };
private:
    BaseSTLTester * pSTLTester;
public:
    CPlusCPlusDesignPatternModuler(eSTLType _eStlType);
    void DoModule();
private:
    void SetpSTLTester(eSTLType _eStlType);
};