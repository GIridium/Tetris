#pragma once
#include <graphics.h>
#include <vector>
#include <memory>

#include "Block.h"
using namespace std;

class Tetris		//��ʾ����˹������Ϸ
{
public:
	//���캯��
	Tetris(int rows, int cols, int left, int top, int blockSize);		//left����Ϸ������ൽx=0�ľ���,top����Ϸ�����ϲൽy=0�ľ���,blockSize��С����ĳߴ�
	void init(); //��ʼ��
	void play();//��ʼ��Ϸ

private:
	//0���հ�
	//5�������ַ���
	vector<vector<int>>map;		//��ά���� �������趨����ߴ磬����ά��
	int rows;				//��������
	int cols;				//��������
	int leftMargin;			//����Χ���뱳�����ľ���
	int topMargin;			//����Χ���뱳���ϲ�ľ���
	int blockSize;			//�����С
	IMAGE imgBg;			//����ͼƬ
	int delay;				//��Ⱦһ�ε�ʱ�������������﷽��Ķ�(�ڳ�ʼ����ʱ��ֵ)
	bool update;			//�ж��Ƿ���Ⱦ 1��Ⱦ 0����Ⱦ
	std::unique_ptr<Block> curBlock;   // ��ǰ����
	std::unique_ptr<Block> nextBlock;  // ��һ������
	Block bakBlock;		//��������½��ķ���� �����ķ���
	int score;			//��ǰ����
	int highestScore;		//��ʷ��߷�
	bool gameOver;		//��Ϸ����
	IMAGE imgOver;		//��Ϸ����ͼƬ
	IMAGE imgStartBg;       // ��ʼ����ͼ
	IMAGE imgBeginBtn;      // ��ʼ��ťͼ
	IMAGE imgBeginBtnHover; // ��ť��ͣЧ��ͼ
	bool isHovering;        // ����Ƿ���ͣ�ڰ�ť��
	int bgOffset;           // ��������ƫ����
	int bgDirection;        // �����ƶ�����(1��-1)
	bool isFirstMusicPlaying;  // �������ֽ��沥�ŵı�־

	void initMusic();          // ��ʼ�����ֲ���
	void updateMusic();        // �������ֲ���״̬
	void keyboard();		//�����û�����
	void render();			//��Ⱦ��Ϸ����
	int getDelay();			//ʱ����
	void drop();			//��������
	void clearLine();		//����һ�з���
	void moveLeftRight(int offset);	//�����ƶ�
	void rotate();			//��ת
	void drawScore();		//���Ʒ���
	void checkOver();		//�����Ϸ�Ƿ����
	void saveScore();		//������߷�
	void displayOver();		//������Ϸ����
	void initStartScreen();  // ��ʼ����ʼ����
	void renderStartScreen();// ��Ⱦ��ʼ����
	bool checkStartClick();  // ��鿪ʼ���
};

