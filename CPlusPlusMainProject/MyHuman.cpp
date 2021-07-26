#include <iostream>
using namespace std;

#include "MyMonster.h"

#include "MyHuman.h"

void MyHuman::Attack(MyMonster* _monster)
{
	_monster->hp = _monster->hp - damage;
}

void MyHuman::PrintHP()
{
	cout << hp << endl;
}
