#pragma once
#include"header.h"
class NPC
{
public:
	char name[MAX_NAME_SIZE];
	int id;
	int type;
	int exp;
	atomic_bool is_active;
	short level;
	short damage;
	short	hp, maxhp;
	short x, y;
	STATE state;
	mutex state_lock;
	lua_State* L;
public:
	//NPC() = default;
	NPC():id(id),exp(0) ,state(ST_FREE),type(0) //player¿ë
	{
		damage = 10;
	}
	NPC(int id, int type, int x, int y) :state(ST_INGAME),id(id),type(type),x(x),y(y)
	{
		sprintf_s(name, "N%d", id);
		damage = 1;
	
	}
	~NPC() {}
	
};

