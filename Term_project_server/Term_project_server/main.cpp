
#include"header.h"
#include"Player.h"
#include"DB.h"
HANDLE g_h_iocp;
SOCKET g_s_socket;

concurrency::concurrent_priority_queue <timer_event> timer_queue;
array<NPC*, MAX_USER + MAX_NPC>clients;
//-------에러확인---------------------------------

void error_display(int err_no);//에러 확인용

bool is_near(int a, int b);//시야에 있는지 체크
bool is_npc(int id);//npc인지 체크
bool is_area(int id) {
	if (MONSTER_AREA < abs(clients[id]->x - clients[id]->origin_x)) return false;
	if (MONSTER_AREA < abs(clients[id]->y - clients[id]->origin_y)) return false;
	return true;
}
bool is_player(int id)
{
	return (id >= 0) && (id < MAX_USER);
}//player 인지 체크
int get_new_id();//새로운 아이디 할당(로그인 아이디 아님)
void type_move(int npc_id);
//--------------------------------------------

void do_timer();
void worker();
timer_event SetTimerEvent(int obj_id,int target_id,EVENT_TYPE ev,int seconds);
//--------------------------------------

void Disconnect(int c_id);
void process_packet(int client_id, unsigned char* p);
void do_timer();
void Initialize_NPC();
void do_npc_move(int npc_id);
int API_get_x(lua_State* L);
int API_get_y(lua_State* L);
int API_recognize_player(lua_State* L);
int API_regen(lua_State* L);
void attack(int client_id, int target_id);





int main()
{
	setlocale(LC_ALL, "korean");
	wcout.imbue(locale("korean"));
	WSADATA WSAData;
	DB::GetInst();
	for (int i = 0; i < MAX_USER; ++i)
		clients[i] = new Player;
	Initialize_NPC();
	cout << "npc초기화끝" << endl;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_s_socket, SOMAXCONN);

	g_h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_s_socket), g_h_iocp, 0, 0);

	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	char	accept_buf[sizeof(SOCKADDR_IN) * 2 + 32 + 100];
	EXP_OVER	accept_ex;
	*(reinterpret_cast<SOCKET*>(&accept_ex._net_buf)) = c_socket;
	ZeroMemory(&accept_ex._wsa_over, sizeof(accept_ex._wsa_over));
	accept_ex._comp_op = OP_ACCEPT;

	AcceptEx(g_s_socket, c_socket, accept_buf, 0, sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16, NULL, &accept_ex._wsa_over);
	cout << "Accept Called\n";
	

	cout << "Creating Worker Threads\n";

	
	vector <thread> worker_threads;
	//thread ai_thread{ do_ai };
	thread timer_thread{ do_timer };
	for (int i = 0; i < 6; ++i)
		worker_threads.emplace_back(worker);
	for (auto& th : worker_threads)
		th.join();

	//ai_thread.join();
	timer_thread.join();
	for (auto cl : clients) {
		if (true == is_npc(cl->id))break;
		
		if (ST_INGAME == cl->state)
		{
			
			DB::GetInst()->Save_Data(cl->id);
			Disconnect(cl->id);
		}
	}
	closesocket(g_s_socket);
	DB::DestroyInst();
	WSACleanup();
	return 0;
}




void Activate_Player_Move_Event(int target, int player_id)
{
	EXP_OVER* exp_over = new EXP_OVER;
	exp_over->_comp_op = OP_NPC_MOVE;
	exp_over->_target = player_id;
	PostQueuedCompletionStatus(g_h_iocp, 1, target, &exp_over->_wsa_over);
}

void error_display(int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, 0);
	wcout << lpMsgBuf << endl;
	//while (true);
	LocalFree(lpMsgBuf);
}


bool is_near(int a, int b)
{
	if (RANGE < abs(clients[a]->x - clients[b]->x)) return false;
	if (RANGE < abs(clients[a]->y - clients[b]->y)) return false;
	return true;
}
bool is_npc(int id)
{
	return (id >= NPC_ID_START) && (id <= NPC_ID_END);
}
int get_new_id()
{
	static int g_id = 0;
	Player* p=NULL;
	for (int i = 0; i < MAX_USER; ++i) {
		p = (Player*)clients[i];
			p->state_lock.lock();
		if (ST_FREE == p->state) {
			p->state = ST_ACCEPT;
			p->state_lock.unlock();
			return i;
		}
		p->state_lock.unlock();
	}
	cout << "Maximum Number of Clients Overflow!!\n";
	return -1;
}



void Disconnect(int c_id)
{
	Player* cl = (Player*)clients[c_id];
	cl->m_vl.lock();
	unordered_set <int> my_vl = cl->m_viewlist;
	cl->m_vl.unlock();
	for (auto& other : my_vl) {
		Player* target = (Player*)clients[other];
		if (true == is_npc(target->id)) continue;
		if (ST_INGAME != target->state)
			continue;
		target->m_vl.lock();
		if (0 != target->m_viewlist.count(c_id)) {
			target->m_viewlist.erase(c_id);
			target->m_vl.unlock();
			target->send_remove_object(c_id);
		}
		else target->m_vl.unlock();
	}
	cl->state_lock.lock();
	closesocket(cl->m_socket);
	cl->state = ST_FREE;
	cl->state_lock.unlock();
}

void process_packet(int client_id, unsigned char* p)
{
	unsigned char packet_type = p[1];
	Player* cl = reinterpret_cast<Player*>(clients[client_id]);
	
	switch (packet_type) {
	case CS_PACKET_LOGIN: {
		cs_packet_login* packet = reinterpret_cast<cs_packet_login*>(p);
		//패킷의 name을 가지고 있는 플레이어가 있는지 체크
		for (int i = 0; i < MAX_USER; ++i)
		{	
			clients[i]->state_lock.lock();
			if (ST_FREE== clients[i]->state){
				clients[i]->state_lock.unlock();
				continue;
		
			}
			clients[i]->state_lock.unlock();
		
			if (strcmp(clients[i]->name, packet->name) == 0)
			{
				cl->send_login_fail(0);
				return;
			}
		}
		//DB::GetInst()->db_l.lock();
		DB::GetInst()->get_login_data(cl->id, packet->name);
		//DB::GetInst()->db_l.unlock();
		cl->damage = 3;
		//strcpy_s(cl->name, packet->name);
		cl->send_login_ok_packet();
		cl->send_status_change_packet();
		Player* other_cl = NULL;
		
		cl->state_lock.lock();
		cl->state = ST_INGAME;
		cl->state_lock.unlock();
		//if(c)
		timer_queue.push(SetTimerEvent(client_id, client_id, EVENT_PLAYER_HILL, 5));
		// 새로 접속한 플레이어의 정보를 주위 플레이어에게 보낸다
		for (auto other : clients) {
			
			other_cl = (Player*)other;
			other_cl->state_lock.lock();
			if (ST_INGAME != other_cl->state) {
				other_cl->state_lock.unlock();
				continue;
			}
			other_cl->state_lock.unlock();
			if (true == is_npc(other->id)) continue;
			if (other->id == client_id) continue;
			if (false == is_near(other_cl->id, client_id))
				continue;

			other_cl->m_vl.lock();
			other_cl->m_viewlist.insert(client_id);
			other_cl->m_vl.unlock();
			sc_packet_put_object packet;
			packet.id = client_id;
			strcpy_s(packet.name, cl->name);
			packet.object_type = 0;
			packet.size = sizeof(packet);
			packet.type = SC_PACKET_PUT_OBJECT;
			packet.x = cl->x;
			packet.y = cl->y;
			other_cl->do_send(sizeof(packet), &packet);
		}

		// 새로 접속한 플레이어에게 주위 객체 정보를 보낸다
		for (auto other : clients) {
			if (other->id == client_id) continue;
			
			other_cl = (Player*)other;
			other_cl->state_lock.lock();
			if (ST_INGAME != other_cl->state) {
				other_cl->state_lock.unlock();
				continue;
			}
			other_cl->state_lock.unlock();

			if (false == is_near(other_cl->id, client_id))
				continue;

			cl->m_vl.lock();
			cl->m_viewlist.insert(other_cl->id);
			cl->m_vl.unlock();
			if (true == is_npc(other_cl->id)){

				timer_queue.push(SetTimerEvent(other_cl->id, other_cl->id, EVENT_NPC_MOVE, 1));
			}
			sc_packet_put_object packet;
			packet.id = other_cl->id;
			strcpy_s(packet.name, other_cl->name);
			packet.object_type = 0;
			packet.size = sizeof(packet);
			packet.type = SC_PACKET_PUT_OBJECT;
			packet.x = other_cl->x;
			packet.y = other_cl->y;
			cl->do_send(sizeof(packet), &packet);
		}
		break;
	}
	case CS_PACKET_MOVE: {
		cs_packet_move* packet = reinterpret_cast<cs_packet_move*>(p);
		cl->m_last_move_time = packet->move_time;
		int x = cl->x;
		int y = cl->y;
		switch (packet->direction) {
		case 0: if (y > 0) y--; break;
		case 1: if (y < (WORLD_HEIGHT - 1)) y++; break;
		case 2: if (x > 0) x--; break;
		case 3: if (x < (WORLD_WIDTH - 1)) x++; break;
		default:
			cout << "Invalid move in client " << client_id << endl;
			exit(-1);
		}
		cl->x = x;
		cl->y = y;

		unordered_set <int> nearlist;
		for (auto other : clients) {
			if (other->id == client_id)
				continue;
			if (ST_INGAME != other->state)
				continue;
			if (false == is_near(client_id, other->id))
				continue;
			
			if (true == is_npc(other->id) ) 
			{
				if(other->is_active==true)
					nearlist.insert(other->id);
			}
			else
				nearlist.insert(other->id);
				//Activate_Player_Move_Event(other->id, cl->id);
				//timer_queue.push(SetTimerEvent(other->id, other->id, EVENT_NPC_MOVE, 1));
			//}
			
		}

		cl->send_move_packet(cl->id);

		cl->m_vl.lock();
		unordered_set <int> my_vl{ cl->m_viewlist };
		cl->m_vl.unlock();
		Player* near_cl = NULL;
		// 새로시야에 들어온 플레이어 처리
		for (auto other : nearlist) {
			if (0 == my_vl.count(other)) {
				cl->m_vl.lock();
				cl->m_viewlist.insert(other);
				cl->m_vl.unlock();
				cl->send_put_object(other);

				if (true == is_npc(other)) {
					if(clients[other]->is_active == true)
					timer_queue.push(SetTimerEvent(other, other, EVENT_NPC_MOVE, 1));
					continue;
				}
				near_cl = (Player*)clients[other];
				near_cl->m_vl.lock();
				if (0 == near_cl->m_viewlist.count(cl->id)) {
					near_cl->m_viewlist.insert(cl->id);
						near_cl->m_vl.unlock();
					near_cl->send_put_object( cl->id);
				}
				else {
					near_cl->m_vl.unlock();
					near_cl->send_move_packet(cl->id);
				}
			}
			// 계속 시야에 존재하는 플레이어 처리
			else {
				if (true == is_npc(other)) continue;
				near_cl = (Player*)clients[other];
				near_cl->m_vl.lock();
				if (0 != near_cl->m_viewlist.count(cl->id)) {
					near_cl->m_vl.unlock();
					near_cl->send_move_packet(cl->id);
				}
				else {
					near_cl->m_viewlist.insert(cl->id);
					near_cl->m_vl.unlock();
					near_cl->send_put_object(cl->id);
				}
			}
		}
		// 시야에서 사라진 플레이어 처리
		for (auto other : my_vl) {
			if (0 == nearlist.count(other)) {
				cl->m_vl.lock();
				cl->m_viewlist.erase(other);
				cl->m_vl.unlock();
				cl->send_remove_object( other);
				
				if (true == is_npc(other)) continue;
				near_cl = (Player*)clients[other];
				near_cl->m_vl.lock();
				if (0 != near_cl->m_viewlist.count(cl->id)) {
					near_cl->m_viewlist.erase(cl->id);
					near_cl->m_vl.unlock();
					near_cl->send_remove_object(cl->id);
				}
				else near_cl->m_vl.unlock();
			}
		}
		for (auto npc : my_vl)
		{
			if (false == is_npc(npc))continue;
			if (false == clients[npc]->is_active)continue;
			EXP_OVER* exp_over = new EXP_OVER;
			exp_over->_comp_op = OP_TRACKING_PLAYER;
			exp_over->_target = client_id;
			PostQueuedCompletionStatus(g_h_iocp, 1, npc, &exp_over->_wsa_over);
		}
		break;
	}
	case CS_PACKET_ATTACK: {
		//공격검사,따라오는 이벤트 API,움직임 로직(do_npc_move())
		cl->m_vl.lock();
		auto near_vl = cl->m_viewlist;
		cl->m_vl.unlock();
		for (auto &monster : near_vl)
		{
			if (false == is_npc(monster))continue;
			if (false == clients[monster]->is_active)continue;
			attack(client_id, monster);
		}
		break;
	}
	case CS_PACKET_TELEPORT: {
		
		int myid = cl->id;
		int x = rand() % WORLD_WIDTH;
		int y = rand() % WORLD_HEIGHT;
		cl->x=x;
		cl->y = y;
		cl->send_move_packet(x, y, myid);
		break;
	}
	}
}
//void start_event()
void do_timer() {

	while (true) {
		while (true) {
			timer_event ev;
			if (!timer_queue.try_pop(ev))continue;
			auto start_t = chrono::system_clock::now();
			if (ev.start_time <= start_t) {
				switch (ev.ev) {
				case EVENT_NPC_MOVE: {

					EXP_OVER* ex_over = new EXP_OVER;
					ex_over->_comp_op = OP_NPC_MOVE;
					PostQueuedCompletionStatus(g_h_iocp, 1, ev.obj_id, &ex_over->_wsa_over);
					break;
				}


				case EVENT_PLAYER_HILL: {
					
					EXP_OVER* ex_over = new EXP_OVER;
					ex_over->_comp_op = OP_PLAYER_HILL;
					PostQueuedCompletionStatus(g_h_iocp, 1, ev.obj_id, &ex_over->_wsa_over);
					break;
				}
				case EVENT_NPC_ATTACK: {
					EXP_OVER* ex_over = new EXP_OVER;
					ex_over->_comp_op = OP_NPC_ATTACK;
					ex_over->_target = ev.target_id;
					PostQueuedCompletionStatus(g_h_iocp, 1, ev.obj_id, &ex_over->_wsa_over);
					break;
				}
				case EVENT_REGEN: {
					EXP_OVER* ex_over = new EXP_OVER;
					ex_over->_comp_op = OP_REGEN;
					PostQueuedCompletionStatus(g_h_iocp, 1, ev.obj_id, &ex_over->_wsa_over);
					break;
				}
				}
			}
			else {
				timer_queue.push(ev);
				break;
			}
		}
		
		this_thread::sleep_for(10ms);
	}
}

void worker()
{
	for (;;) {
		DWORD num_byte;
		LONG64 iocp_key;
		WSAOVERLAPPED* p_over;
		BOOL ret = GetQueuedCompletionStatus(g_h_iocp, &num_byte, (PULONG_PTR)&iocp_key, &p_over, INFINITE);
		//cout << "GQCS returned.\n";
		int client_id = static_cast<int>(iocp_key);
		EXP_OVER* exp_over = reinterpret_cast<EXP_OVER*>(p_over);
		if (FALSE == ret) {
			int err_no = WSAGetLastError();
			cout << "GQCS Error : ";
			error_display(err_no);
			cout << endl;
			
			DB::GetInst()->Save_Data(client_id);
			Disconnect(client_id);
			if (exp_over->_comp_op == OP_SEND)
				delete exp_over;
			continue;
		}

		switch (exp_over->_comp_op) {
		case OP_RECV: {
			if (num_byte == 0) {
				DB::GetInst()->Save_Data(client_id);
				
				Disconnect(client_id);
				continue;
			}
			Player* cl =(Player*) clients[client_id];
			int remain_data = num_byte + cl->m_prev_size;
			unsigned char* packet_start = exp_over->_net_buf;
			int packet_size = packet_start[0];

			while (packet_size <= remain_data) {
				process_packet(client_id, packet_start);
				remain_data -= packet_size;
				packet_start += packet_size;
				if (remain_data > 0) packet_size = packet_start[0];
				else break;
			}

			if (0 < remain_data) {
				cl->m_prev_size = remain_data;
				memcpy(&exp_over->_net_buf, packet_start, remain_data);
			}
			cl->do_recv();
			break;
		}
		case OP_SEND: {
			if (num_byte != exp_over->_wsa_buf.len) {
				DB::GetInst()->Save_Data(client_id);
				Disconnect(client_id);
			}
			delete exp_over;
			break;
		}
		case OP_ACCEPT: {
			cout << "Accept Completed.\n";
			SOCKET c_socket = *(reinterpret_cast<SOCKET*>(exp_over->_net_buf));
			int new_id = get_new_id();
			if (-1 == new_id) {
				cout << "Maxmum user overflow. Accept aborted.\n";
			}
			else {
				Player* cl = (Player*)clients[new_id];
				cl->id = new_id;
				//cout << new_id << endl;
				cl->m_prev_size = 0;
				cl->m_recv_over._comp_op = OP_RECV;
				cl->m_recv_over._wsa_buf.buf = reinterpret_cast<char*>(cl->m_recv_over._net_buf);
				cl->m_recv_over._wsa_buf.len = sizeof(cl->m_recv_over._net_buf);
				ZeroMemory(&cl->m_recv_over._wsa_over, sizeof(cl->m_recv_over._wsa_over));
				cl->m_socket = c_socket;

				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), g_h_iocp, new_id, 0);
				cl->do_recv();
			}
			
			ZeroMemory(&exp_over->_wsa_over, sizeof(exp_over->_wsa_over));
			c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
			*(reinterpret_cast<SOCKET*>(exp_over->_net_buf)) = c_socket;
			AcceptEx(g_s_socket, c_socket, exp_over->_net_buf + 8, 0, sizeof(SOCKADDR_IN) + 16,
				sizeof(SOCKADDR_IN) + 16, NULL, &exp_over->_wsa_over);
		}
					  break;
		case OP_NPC_MOVE: {
			delete exp_over;
			do_npc_move(client_id);
			break;
		}
		case OP_PLAYER_HILL: {
			delete exp_over;
			clients[client_id]->state_lock.lock();
			if (clients[client_id]->state != ST_INGAME) {
				clients[client_id]->state_lock.unlock();
				break;
			}
			clients[client_id]->state_lock.unlock();
			
			((Player*)clients[client_id])->player_hill();
			timer_queue.push(SetTimerEvent(client_id, client_id, EVENT_PLAYER_HILL, 5));
			break;
		}
		case OP_TRACKING_PLAYER: {
			if (clients[client_id]->type < 3) {
				lua_State* L = clients[client_id]->L;
				clients[client_id]->lua_lock.lock();
				lua_getglobal(L, "event_agro_fallow");
				lua_pushnumber(L, clients[client_id]->target_id);
				lua_pcall(L, 1, 0, 0);
				clients[client_id]->lua_lock.unlock();
			}
			//cout << "tracking들어오기는함\n";
			
			delete exp_over;
			break;
		
		}
		case OP_NPC_ATTACK: {
			if (exp_over->_target < MAX_USER&&clients[client_id]->is_active==true&&
				clients[client_id]->type!=4)
			{
				
				attack(client_id, exp_over->_target);
				if (true == clients[client_id]->is_recognize)
					timer_queue.push(SetTimerEvent(client_id, exp_over->_target,
						EVENT_NPC_ATTACK, 1));
			}
			delete exp_over;
			break;
		}
		case OP_REGEN:
		{
			
			clients[client_id]->hp = clients[client_id]->maxhp;
			clients[client_id]->x = clients[client_id]->origin_x;
			clients[client_id]->y = clients[client_id]->origin_y;
			clients[client_id]->is_active = true;
			clients[client_id]->is_recognize = false;
			
			delete exp_over;
			break;
		}
		}
	}
}
void Initialize_NPC()
{
	int x, y;
	short level,dam;
	for (int i = NPC_ID_START; i <= NPC_ID_END; ++i) {// type 0(pl),1(ke),2(wo),3(sl),4(ch)
		if (i <= NPC_KEROB_END)
		{
			
			x = (rand() % (WORLD_WIDTH/2))+((WORLD_WIDTH/2)-1);
			y = rand() % WORLD_HEIGHT;
			level = 12;
			
			clients[i] = new NPC(i, 1, x, y,level*10, level,level*2, level*10, "Cerberus");
			
			
		}
		else if (i > NPC_KEROB_END && i <= NPC_WOLF_END)
		{
			x = rand() % WORLD_WIDTH; 
			y = (rand() % (WORLD_HEIGHT/2)) + ((WORLD_HEIGHT/2) - 1);
			level = 10;
			clients[i] = new NPC(i, 2, x, y,level*10, level, level * 2, level * 10, "WereWolf");
		}
		else if (i > NPC_WOLF_END && i <= NPC_SLIME_END)
		{
			x = rand() % WORLD_WIDTH;
			y = rand() % (WORLD_HEIGHT/2);
			level = rand() % 7 + 2;
			clients[i] = new NPC(i, 3, x, y, level * 10,level, level * 2, level * 10, "SLIME");
		}
		else if (i > NPC_SLIME_END && i <= NPC_CHICKEN_END)
		{
			x = rand() % (WORLD_WIDTH/2);
			y = rand() % WORLD_HEIGHT;
			level = rand() % 5 + 1;
			clients[i] = new NPC(i, 4, x, y, level * 10, level, level * 2, level * 10,"CHICKEN");
		}
		//cout << "type : " << clients[i]->type << "dam:" << clients[i]->damage << endl;
		lua_State* L=clients[i]->L = luaL_newstate();
		luaL_openlibs(L);
		int error = luaL_loadfile(L, "monster.lua") ||
			lua_pcall(L, 0, 0, 0);
		lua_getglobal(L, "initializMonster");
		lua_pushnumber(L, clients[i]->id);
		lua_pushnumber(L, clients[i]->x);
		lua_pushnumber(L, clients[i]->y);					  
		lua_pushnumber(L, clients[i]->level);	  
		lua_pushnumber(L, clients[i]->maxhp);					  
		lua_pushnumber(L, clients[i]->exp);					 
		lua_pushnumber(L, clients[i]->damage);
		lua_pushnumber(L, clients[i]->type);
		lua_pcall(L, 8, 0, 0);
		//lua_pop(L, 8);
		lua_register(L, "API_recognize_player", API_recognize_player);
		lua_register(L, "API_get_x", API_get_x);
		lua_register(L, "API_get_y", API_get_y);
		//lua_register(L, "API_regen", API_regen);
		
	}
}
void do_npc_move(int npc_id)
{
	if (false == clients[npc_id]->is_active)
		return;
	unordered_set <int> old_viewlist;
	unordered_set <int> new_viewlist;

	for (auto obj : clients) {
		if (obj->state != ST_INGAME)
			continue;
		if (false == is_player(obj->id))
			break;
		if (true == is_near(npc_id, obj->id))
			old_viewlist.insert(obj->id);
	}
	auto& x = clients[npc_id]->x;
	auto& y = clients[npc_id]->y;
	type_move(npc_id);
	for (auto obj : clients) {
		if (obj->state != ST_INGAME)
			continue;
		if (false == is_player(obj->id))
			break;
		if (true == is_near(npc_id, obj->id))
		{
			new_viewlist.insert(obj->id);
			timer_queue.push(SetTimerEvent(npc_id, npc_id, EVENT_NPC_MOVE, 1));
		}
	}
	// 새로 시야에 들어온 플레이어
	for (auto pl : new_viewlist) {
		if (clients[npc_id]->is_active == false)continue;
		if (0 == old_viewlist.count(pl)) {
			
			((Player*)clients[pl])->m_vl.lock();
			((Player*)clients[pl])->m_viewlist.insert(npc_id);
			((Player*)clients[pl])->m_vl.unlock();
			((Player*)clients[pl])->send_put_object(npc_id);
		}
		else {
			((Player*)clients[pl])->send_move_packet(npc_id);
		}
	}
	// 시야에서 사라지는 경우
	for (auto pl : old_viewlist) {
		if (0 == new_viewlist.count(pl)) {
			((Player*)clients[pl])->m_vl.lock();
			((Player*)clients[pl])->m_viewlist.erase(npc_id);
			((Player*)clients[pl])->m_vl.unlock();
			((Player*)clients[pl])->send_remove_object(npc_id);
		}
	}

}

timer_event SetTimerEvent(int obj_id, int target_id, EVENT_TYPE ev,int seconds)
{
	timer_event t;
	t.obj_id = obj_id;
	t.target_id = target_id;
	t.ev = ev;
	t.start_time=chrono::system_clock::now() + (1s*seconds);
	return t;
}

int API_get_x(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int x = clients[user_id]->x;
	lua_pushnumber(L, x);
	return 1;
}

int API_get_y(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int y = clients[user_id]->y;
	lua_pushnumber(L, y);
	return 1;
}

int API_recognize_player(lua_State* L)
{
	int npc_id= (int)lua_tointeger(L, -2);
	int user_id = (int)lua_tointeger(L, -1);
	
	lua_pop(L, 3);
	if (clients[npc_id]->is_recognize == true)return 1;
	
	
	clients[npc_id]->is_recognize = true;
	clients[npc_id]->target_id = user_id;
	if(clients[npc_id]->type<3)
	timer_queue.push(SetTimerEvent(npc_id, user_id, EVENT_NPC_ATTACK, 1));
	return 1;
	
}

void attack(int client_id, int target_id)
{
	//if (false==clients[client_id]->is_active)
	//	return;

	if (clients[client_id]->x == clients[target_id]->x && abs(clients[client_id]->y - clients[target_id]->y) <= 1 ||
		clients[client_id]->y == clients[target_id]->y && abs(clients[client_id]->x - clients[target_id]->x) <= 1)
	{
		clients[target_id]->hp -= clients[client_id]->damage;
		
		
		cout << client_id << "가 " << target_id << "를 공격-> " << clients[target_id]->damage << "의 데미지를 입힘" << endl;
		if (clients[target_id]->type == 3&& clients[target_id]->is_recognize==false) {//슬라임
			clients[target_id]->is_recognize = true;
			clients[target_id]->target_id = client_id;
			timer_queue.push(SetTimerEvent(target_id, client_id, EVENT_NPC_ATTACK, 1));
		}
			
			if (target_id < MAX_USER) {
				char mess[256];
				sprintf_s(mess, "monster %s attack-> player :%s hp -%d", clients[client_id]->name, clients[target_id]->name, clients[client_id]->damage);
				((Player*)clients[target_id])->send_chat_packet(mess);
				((Player*)clients[target_id])->send_status_change_packet();
			}
			else
			{
				char mess[256];
				sprintf_s(mess, "player %s attack-> monster :%s hp -%d", clients[client_id]->name, clients[target_id]->name, clients[client_id]->damage);
				((Player*)clients[client_id])->send_chat_packet(mess);
			}
		if (clients[target_id]->hp <= 0 && target_id < MAX_USER)
		{
			Player* target = (Player*)clients[target_id];
			char mess[256];
			sprintf_s(mess, "player %s died", clients[target_id]->name);
			target->send_chat_packet(mess);
			
			target->m_vl.lock();
			auto& old_viewlist = target->m_viewlist;
			target->m_vl.unlock();

			target->hp = target->maxhp;
			target->exp = target->exp / 2;
			target->x = 10;
			target->y = 10;
			//다른곳에서 is_recognize초기화
			unordered_set<int>new_viewlist;
			for (int i = 0; i < MAX_USER; ++i)
			{
				if (target_id == i)continue;
				clients[i]->state_lock.lock();
				if (clients[i]->state != ST_INGAME)
				{
					clients[i]->state_lock.unlock();
					continue;
				}
				clients[i]->state_lock.unlock();
				if (false == is_near(i, target_id))continue;
				new_viewlist.insert(i);
			}
			for (int i = NPC_ID_START; i <=NPC_ID_END; ++i)
			{
				if (false == is_near(i, target_id)) continue;
				new_viewlist.insert(i);
			}
			target->send_move_packet(target->id);
			target->send_status_change_packet();
			Player* other = NULL;
			for (auto pl : old_viewlist)
			{
				if (0 == new_viewlist.count(pl))continue;
				if (true == is_npc(pl))continue;
				other = (Player*)clients[pl];
				other->m_vl.lock();
				if (0 < other->m_viewlist.count(target_id))
				{
					other->m_vl.unlock();
					other->send_move_packet(target_id);
				}
				else
				{
					other->m_viewlist.insert(target_id);
					other->m_vl.unlock();
					other->send_put_object(target_id);
				}

				for (auto& pl : new_viewlist)
				{
					if (0 < old_viewlist.count(pl))continue;
					target->m_vl.lock();
					target->m_viewlist.insert(pl);
					target->m_vl.unlock();
					target->send_put_object(pl);
					
					if (true == is_npc(pl))continue;
					other = (Player*)clients[pl];
					other->m_vl.lock();
					if (0 == other->m_viewlist.count(target_id))
					{
						other->m_viewlist.insert(target_id);
						other->m_vl.unlock();
						other->send_put_object(target_id);
					}
					else
					{
						other->m_vl.unlock();
						other->send_move_packet(target_id);
					}
					
				}

				for (auto pl : old_viewlist)
				{
					
					if( 0< new_viewlist.count(pl))continue;
					target->m_vl.lock();
					target->m_viewlist.erase(pl);
					target->m_vl.unlock();
					target->send_remove_object(pl);
					if (true == is_npc(pl))continue;
					other = (Player*)clients[pl];
					other->m_vl.lock();
					if (0 == other->m_viewlist.count(pl))
					{
						other->m_viewlist.erase(target_id);
						other->m_vl.unlock();
						other->send_remove_object(target_id);
					}
					else
						other->m_vl.unlock();

				}

				for (auto npc : new_viewlist)
				{
					if (false == is_npc(npc))continue;

					lua_State* L = clients[npc]->L;
					clients[npc]->lua_lock.lock();
					if (clients[npc]->type < 3) {
						lua_getglobal(L, "event_agro_fallow");
						lua_pushnumber(L, target_id);
						lua_pcall(L, 1, 0, 0);
						clients[npc]->lua_lock.unlock();
					}
					else
						clients[npc]->lua_lock.unlock();
					
				}
			}
			
		
		}
		else if(target_id>MAX_USER&& clients[target_id]->hp<=0)
		{
			int exp = clients[target_id]->exp;
			int type = clients[target_id]->type;
			int level = clients[client_id]->level;
			if (type < 3) { exp = exp * 2; }
			clients[client_id]->exp += exp;
			char mess[256];
			strcpy_s(mess, "player level up!");

			if ( level== 1)
			{
				if (clients[client_id]->exp >= 100)
				{
					clients[client_id]->exp -= 100;
					clients[client_id]->level++;
				}
				((Player*)clients[client_id])->send_chat_packet(mess);

			}
			else if (level == 2)
			{
				if (clients[client_id]->exp >= 200)
				{
					clients[client_id]->exp -= 200;
					clients[client_id]->level++;
				}
				((Player*)clients[client_id])->send_chat_packet(mess);

			}
			else {
				if (clients[client_id]->exp >= ((int)pow(2.0, (float)(level - 2))) * 200)
				{
					clients[client_id]->exp -= ((int)pow(2.0, (float)(level - 2))) * 200;
					clients[client_id]->level++;
					((Player*)clients[client_id])->send_chat_packet(mess);
				}
			}
			((Player*)clients[client_id])->send_status_change_packet();
			
			Player* other = NULL;
			for (int i = 0; i < MAX_USER; ++i)
			{
				other = (Player*)clients[i];
				other->state_lock.lock();
				if (other->state != ST_INGAME)
				{
					other->state_lock.unlock();
					continue;
				}
				other->state_lock.unlock();

				other->m_vl.lock();
				if (0 < other->m_viewlist.count(target_id))
				{
					other->m_viewlist.erase(target_id);
					other->m_vl.unlock();
					other->send_remove_object(target_id);
				}
				else
					other->m_vl.unlock();
			
				
			}
			clients[target_id]->is_active= false;
			//메세지 넣어주자
			timer_queue.push(SetTimerEvent(target_id, target_id, EVENT_REGEN, 30));
		}
		//이벤트 호출
	}
}

int API_regen(lua_State* L)
{
	int npc_id = (int)lua_tointeger(L, -4);
	short x = (int)lua_tointeger(L, -3);
	short y = (int)lua_tointeger(L, -2);
	short hp = (int)lua_tointeger(L, -1);
	lua_pop(L, 5);
	clients[npc_id]->x = x;
	clients[npc_id]->y = y;
	clients[npc_id]->hp = hp;
	clients[npc_id]->is_active = true;
	clients[npc_id]->is_recognize = false;
	return 1;

}
void type_move(int npc_id)
{
	auto& x = clients[npc_id]->x;
	auto& y = clients[npc_id]->y;
	switch (clients[npc_id]->type) {
	case 1: {
		if (true == clients[npc_id]->is_recognize) {
			if (is_area(npc_id) == true) {
				if (clients[clients[npc_id]->target_id]->x - x != 0)
				{
					x += (clients[clients[npc_id]->target_id]->x - x) / abs(clients[clients[npc_id]->target_id]->x - x);
				}
				else if (clients[clients[npc_id]->target_id]->y - y != 0)
				{
					y += (clients[clients[npc_id]->target_id]->y - y) / abs(clients[clients[npc_id]->target_id]->y - y);
				}
			}
			
			
		}
		else
		{
			if (clients[npc_id]->origin_x - x != 0)
			{
				x += (clients[npc_id]->origin_x - x) / abs(clients[npc_id]->origin_x - x);
			}
			else if (clients[npc_id]->origin_y - y != 0)
			{
				y += (clients[npc_id]->origin_y - y) / abs(clients[npc_id]->origin_y - y);
			}
		}
		break;
	}
	case 2: {
		if (true == is_area(npc_id))
		{
			if (false == clients[npc_id]->is_recognize) {

				switch (rand() % 4) {
				case 0: if (y > 0) y--; break;
				case 1: if (y < (WORLD_HEIGHT - 1)) y++; break;
				case 2: if (x > 0) x--; break;
				case 3: if (x < (WORLD_WIDTH - 1)) x++; break;
				}

			}
			else
			{
				if (clients[clients[npc_id]->target_id]->x - x != 0)
				{
					x += (clients[clients[npc_id]->target_id]->x - x) / abs(clients[clients[npc_id]->target_id]->x - x);
				}
				else if (clients[clients[npc_id]->target_id]->y - y != 0)
				{
					y += (clients[clients[npc_id]->target_id]->y - y) / abs(clients[clients[npc_id]->target_id]->y - y);
				}

			}
		}
		else
		{
			if (clients[npc_id]->origin_x - x != 0)
			{
				x += (clients[npc_id]->origin_x - x) / abs(clients[npc_id]->origin_x - x);
			}
			else if (clients[npc_id]->origin_y - y != 0)
			{
				y += (clients[npc_id]->origin_y - y) / abs(clients[npc_id]->origin_y - y);
			}
		}
		break;
	}
	case 4: {
		if (true == is_area(npc_id))
		{
			switch (rand() % 4) {
			case 0: if (y > 0) y--; break;
			case 1: if (y < (WORLD_HEIGHT - 1)) y++; break;
			case 2: if (x > 0) x--; break;
			case 3: if (x < (WORLD_WIDTH - 1)) x++; break;
			}
		}
		else
		{
			if (clients[npc_id]->origin_x - x != 0)
			{
				x += (clients[npc_id]->origin_x - x) / abs(clients[npc_id]->origin_x - x);
			}
			else if (clients[npc_id]->origin_y - y != 0)
			{
				y += (clients[npc_id]->origin_y - y) / abs(clients[npc_id]->origin_y - y);
			}
		}
		break;
	}
	default:break;
	
	}
	}
