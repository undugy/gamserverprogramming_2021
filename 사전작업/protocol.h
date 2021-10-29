
const char SERVERPORT = 49500;

const char NAME_SIZE = 12;




// �������� ����
// Ÿ��
const char CS_PACKET_LOGIN = 1;
const char CS_PACKET_ENTER_LOBBY = 2;
const char CS_PACKEt_READY = 3;
const char CS_PACKET_MOVE = 4;
const char CS_PACKET_MISSION = 5;
const char CS_PACKET_ATTACK = 6;
const char CS_PACKET_LOOK_DEAD = 7;
const char CS_PACKET_DISCUSS_MSG = 8;
const char CS_PACKET_SELECT_VOTE = 9;




const char SC_PACKET_LOGIN_OK = 1;
const char SC_PACKET_LOBBY_INFO = 2;
const char SC_PACKET_READY = 3;
const char SC_PACKET_START_GAME = 4;
const char SC_PACKET_PLAYER_JOB = 5;
const char SC_PACKET_MOVE = 6;
const char SC_PACKET_MISSION_PROGRESS = 7;

const char SC_PACKET_DEAD = 8;

const char SC_PACKET_START_VOTE = 9;

const char SC_PACKET_GAME_RESULT = 10;
const char SC_PACKET_DISCUSS_MSG = 11;
const char SC_PACKET_START_VOTE = 12;
const char SC_PACKET_COMPLETE_VOTE = 13;
const char SC_PACKET_SAME_VOTE = 14;
// Packet(Application) ����
#pragma pack(push, 1)
//���̵� ��Ŷ
struct cs_packet_login {
	char size;
	char type;
	char name[NAME_SIZE];
};
//�κ� �����ߴٰ� �˷��ֱ�
struct cs_packet_enter_lobby {
	char size;
	char type;
	char id;
};
//�غ��ư Ŭ���� ����
struct cs_packet_ready {
	char size;
	char type;
	char id;
};
//Ŭ�󿡼� ��ǲ ������
struct cs_packet_move {
	unsigned char size;
	char	type;
	char	direction;			// 0 : up,  1: down, 2:left, 3:right
};
// Ŭ�� �ӹ��� �����ϸ� ������
struct cs_packet_mission {
	char size;
	char type;
};
//������ �� �÷��̾� ����
struct cs_packet_attack {
	char size;
	char type;
	char id;
};
//ũ����� ��ü�� �߰����� ��
struct cs_packet_look_dead {
	char size;
	char type;
};
//����ä��
// ���� ��� ������ ��� 
//1.a�� 1.������ 1.���� 1.�ô�
//2.b�� 2.������ 2.������ �ൿ 2.�ߴ�
struct cs_packet_discuss_msg {
	char size;
	char type;
	char id;
	char W3H1[4];
};
//��ǥ�� ����� ����.
struct cs_packet_select_vote {
	char size;
	char type;
	char my_id;
	char select_id;
};



//�α��� ������ ����
struct sc_packet_login_ok {
	char size;
	char type;
	char id;
};

//Ŭ�� ���ӽ� �ٸ� Ŭ����� ������ ������ ����
struct sc_packet_lobby_info {
	char size;
	char type;
	char id;
	char name[NAME_SIZE];
	char ready;
};

//�������� ������� ����
struct sc_packet_ready {
	char size;
	char type;
	char id;
};

//���� �غ�� ���ӽ����� �˸�
struct sc_packet_start_game {
	char size;
	char type;
};




//------------------- �ΰ���---------------------
//ũ��� ���� ������������
struct sc_packet_player_job {
	char size;
	char type;
	char job;
};
//������Ʈ�� ��ġ ����
struct sc_packet_move {
	char size;
	char type;
	char id;
	int x, y;
};

//�ӹ� �޼��� %
struct sc_packet_mission_progress {
	char size;
	char type;
	char progress;
};



//�������� �������Ͱ��ƴϰ�, ��Ÿ����̰�, �����ѻ���� ���尡��� ��� ���̴� �ɷ�
//�÷��̾ �׾��� ��
struct sc_packet_dead {
	char size;
	char type;
	char id;
};



//��ǥ�� �����ϱ����� ��� Ŭ���̾�Ʈ���� �˸��� ���� ���
struct sc_packet_start_vote {
	char size;
	char type;
};

//�̱� ������ ���ؼ� �����ش�. ex) ũ����� �̱�� ũ��� ����
struct sc_packet_game_result {
	char size;
	char type;
	char job;
};
//------------------------------------------------------


//--------------------��ǥ ���� ��--------------------


//��� Ŭ�󿡰� �޼����� ������.
struct sc_packet_discuss_msg {
	char size;
	char type;
	char id;
	char W3H1[4];
};

//���� ��ǥ�� ���´��� ��ǥ���� ��Ȳ�� �����ش�.
struct sc_packet_state_vote {
	char size;
	char type;
	char id;
};



//��ü ��ǥ�� �Ϸ�Ǹ� ���õ� ����� ���̵� �����ش�.
struct sc_packet_complete_vote {
	char size;
	char type;
	char complete_id;
};

//��ǥ����� ������ ��� �����ش�
struct sc_packet_same_vote {
	char size;
	char type;
};
//---------------------------------------------------------


#pragma pack(pop)