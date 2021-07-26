#pragma once

class MyHuman;

class MyMonster
{
public:
	int hp = 100;
	int damage = 20;
public:
	void Attack(MyHuman * _human);
	void PrintHP();
};

