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
int main()
{
	const char* lua_program = "print \"Hello From lua\"\n";
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	luaL_loadbuffer(L, lua_program, strlen(lua_program), "line");
	int err = lua_pcall(L, 0, 0, 0);
	if (err)
	{
		cout << "err:" << lua_tostring(L, -1);
		lua_pop(L, 1);
	}
	lua_close(L);
}