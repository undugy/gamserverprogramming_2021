#include "Player.h"

extern void error_display(int);
extern array<NPC*, MAX_USER + MAX_NPC>clients;
void Player::do_recv()
{
	DWORD recv_flag = 0;
	ZeroMemory(&m_recv_over._wsa_over, sizeof(m_recv_over._wsa_over));
	m_recv_over._wsa_buf.buf = reinterpret_cast<char*>(m_recv_over._net_buf + m_prev_size);
	m_recv_over._wsa_buf.len = sizeof(m_recv_over._net_buf) - m_prev_size;
	int ret = WSARecv(m_socket, &m_recv_over._wsa_buf, 1, 0, &recv_flag, &m_recv_over._wsa_over, NULL);
	if (SOCKET_ERROR == ret) {
		int error_num = WSAGetLastError();
		if (ERROR_IO_PENDING != error_num)
			error_display(error_num);
	}
}

void Player::do_send(int num_bytes, void* mess)
{

	EXP_OVER* ex_over = new EXP_OVER(OP_SEND, num_bytes, mess);
	int ret = WSASend(m_socket, &ex_over->_wsa_buf, 1, 0, 0, &ex_over->_wsa_over, NULL);
	if (SOCKET_ERROR == ret) {
		int error_num = WSAGetLastError();
		if (ERROR_IO_PENDING != error_num)
			error_display(error_num);
	}
}
void Player::send_login_ok_packet()
{
	sc_packet_login_ok packet;
	
	packet.id = id;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_LOGIN_OK;
	packet.x = x;
	packet.y = y;
	packet.exp =exp;
	packet.hp = hp;
	packet.maxhp = maxhp;
	packet.level = level;
	do_send(sizeof(packet), &packet);
}
void Player::send_remove_object(int victim)
{
	sc_packet_remove_object packet;
	packet.id = victim;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_REMOVE_OBJECT;
	do_send(sizeof(packet), &packet);
}
void Player::send_chat_packet(int user_id, char* mess)
{
	sc_packet_chat packet;
	packet.id = user_id;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_CHAT;
	strcpy_s(packet.message, mess);
	do_send(sizeof(packet), &packet);
}
void Player::send_login_fail(int reason)
{
	sc_packet_login_fail packet;
	packet.type = SC_PACKET_LOGIN_FAIL;
	packet.size = sizeof(packet);
	packet.reason = reason;
	do_send(sizeof(packet), &packet);
}
void Player::send_status_change_packet()
{
	sc_packet_status_change packet;
	packet.id = id;
	packet.type = SC_PACKET_STATUS_CHANGE;
	packet.level =level;
	packet.exp = exp;
	packet.hp = hp;
	packet.maxhp = maxhp;
	packet.size = sizeof(packet);
	do_send(sizeof(packet), &packet);
}
void Player::player_hill()
{
	hp += (maxhp / 10);
	if (hp > maxhp)
		hp = maxhp;
	send_status_change_packet();


}

void Player::send_move_packet(int mover)
{
	sc_packet_move packet;
	Player* p = (Player*)clients[mover];
	packet.id = mover;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_MOVE;
	packet.x = p->x;
	packet.y = p->y;
	packet.move_time = p->m_last_move_time;
	do_send(sizeof(packet), &packet);
}
void Player::send_put_object(int target)
{
	sc_packet_put_object packet;
	Player* p = (Player*)clients[target];
	packet.id = target;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_PUT_OBJECT;
	packet.x = p->x;
	packet.y = p->y;
	strcpy_s(packet.name, p->name);
	packet.object_type = 0;
	do_send(sizeof(packet), &packet);
}