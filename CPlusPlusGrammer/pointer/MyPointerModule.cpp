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
 * @brief * 연산자는 + 연산자보다 우선순위가 높다.
 * 
*/
void MyPointerModule::PointerAndArrayTest2()
{
    int arr [10]={1,2,3,4,5,6,7,8,9,10};

    cout<<*(arr+2)<<endl;
    cout<<arr[2]<<endl;
}
/**
 * @brief c++11 에서 가능한 문법
 * arg는 반드시 널포인터여야 한다.
*/
void MyPointerModule::PointerNulptr_tTest1(std::nullptr_t arg)
{
    printf("This is nullptr");
}

/**
 * @brief 상수 문자열의 저장되는 메모리가 다르게 잡히는 테스트
*/
void MyPointerModule::DiffrentAssignMemoryTest1()
{
    char strTest1 []="Alex";
    const char * strTest2 ="Alex";
    cout<<strTest1<<endl;
}
/**
 * @brief std cout의 경웅 char *에 대해 주소값을 출력하지 않고, 그것을 문자로 변경해서 출력하려고 한다
*/
void MyPointerModule::STDCOUTcharPointerErrorTest1()
{
    char c='Q';
    int a=10;
    cout<<&c;//문자열을 출력하려고 하다보니 \n을 만나기 전까지 무작정 출력하는 문제가 있다.
    cout<<&a<<endl;


}

