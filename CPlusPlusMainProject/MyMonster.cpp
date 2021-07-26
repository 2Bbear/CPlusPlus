#include <iostream>
using namespace std;

#include "MyHuman.h"

#include "MyMonster.h"


void MyMonster::Attack(MyHuman* _human)
{
	_human->hp = _human->hp - damage;
}

void MyMonster::PrintHP()
{
	cout << hp << endl;
}
