#include"CPlusPlusSTLModuler.h"
#include"BaseSTLTester.h"

#include "TestTemplate.h"

CPlusPlusSTLModuler::CPlusPlusSTLModuler(eSTLType _eStlType)
{
    SetpSTLTester(_eStlType);
}
CPlusPlusSTLModuler::~CPlusPlusSTLModuler()
{
    delete pSTLTester;
}
void CPlusPlusSTLModuler::DoModule()
{
    pSTLTester->DoTest();
}

void CPlusPlusSTLModuler::SetpSTLTester(eSTLType _eStlType)
{
    switch (_eStlType)
    {
    case eSTLType::templateTest:
    {
        pSTLTester=new TestTemplate();
    } break;
    default:
        break;
    }
}