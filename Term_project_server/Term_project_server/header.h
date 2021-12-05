#pragma once
#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <array>
#include <vector>
#include <mutex>
#include <unordered_set>
#include <concurrent_priority_queue.h>
#include <sqlext.h>  

#include "2021_°¡À»_protocol.h"
#include"define.h"
#include"type.h"

extern "C" {
#include "include\lua.h"
#include "include\lauxlib.h"
#include "include\lualib.h"
}


#pragma comment (lib, "lua54.lib")
#pragma comment (lib, "WS2_32.LIB")
#pragma comment (lib, "MSWSock.LIB")


using namespace std;

struct timer_event {
	int obj_id;
	chrono::system_clock::time_point	start_time;
	EVENT_TYPE ev;
	int target_id;
	constexpr bool operator < (const timer_event& _Left) const
	{
		return (start_time > _Left.start_time);
	}

};


