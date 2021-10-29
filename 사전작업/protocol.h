
const char SERVERPORT = 49500;

const char NAME_SIZE = 12;




// 프로토콜 설계
// 타입
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
// Packet(Application) 정의
#pragma pack(push, 1)
//아이디 패킷
struct cs_packet_login {
	char size;
	char type;
	char name[NAME_SIZE];
};
//로비에 진입했다고 알려주기
struct cs_packet_enter_lobby {
	char size;
	char type;
	char id;
};
//준비버튼 클릭시 레디
struct cs_packet_ready {
	char size;
	char type;
	char id;
};
//클라에서 인풋 보내기
struct cs_packet_move {
	unsigned char size;
	char	type;
	char	direction;			// 0 : up,  1: down, 2:left, 3:right
};
// 클라가 임무를 수행하면 보내기
struct cs_packet_mission {
	char size;
	char type;
};
//술레일 때 플레이어 공격
struct cs_packet_attack {
	char size;
	char type;
	char id;
};
//크루원이 시체를 발견했을 때
struct cs_packet_look_dead {
	char size;
	char type;
};
//빠른채팅
// 누가 어디서 무엇을 어떻게 
//1.a가 1.연구소 1.살인 1.봤다
//2.b가 2.관측소 2.수상한 행동 2.했다
struct cs_packet_discuss_msg {
	char size;
	char type;
	char id;
	char W3H1[4];
};
//투표할 대상을 고른다.
struct cs_packet_select_vote {
	char size;
	char type;
	char my_id;
	char select_id;
};



//로그인 성공시 전송
struct sc_packet_login_ok {
	char size;
	char type;
	char id;
};

//클라가 접속시 다른 클라들의 정보를 보내기 위해
struct sc_packet_lobby_info {
	char size;
	char type;
	char id;
	char name[NAME_SIZE];
	char ready;
};

//서버에서 레디상태 전송
struct sc_packet_ready {
	char size;
	char type;
	char id;
};

//전부 준비시 게임시작을 알림
struct sc_packet_start_game {
	char size;
	char type;
};




//------------------- 인게임---------------------
//크루원 인지 임포스터인지
struct sc_packet_player_job {
	char size;
	char type;
	char job;
};
//오브젝트의 위치 수신
struct sc_packet_move {
	char size;
	char type;
	char id;
	int x, y;
};

//임무 달성도 %
struct sc_packet_mission_progress {
	char size;
	char type;
	char progress;
};



//서버에서 임포스터가아니고, 사거리안이고, 공격한사람과 가장가까운 사람 죽이는 걸로
//플레이어가 죽었을 때
struct sc_packet_dead {
	char size;
	char type;
	char id;
};



//투표를 시작하기위해 모든 클라이언트에게 알리기 위해 사용
struct sc_packet_start_vote {
	char size;
	char type;
};

//이긴 직업에 대해서 보내준다. ex) 크루원이 이기면 크루원 전송
struct sc_packet_game_result {
	char size;
	char type;
	char job;
};
//------------------------------------------------------


//--------------------투표 시작 후--------------------


//모든 클라에게 메세지를 보낸다.
struct sc_packet_discuss_msg {
	char size;
	char type;
	char id;
	char W3H1[4];
};

//누가 투표를 끝냈는지 투표진행 상황을 보내준다.
struct sc_packet_state_vote {
	char size;
	char type;
	char id;
};



//전체 투표가 완료되면 선택된 사람의 아이디를 보내준다.
struct sc_packet_complete_vote {
	char size;
	char type;
	char complete_id;
};

//투표결과가 동률일 경우 보내준다
struct sc_packet_same_vote {
	char size;
	char type;
};
//---------------------------------------------------------


#pragma pack(pop)