#include<iostream>

extern "C" {			//c언어로 정의된 것 이라는 뜻
#include"include/lua.h"
#include"include/lauxlib.h"
//#include"include/lua.hpp"
//#include"include/luaconf.h"
#include"include/lualib.h"
}
#pragma comment(lib,"lua54.lib")
using namespace std;
int addnum_c(lua_State* L)
{
	int a = (int)lua_tonumber(L, -2);
	int b = (int)lua_tonumber(L, -1);
	int result = a + b;
	lua_pop(L, 3);
	lua_pushnumber(L, result);
	return 1;
}

int main()
{
	const char* lua_program = "print \"Hello From lua\"\n";
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	luaL_loadfile(L, "dragon.lua");
	int err = lua_pcall(L, 0, 0, 0);
	if (err)
	{
		cout << "err:" << lua_tostring(L, -1);
		lua_pop(L, 1);
	}
	//lua_getglobal(L, "pos_x");
	//lua_getglobal(L, "pos_y");
	//
	//int px = lua_tonumber(L, -2);
	//int py = lua_tonumber(L, -1);
	//lua_pop(L, 2);//이거 안하면 메모리 누수

	//cout << "pos_x =" << px << ", pos_y = " << py << endl;
	/*lua_getglobal(L, "plustwo");
	lua_pushnumber(L, 100);
	lua_pcall(L, 1, 1, 0);
	int res = lua_tonumber(L, -1);
	lua_pop(L, 1);*/
	
	//cout << "lua returned " << res << endl;
	lua_register(L, "c_addnum", addnum_c);
	
	lua_getglobal(L, "add_num");
	lua_pushnumber(L, 100);
	lua_pushnumber(L, 200);
	lua_pcall(L, 2, 1, 0);
	int result = (int)lua_tonumber(L, -1);
	printf("Result of addnum %d\n", result);
	lua_pop(L, 1);
	
	lua_close(L);
}