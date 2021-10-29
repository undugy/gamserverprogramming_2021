#include <windows.h>

class GameObject {
public:
	virtual void update(); 
	virtual void draw();
};

//���ӿ��� ����ϴ� ��Ʈ���̹����� �����ϴ� Ŭ����
class GameBitmap {

public :
	HBITMAP load(); // ��Ʈ���̹��� �ε��Լ�
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
	char type; //�ù�,�������� ����
	int x,y; // x,y ĳ������ �߽���ǥ
	int spd; // �ӵ� 
	RECT collision_rect; // �浹 üũ�� ���� �簢��
	GameBitmap bitmap; // ���� �׸��� �ִ� �̹���
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

	//������Ʈ ��ȣ�ۿ�,�ù����̱�,�Ű�� ��� ��ȣ�ۿ� ����Ű
	void action(); 

	void update() override;
	void draw() override;
};

class Mission : GameObject {
	char type; //�̼� ����
	GameBitmap bitmap[5]; // �ӹ������� ���̴� �̹���

public:
	Mission();

	char getType();

	void setType();

	void update() override;
	void draw() override;
};

class MObj : GameObject {
	RECT collision_rect; // �浹 üũ�� ���� �簢��, �̹����ȿ��� Ű�������� �̼��� ������
	Mission mission;
	RECT src_rect;
	RECT dst_rect;
	GameBitmap bitmap; // �ʿ� ���̴� ������Ʈ �̹���
	
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