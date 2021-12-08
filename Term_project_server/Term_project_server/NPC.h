#pragma once
#include"header.h"
class NPC
{
public:
	char name[MAX_NAME_SIZE];
	int id;
	int type;
	int exp;
	int target_id;
	atomic_bool is_active;
	short level;
	short damage;
	short	hp, maxhp;
	short x, y;
	bool is_recognize=false;
	
	STATE state;
	mutex state_lock;
	lua_State* L;
	mutex lua_lock;
public:
	//NPC() = default;
	NPC():id(id),exp(0) ,state(ST_FREE),type(0),is_active(true) //player¿ë
	{
		damage = 50;
		target_id = 0;
	}
	NPC(int id, int type, int x, int y, short level, short damgae,short maxhp) :
		state(ST_INGAME),id(id),type(type),x(x),y(y),
		level(level),damage(damgae),maxhp(maxhp)
	{
		sprintf_s(name, "N%d", id);
		
		target_id = 0;
	
	}
	~NPC() {}
	
};

