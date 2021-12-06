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
	short	hp, maxhp;
	short x, y;
	STATE state;
	mutex state_lock;
	lua_State* L;
public:
	//NPC() = default;
	NPC():id(id),exp(0) ,state(ST_FREE),type(0) //player¿ë
	{
		
	}
	NPC(int id, int type, int x, int y) :state(ST_INGAME),id(id),type(type),x(x),y(y)
	{
		sprintf_s(name, "N%d", id);
		
	
	}
	~NPC() {}
	lua_State* Getlua()
	{
		return L;
	}
};

