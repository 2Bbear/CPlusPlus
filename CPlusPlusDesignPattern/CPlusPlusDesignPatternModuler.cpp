#include "CPlusPlusDesignPatternModuler.h"

#include "PatternTestInterface.h"

#include "CommandPattern/CommandPattern.h"

void CPlusCPlusDesignPatternModuler::DoModule()
{
    PB->DoPattern();
    
}

CPlusCPlusDesignPatternModuler::CPlusCPlusDesignPatternModuler(ePatternType _ePType)
{
    ePType=_ePType;
    SetPB(ePType);
    
}

void CPlusCPlusDesignPatternModuler::SetPB(ePatternType _epType)
{
    switch (_epType)
    {
        case ePatternType::commandPattern:
        {
            PB = new CommandPattern();
        }break;
        case ePatternType::flyWeightPattern:
        {
            
        }break;
    }
}
CPlusCPlusDesignPatternModuler::~CPlusCPlusDesignPatternModuler()
{
    delete PB;
}
