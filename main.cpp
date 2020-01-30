
#include "CplusPlusGrammer/pointer/MyPointerModule.h"
#include "CPlusPlusDesignPattern/CPlusPlusDesignPatternModuler.h"
#include "CPlusPlusSTL/CPlusPlusSTLModuler.h"

#include "main.h"
#include<iostream>


int main()
{
    printf("main is started!!dd\n");
    DoSTLModule();
    return 0;
}

void DoPointerModule()
{
    MyPointerModule mpm;
    mpm.HowToUseFunctionoPointerTest1();
}

void DoDesignPatternModule()
{
    CPlusCPlusDesignPatternModuler DPM(CPlusCPlusDesignPatternModuler::ePatternType::commandPattern);
    DPM.DoModule();
    
}

void DoSTLModule()
{
    CPlusPlusSTLModuler cppStl(CPlusPlusSTLModuler::eSTLType::templateTest);
    cppStl.DoModule();
}