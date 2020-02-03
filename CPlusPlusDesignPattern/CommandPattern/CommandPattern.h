#ifndef  CommandPattern_h
#define CommandPattern_h
#include "../PatternTestInterface.h"
class CommandPattern :public PatternBase
{
public:
    CommandPattern(){}
    virtual void DoPattern();
};

#endif