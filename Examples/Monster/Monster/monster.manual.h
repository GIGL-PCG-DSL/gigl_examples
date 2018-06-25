#include "GiglDefaultInclude.h"

class DungeonMonster
{
public:
	virtual ~DungeonMonster() {}
	int damage;
};

class Weapon
{
public:
	int damage;
};

class weakMonster : public DungeonMonster
{
public:
	weakMonster()
	{ damage = GetRandInt(2, 5); }
};

class strongMonster : public DungeonMonster
{
public:
	strongMonster(Weapon* _weapon) : weapon(_weapon)
	{ damage = GetRandInt(6, 9) + weapon->damage; }

	~strongMonster() { delete weapon; }

	Weapon* weapon;
};

class clubWeapon : public Weapon
{
public:
	clubWeapon()
	{ damage = GetRandInt(1, 3); }
};

class flailWeapon : public Weapon
{
public:
	flailWeapon()
	{ damage = GetRandInt(11, 19); }
};

DungeonMonster* Generate_DungeonMonster(bool is_hard);
Weapon* Generate_Weapon(bool is_hard);

DungeonMonster* Generate_DungeonMonster(bool is_easy)
{
	double probs[2] = { (is_easy ? 0.8 : 0.6), (is_easy ? 0.2 : 0.4) };
	int choice = GetRandChoiceFromProbs(probs, 2);

	switch (choice)
	{
	case 0:
		return new weakMonster;
	case 1:
		return new strongMonster(Generate_Weapon(is_easy));
	}

	return nullptr;
}

Weapon* Generate_Weapon(bool is_easy)
{
	double probs[2] = { (is_easy ? 1.0 : 0.7), (is_easy ? 0.0 : 0.3) };
	int choice = GetRandChoiceFromProbs(probs, 2);

	switch (choice)
	{
	case 0:
		return new clubWeapon;
	case 1:
		return new flailWeapon;
	}

	return nullptr;
}

void GenerateEasyRoomMonsters(DungeonMonster** list)
{
	for(int i = 0; i < 10; i++)
		list[i] = Generate_DungeonMonster(true);
}

void GenerateHardRoomMonsters(DungeonMonster** list)
{
	for(int i = 0; i < 10; i++)
		list[i] = Generate_DungeonMonster(false);
}
