#pragma once
#include "NPC.h"
#include"EXP_OVER.h"
class Player :
    public NPC
{
public:
    unordered_set<int>m_viewlist;
    mutex m_vl;

    
    

    EXP_OVER m_recv_over;
	SOCKET  m_socket;
	int		m_prev_size;
	int		m_last_move_time;

	Player() { cout << "잘못된 생성" << endl; }
	Player(int c_id) : m_prev_size(0)
	{
		x = 0;
		y = 0;
	}

	~Player()
	{
		closesocket(m_socket);
	}

	void do_recv();
	void do_send(int num_bytes,void*mess);
	void send_login_ok_packet();
	void send_move_packet( int mover);//이거 정의 안해놔서 오류뜸
	void send_remove_object(int victim);
	void send_put_object(int target);
	void send_chat_packet(int user_id, char* mess);
	void send_login_fail(int reason);
	void send_status_change_packet();
};

