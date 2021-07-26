#pragma once

class MyMonster;

class MyHuman
{
public:
	int hp = 100;
	int damage = 10;
public:
	void Attack(MyMonster* _monster);
	void PrintHP();
};

