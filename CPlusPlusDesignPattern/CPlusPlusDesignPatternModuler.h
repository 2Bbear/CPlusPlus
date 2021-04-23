#pragma once
class PatternBase;
class PatternTestInterface;
class CPlusCPlusDesignPatternModuler
{
public:
    enum ePatternType
    {
        commandPattern=0,
        flyWeightPattern
    };
public:
    PatternBase * PB;
private:
    ePatternType ePType;
public:
    CPlusCPlusDesignPatternModuler(ePatternType _ePType);
    ~CPlusCPlusDesignPatternModuler();
  
    void DoModule();
private:
    void SetPB(ePatternType _epType);
    
};