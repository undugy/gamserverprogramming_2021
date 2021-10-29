#include <windows.h>

class GameObject {
public:
	virtual void update(); 
	virtual void draw();
};

//게임에서 사용하는 비트맵이미지를 관리하는 클래스
class GameBitmap {

public :
	HBITMAP load(); // 비트맵이미지 로드함수
	GameBitmap(); // 
	void draw();

	int getHeight();
	int getWidth();
protected:
	RECT src_rect; 
	RECT dst_rect; 
	int width;
	int height;
	
};

class Player : GameObject{
	char id;
	char type; //시민,임포스터 종류
	int x,y; // x,y 캐릭터의 중심좌표
	int spd; // 속도 
	RECT collision_rect; // 충돌 체크를 위한 사각형
	GameBitmap bitmap; // 현재 그리고 있는 이미지
public:
	Player();

	char getId();
	char getType();
	int getX();
	int getY();
	int getSpd();

	void setId();
	void setType();
	void setX();
	void setY();

	//오브젝트 상호작용,시민죽이기,신고등 모든 상호작용 관련키
	void action(); 

	void update() override;
	void draw() override;
};

class Mission : GameObject {
	char type; //미션 종류
	GameBitmap bitmap[5]; // 임무내용이 보이는 이미지

public:
	Mission();

	char getType();

	void setType();

	void update() override;
	void draw() override;
};

class MObj : GameObject {
	RECT collision_rect; // 충돌 체크를 위한 사각형, 이범위안에서 키를눌러야 미션을 시작함
	Mission mission;
	RECT src_rect;
	RECT dst_rect;
	GameBitmap bitmap; // 맵에 보이는 오브젝트 이미지
	
public:

	void update() override;
	void draw() override;
};

class Background : GameObject {
	RECT src_rect;
	RECT dst_rect;
	GameBitmap bitmap;

public:
	Background();

	void update() override;
	void draw() override;
};

class UI : GameObject {
	char progress;
	
	GameBitmap bitmap[5];
public:
	UI();

	char getProgress();

	void setProgress();

	void update() override;
	void draw() override;
};