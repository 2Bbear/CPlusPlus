#include "MyPointerModule.h"

void MyPointerModule::PointerRefferenceTest1()
{
    int temp=10;
    int * pTemp;
    pTemp = &temp;
    temp=40;
    *pTemp=4030;

    printf("%d",*pTemp);
}
void MyPointerModule::PointerAndArrayTest1()
{
    int intArray [10];
    int * pIArr;
    pIArr = intArray;

    for (size_t i = 0; i < 10; i++)
    {
        intArray[i] = i;
    }

    pIArr+=4;
    *pIArr=2000;

    for (size_t i = 0; i < 10; i++)
    {
        printf("%d\n",intArray[i]);
    }
}


/**
 * @brief c++11 에서 가능한 문법
 * arg는 반드시 널포인터여야 한다.
*/
void MyPointerModule::PointerNulptr_tTest1(std::nullptr_t arg)
{
    printf("This is nullptr");
}