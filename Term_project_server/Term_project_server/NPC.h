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
	atomic_bool is_active=true;
	short level;
	short damage;
	short	hp, maxhp;
	short x, y;
	short origin_x, origin_y;
	atomic_bool is_recognize=false;
	
	STATE state;
	mutex state_lock;
	lua_State* L;
	mutex lua_lock;
public:
	//NPC() = default;
	NPC():id(id),exp(0) ,state(ST_FREE),type(0) //player용
	{
		//cout << "Player 초기화";
		damage = 50;
		target_id = 0;
		origin_x = 0;
		origin_y = 0;
	}
	NPC(int id, int type, int x, int y,int exp, short level, short damgae,short maxhp,const char* sp_name) :
		state(ST_INGAME),id(id),type(type),x(x),y(y),exp(exp),
		level(level),damage(damgae),maxhp(maxhp),origin_x(x),origin_y(y),hp(maxhp)
	{
		sprintf_s(name, "%s-%d", sp_name, id);
		
		target_id = 0;
	
	}
	~NPC() {}
	
};

