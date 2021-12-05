
#include"header.h"
#include"Player.h"
HANDLE g_h_iocp;
SOCKET g_s_socket;
SQLHENV henv;
SQLHDBC hdbc;
concurrency::concurrent_priority_queue <timer_event> timer_queue;
array<NPC*, MAX_USER + MAX_NPC>clients;
//-------에러확인---------------------------------

void error_display(int err_no);//에러 확인용

bool is_near(int a, int b);//시야에 있는지 체크
bool is_npc(int id);//npc인지 체크
bool is_player(int id);//player 인지 체크
int get_new_id();//새로운 아이디 할당(로그인 아이디 아님)

//--------------------------------------------

//---------패킷전송---------------------


void do_timer();
void worker();
//--------------------------------------

void Disconnect(int c_id);
void process_packet(int client_id, unsigned char* p);
void do_timer();
void Initialize_NPC();
int main()
{
	wcout.imbue(locale("korean"));
	WSADATA WSAData;
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

	for (int i = 0; i < MAX_USER; ++i)
		clients[i] = new Player(i);

	cout << "Creating Worker Threads\n";

	Initialize_NPC();

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
		if (ST_INGAME == cl->state)
			Disconnect(cl->id);
	}
	closesocket(g_s_socket);
	WSACleanup();
	return 0;
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
		p = ((Player*)(clients[i]));
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
		
		for(int i=0; i< MAX_USER; ++i)
		strcpy_s(cl->name, packet->name);
		cl->send_login_ok_packet();
		Player* other_cl = NULL;
		
		cl->state_lock.lock();
		cl->state = ST_INGAME;
		cl->state_lock.unlock();
		
		// 새로 접속한 플레이어의 정보를 주위 플레이어에게 보낸다
		for (auto other : clients) {
			if (true == is_npc(other->id)) continue;
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
			if (ST_INGAME != ((Player*)other)->state)
				continue;
			if (false == is_near(client_id, other->id))
				continue;
			//if (true == is_npc(other._id)) {
			//	Activate_Player_Move_Event(other._id, cl._id);
			//}
			nearlist.insert(other->id);
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

				if (true == is_npc(other)) continue;
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
	}
	}
}
void do_timer() {

	while (true) {
		while (true) {
			timer_event ev;
			if (!timer_queue.try_pop(ev))break;
			auto start_t = chrono::system_clock::now();
			if (ev.start_time <= start_t) {
				EXP_OVER* ex_over = new EXP_OVER;
				ex_over->_comp_op = OP_NPC_MOVE;
				PostQueuedCompletionStatus(g_h_iocp, 1, ev.obj_id, &ex_over->_wsa_over);
			}
			else {//ev.start_time > start_t
				//timer_queue.push(ev);	// timer_queue에 넣지 않고 최적화 필요// 1457명
				this_thread::sleep_for(ev.start_time - start_t);
				EXP_OVER* ex_over = new EXP_OVER;
				ex_over->_comp_op = OP_NPC_MOVE;
				PostQueuedCompletionStatus(g_h_iocp, 1, ev.obj_id, &ex_over->_wsa_over);
				//break;
			}
		}
		//this_thread::sleep_for(10ms);
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
			//save_pos(client_id);
			Disconnect(client_id);
			if (exp_over->_comp_op == OP_SEND)
				delete exp_over;
			continue;
		}

		switch (exp_over->_comp_op) {
		case OP_RECV: {
			if (num_byte == 0) {
				//save_pos(client_id);
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
				//save_pos(client_id);
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
				cl->x = rand() % WORLD_WIDTH;
				cl->y = rand() % WORLD_HEIGHT;
				cl->id = new_id;
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
			//do_npc_move(client_id);
			break;
		}
		}
	}
}
void Initialize_NPC()
{
	for (int i = NPC_ID_START; i <= NPC_ID_END; ++i) {
		clients[i] = new NPC(i, 1, rand() % WORLD_WIDTH, rand() % WORLD_HEIGHT);

		lua_State* L = clients[i]->L = luaL_newstate();
		luaL_openlibs(L);
		int error = luaL_loadfile(L, "monster.lua") ||
			lua_pcall(L, 0, 0, 0);
		lua_getglobal(L, "set_uid");
		lua_pushnumber(L, i);
		lua_pcall(L, 1, 1, 0);
		lua_pop(L, 1);// eliminate set_uid from stack after call

		/*lua_register(L, "API_SendMessage", API_SendMessage);
		lua_register(L, "API_get_x", API_get_x);
		lua_register(L, "API_get_y", API_get_y);*/
	}
}