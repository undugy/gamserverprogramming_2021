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
	NPC() = default;
	NPC(int id):is_active(false),id(id),exp(0) ,state(ST_FREE) //player¿ë
	{
		
	}
	NPC(int id, int type, int x, int y) :state(ST_INGAME), is_active(false) 
	{
		sprintf_s(name, "N%d", id);
	
	}
	~NPC() {}

};

