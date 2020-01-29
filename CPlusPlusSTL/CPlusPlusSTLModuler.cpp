#include"CPlusPlusSTLModuler.h"
#include"BaseSTLTester.h"

CPlusCPlusDesignPatternModuler::CPlusCPlusDesignPatternModuler(eSTLType _eStlType)
{
    SetpSTLTester(_eStlType);
}
void CPlusCPlusDesignPatternModuler::DoModule()
{
    pSTLTester->DoTest();
}

void CPlusCPlusDesignPatternModuler::SetpSTLTester(eSTLType _eStlType)
{
    switch (_eStlType)
    {
    case eSTLType::templaterTest:
    {
        pSTLTester=nullptr;
    } break;
    default:
        break;
    }
}