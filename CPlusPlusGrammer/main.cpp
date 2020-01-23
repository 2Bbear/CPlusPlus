#include<iostream>

#include "main.h"
#include "pointer/MyPointerModule.h"


int main()
{
    printf("main is started!!\n");
    DoPointerModule();
    return 0;
}

void DoPointerModule()
{
    MyPointerModule mpm;
    mpm.DynamicArrayInitializeTest1();
}

