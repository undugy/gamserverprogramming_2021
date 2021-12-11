#define SFML_STATIC 1

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <chrono>
#include<vector>
using namespace std;

#ifdef _DEBUG
#pragma comment (lib, "lib/sfml-graphics-s-d.lib")
#pragma comment (lib, "lib/sfml-window-s-d.lib")
#pragma comment (lib, "lib/sfml-system-s-d.lib")
#pragma comment (lib, "lib/sfml-network-s-d.lib")
#else
#pragma comment (lib, "lib/sfml-graphics-s.lib")
#pragma comment (lib, "lib/sfml-window-s.lib")
#pragma comment (lib, "lib/sfml-system-s.lib")
#pragma comment (lib, "lib/sfml-network-s.lib")
#endif
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "ws2_32.lib")

#include"/Users/undug/Documents/GitHub/gamserverprogramming_2021/Term_project_server/Term_project_server/2021_가을_protocol.h"
//#include "../Term_project_server/Term_project_server\2021_가을_protocol.h"

sf::TcpSocket socket;

constexpr auto BUF_SIZE = 256;
constexpr auto SCREEN_WIDTH = 20;
constexpr auto SCREEN_HEIGHT = 20;


constexpr auto TILE_WIDTH = 65;
constexpr auto WINDOW_WIDTH = TILE_WIDTH * SCREEN_WIDTH + 10;   // size of window
constexpr auto WINDOW_HEIGHT = TILE_WIDTH * SCREEN_WIDTH + 10;
//constexpr auto BUF_SIZE = MAX_BUFFER;

int g_myid;
int g_x_origin;
int g_y_origin;
vector<string>m_text_log;
sf::RenderWindow* g_window;
sf::Font g_font;
class TextField : public sf::Transformable {
private:
	unsigned int m_size;
	sf::Font m_font;
	std::wstring m_text;
	sf::RectangleShape log_rect;
	bool m_hasfocus;
public:
	TextField(unsigned int maxChars) : m_size(maxChars),
		
		m_hasfocus(false),
		log_rect(sf::Vector2f(400, 200))
	{
		m_font.loadFromFile("C:/Windows/Fonts/Arial.ttf"); // I'm working on Windows, you can put your own font instead
		

		log_rect.setFillColor(sf::Color(0, 0, 0, 100));
		log_rect.setPosition(this->getPosition().x, this->getPosition().y - 180);

	}

	void setPosition(float x, float y) {
		sf::Transformable::setPosition(x, y);
		//m_rect.setPosition(x, y);
		log_rect.setPosition(x, y - 180);
	}



	void draw() {
		sf::Text temp_text;
		temp_text.setFont(m_font);
		temp_text.setString(m_text);
		temp_text.setPosition(this->getPosition());
		temp_text.scale(0.5f, 0.5f);
		temp_text.setFillColor(sf::Color::White);

		g_window->draw(log_rect);
		//g_window->draw(m_rect);
		g_window->draw(temp_text);


		for (int i = 0; i < m_text_log.size(); i++) {
			string m_string = m_text_log[i];
			wstring w_string;
			w_string.assign(m_string.begin(), m_string.end());
			temp_text.setString(w_string);
			temp_text.setPosition(this->getPosition().x, this->getPosition().y - 15 * (11 - i) - 10);
			g_window->draw(temp_text);
		}
	}

	

	bool getFocus() {
		return m_hasfocus;
	}


	~TextField() {}
};

class OBJECT {
private:
	bool m_showing;
	sf::Sprite m_sprite;
	sf::Text m_name;
	sf::Text m_chat;
	sf::Vector2f m_text_tile_size;
	sf::Vector2i m_text_pos;
	sf::Vector2i m_log_size;
	
	chrono::system_clock::time_point m_mess_end_time;
public:
	int id;
	int m_x, m_y;
	short  m_hp, m_maxhp;
	short  m_level;
	int	   m_exp;
	char    m_type;
	OBJECT(sf::Texture& t, int x, int y, int x2, int y2) {
		m_showing = false;
		m_sprite.setTexture(t);
		m_sprite.setTextureRect(sf::IntRect(x, y, x2, y2));
		set_name("NONAME");
		m_mess_end_time = chrono::system_clock::now();
		m_text_pos = { 10,5 };
		m_log_size = {20 - 10,5 };
		m_text_tile_size={ 50.f,50.f };
	}
	OBJECT() {
		m_showing = false;
	}
	void show()
	{
		m_showing = true;
	}
	void hide()
	{
		m_showing = false;
	}

	void a_move(int x, int y) {
		m_sprite.setPosition((float)x, (float)y);
	}

	void a_draw() {
		g_window->draw(m_sprite);
	}

	void move(int x, int y) {
		m_x = x;
		m_y = y;
	}
	void draw() {
		if (false == m_showing) return;
		float rx = (m_x - g_x_origin) * 65.0f + 8;
		float ry = (m_y - g_y_origin) * 65.0f + 8;
		m_sprite.setPosition(rx, ry);
		g_window->draw(m_sprite);
		
		if (m_mess_end_time < chrono::system_clock::now()) {
			m_name.setPosition(rx - 10, ry - 20);
			g_window->draw(m_name);
		}
		else {
			
		}
		
	}
	
	void set_name(const char str[]) {
		m_name.setFont(g_font);
		m_name.setString(str);
		m_name.setFillColor(sf::Color(255, 255, 0));
		m_name.setStyle(sf::Text::Bold);
	}
	sf::Text set_chat(const char str[]) {
		m_chat.setFont(g_font);
		m_chat.setString(str);
		m_chat.setFillColor(sf::Color(255, 255, 255));
		m_chat.setStyle(sf::Text::Bold);
		return m_chat;
		//m_mess_end_time = chrono::system_clock::now() + chrono::seconds(3);
	}
	
	void set_status(short level, short hp,short maxhp,int exp){
		m_hp = hp;
		m_maxhp=maxhp;
		m_level=level;
		m_exp= exp;
		
	}
};

OBJECT avatar;
OBJECT players[MAX_USER + MAX_NPC];

OBJECT white_tile;
OBJECT tent_tile;
OBJECT black_tile;
OBJECT rock_tile;
sf::Texture* board;
sf::Texture* pieces;

void client_initialize()
{
	board = new sf::Texture;
	pieces = new sf::Texture;
	if (false == g_font.loadFromFile("cour.ttf")) {
		cout << "Font Loading Error!\n";
		while (true);
	}
	board->loadFromFile("chessmap2.bmp");
	pieces->loadFromFile("chess3.png");
	white_tile = OBJECT{ *board, 5, 5, TILE_WIDTH, TILE_WIDTH };
	black_tile = OBJECT{ *board, 69, 5, TILE_WIDTH, TILE_WIDTH };
	tent_tile = OBJECT{ *board, 5, 69, TILE_WIDTH, TILE_WIDTH };
	rock_tile = OBJECT{ *board, 69, 69, TILE_WIDTH, TILE_WIDTH };
	avatar = OBJECT{ *pieces, 0, 0, 64, 64 };
	for (int i = 0; i <= NPC_ID_END;++i) {
		if(i<MAX_USER)
			players[i] = OBJECT{ *pieces, 256, 0, 64, 64 };
		else if(i>MAX_USER&&i<=NPC_KEROB_END)
			players[i] = OBJECT{ *pieces, 128, 0, 64, 64 };
		else if (i > NPC_KEROB_END && i <= NPC_WOLF_END)
			players[i] = OBJECT{ *pieces, 64, 0, 64, 64 };
		else if (i > NPC_WOLF_END && i <= NPC_SLIME_END)
			players[i] = OBJECT{ *pieces, 192, 0, 64, 64 };
		else if (i > NPC_SLIME_END && i <= NPC_CHICKEN_END)
			players[i] = OBJECT{ *pieces, 320, 0, 64, 64 };
	}
}

void client_finish()
{
	delete board;
	delete pieces;
}

void ProcessPacket(char* ptr)
{
	static bool first_time = true;
	switch (ptr[1])
	{
	case SC_PACKET_LOGIN_OK:
	{
		sc_packet_login_ok* packet = reinterpret_cast<sc_packet_login_ok*>(ptr);
		if (packet->id == -1)
		{
			cout << "로그인 실패" << endl;
			while (true);
			
			
		}
		g_myid = packet->id;
		avatar.m_x = packet->x;
		avatar.m_y = packet->y;
		g_x_origin = packet->x - SCREEN_WIDTH / 2;
		g_y_origin = packet->y - SCREEN_WIDTH / 2;
		avatar.move(packet->x, packet->y);
		avatar.show();
		break;
	}
	case SC_PACKET_LOGIN_FAIL:
	{
		sc_packet_login_fail* packet = reinterpret_cast<sc_packet_login_fail*>(ptr);
		if (packet->reason == 0)cout << "해당 아이디는 접속중입니다." << endl;
		else cout << "최대 수용인원수를 초과하였습니다" << endl;
		g_window->close();
	}
	case SC_PACKET_PUT_OBJECT:
	{
		sc_packet_put_object* my_packet = reinterpret_cast<sc_packet_put_object*>(ptr);
		int id = my_packet->id;

		if (id < MAX_USER) { // PLAYER
			players[id].set_name(my_packet->name);
			players[id].move(my_packet->x, my_packet->y);
			players[id].show();
		}
		else {  // NPC
			players[id].set_name(my_packet->name);
			players[id].move(my_packet->x, my_packet->y);
			players[id].show();
		}
		break;
	}
	case SC_PACKET_MOVE:
	{
		sc_packet_move * my_packet = reinterpret_cast<sc_packet_move *>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			avatar.move(my_packet->x, my_packet->y);
			g_x_origin = my_packet->x - SCREEN_WIDTH / 2;
			g_y_origin = my_packet->y - SCREEN_WIDTH / 2;
		}
		else if (other_id < MAX_USER) {
			players[other_id].move(my_packet->x, my_packet->y);
		}
		else {
			players[other_id].move(my_packet->x, my_packet->y);
		}
		break;
	}

	case SC_PACKET_REMOVE_OBJECT:
	{
		sc_packet_remove_object* my_packet = reinterpret_cast<sc_packet_remove_object*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			avatar.hide();
		}
		else if (other_id < MAX_USER) {
			players[other_id].hide();
		}
		else {
			players[other_id].hide();
		}
		break;
	}

	case SC_PACKET_CHAT:
	{
		sc_packet_chat* my_packet = reinterpret_cast<sc_packet_chat*>(ptr);
		int other_id = my_packet->id;
	
		if (m_text_log.size() < 10)
			m_text_log.push_back(my_packet->message);
		else
		{
			m_text_log.erase(m_text_log.begin());
			m_text_log.push_back(my_packet->message);
		}
		
		break;
	}
	case SC_PACKET_STATUS_CHANGE:
	{
		sc_packet_status_change* packet = reinterpret_cast<sc_packet_status_change*>(ptr);
		int other_id = packet->id;
		if (g_myid == packet->id) {
			avatar.set_status(packet->level, packet->hp, packet->maxhp, packet->exp);
		}
		else if(other_id<MAX_USER)
		{
			players[other_id].set_status(packet->level, packet->hp, packet->maxhp, packet->exp);
		}
		else
		{
			players[other_id].set_status(packet->level, packet->hp, packet->maxhp, packet->exp);
		}
		break;
	}
	default:
		printf("Unknown PACKET type [%d]\n", ptr[1]);
	}
}

void process_data(char* net_buf, size_t io_byte)
{
	char* ptr = net_buf;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (0 != io_byte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (io_byte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			io_byte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
}

bool client_main()
{
	char net_buf[BUF_SIZE];
	size_t	received;
	float hp_progress;
	
	auto recv_result = socket.receive(net_buf, BUF_SIZE, received);
	if (recv_result == sf::Socket::Error)
	{
		wcout << L"Recv 에러!";
		while (true);
	}
	if (recv_result == sf::Socket::Disconnected)
	{
		wcout << L"서버 접속 종료.\n";
		return false;
	}
	if (recv_result != sf::Socket::NotReady)
		if (received > 0) process_data(net_buf, received);

	for (int i = 0; i < SCREEN_WIDTH; ++i)
		for (int j = 0; j < SCREEN_HEIGHT; ++j)
		{
			int tile_x = i + g_x_origin;
			int tile_y = j + g_y_origin;
			if ((tile_x < 0) || (tile_y < 0)) continue;
			if ((((tile_x / 3)  + (tile_y / 3)) % 2) == 1) {
				white_tile.a_move(TILE_WIDTH * i + 7, TILE_WIDTH * j + 7);
				white_tile.a_draw();
			}
			else
			{
				black_tile.a_move(TILE_WIDTH * i + 7, TILE_WIDTH * j + 7);
				black_tile.a_draw();
			}
		}
	avatar.draw();
	for (auto& pl : players) pl.draw();
	sf::Text text;
	sf::Text hp_text;
	text.setFont(g_font);
	hp_text.setFont(g_font);
	char buf[100];
	sprintf_s(buf, "(level : %d , exp : %d)", avatar.m_level, avatar.m_exp);
	text.setString(buf);
	text.setPosition(300, 150);
	text.setFillColor(sf::Color(0,0,255,255));
	hp_text.setString("hp:");
	hp_text.setPosition(10, 80);
	hp_progress = 500 * ((float)avatar.m_hp / (float)avatar.m_maxhp);
	sf::RectangleShape hpbar(sf::Vector2f(hp_progress, 100));
	sf::RectangleShape maxhp_bar(sf::Vector2f(500, 100));
	hpbar.setFillColor(sf::Color(255, 0, 0, 200));
	maxhp_bar.setFillColor(sf::Color(5, 5, 5, 230));
	hpbar.setPosition(60.0f, 30);
	maxhp_bar.setPosition(60.0f, 30);
	g_window->draw(maxhp_bar);
	g_window->draw(hpbar);
	g_window->draw(text);
	g_window->draw(hp_text);
	return true;
}
void send_attack_packet()
{
	cs_packet_attack packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_ATTACK;
	size_t sent=0;
	socket.send(&packet, sizeof(packet), sent);
}
void send_move_packet(char dr)
{
	cs_packet_move packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_MOVE;
	packet.direction = dr;
	size_t sent = 0;
	socket.send(&packet, sizeof(packet), sent);
}

void send_login_packet(string &name)
{
	cs_packet_login packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_LOGIN;
	strcpy_s(packet.name, name.c_str());
	size_t sent = 0;
	socket.send(&packet, sizeof(packet), sent);
}
void send_teleport_packet()
{
	cs_packet_login packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_TELEPORT;
	size_t sent = 0;
	socket.send(&packet, sizeof(packet), sent);
}
chrono::steady_clock::time_point t1 = chrono::high_resolution_clock::now();
int main()
{
	wcout.imbue(locale("korean"));
	string name;
	cout << "id 를 입력하세요 : ";
	cin >> name;
	sf::Socket::Status status = socket.connect("127.0.0.1", SERVER_PORT);

	
	socket.setBlocking(false);

	if (status != sf::Socket::Done) {
		wcout << L"서버와 연결할 수 없습니다.\n";
		while (true);
	}

	client_initialize();

	send_login_packet(name);	
	avatar.set_name(name.c_str());
	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2D CLIENT");
	g_window = &window;
	TextField text_field(20);
	text_field.setPosition(900, 900);
	
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			
			if (event.type == sf::Event::KeyPressed) {
				
				switch (event.key.code) {
				case sf::Keyboard::Left:
					send_move_packet(2);
					break;
				case sf::Keyboard::Right:
					send_move_packet(3);
					break;
				case sf::Keyboard::Up:
					send_move_packet(0);
					break;
				case sf::Keyboard::Down:
					send_move_packet(1);
					break;
				case sf::Keyboard::A:
					send_attack_packet();
					break;
				case sf::Keyboard::B:
					send_teleport_packet();
					break;
				case sf::Keyboard::Escape:
					window.close();
					break;
				}
				
			}
		}

		window.clear();
		if (false == client_main())
			window.close();
		text_field.draw();
		window.display();
	}
	client_finish();

	return 0;
}