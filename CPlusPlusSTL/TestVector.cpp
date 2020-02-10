#include <iostream>
#include <vector>
using namespace std;

#include "TestVector.h"


TestVector::TestVector(/* args */)
{
}

TestVector::~TestVector()
{
}

void TestVector::DoTest()
{
    cout<<"TestVector DoTest!!"<<endl;
    TestCharacter();
}

void TestVector::BasicVectorTest()
{
    vector<int> ivect;
    for(int i=0;i<10;++i)
    {
        ivect.push_back(i);
    }
    ivect.at(1)=20;
    vector<int>::iterator itr = ivect.begin();
    ivect.erase(itr);
    for(auto t: ivect)
    {
        cout<< t << endl;
    }
}

/**
 * @brief 특정원소로 채운다.
 * 
 */
void TestVector::TestAssignVector()
{
    // 갯수를 지정해서 채우기
    vector<int> ivect;
    ivect.assign(2,10); //2개를 10으로 채운다

    for(auto t: ivect)
    {
        cout<< t<< endl;
    }

}

void TestVector::TestEndItrVector()
{
    vector<int> ivect;
    for(int i=0;i<10;++i)
    {
        ivect.push_back(i);
    }
    vector<int>::iterator itr = ivect.end();

}

void TestVector::TestSwqpVector()
{
    vector<int> ivect;
    vector<int> ivect2;
    for(int i=0;i<10;++i)
    {
        ivect.push_back(i);
        ivect2.push_back(i*10);
    }

    ivect.swap(ivect2);
    for(auto t: ivect)
    {
        cout<< t << endl;
    }
}

void TestVector::TestCharacter()
{
    //유저 1
    CharacterInfo user1;
    user1.lv=10;
    //user1.name="Bill";
    //유저 2 
    CharacterInfo user2;
    user2.lv=80;
    //user2.name="Bill";
    //유저 3
    CharacterInfo user3;
    user3.lv=22;
    //user3.name="Bill";

    vector<CharacterInfo> roomUserList;
    roomUserList.push_back(user1);
    roomUserList.push_back(user2);
    roomUserList.push_back(user3);

    // 아직 방에 유저가 있다면 모두 삭제한다.
	if( false == roomUserList.empty() )
	{
		roomUserList.clear();
	}

	int UserCount = roomUserList.size();
	cout << "현재 방에 있는 유저 수: " << UserCount << endl;
}