
#include"BaseSTLTester.h"
#include"TestLinkedList.h"

#include "TestTemplate.h"

#include"CPlusPlusSTLModuler.h"

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
    case eSTLType::linkedList:
    {
        pSTLTester = new TestLinkedList();
    }break;
    default:
        break;
    }
}